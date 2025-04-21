#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


//gestione client
void *gestisci_client(void *arg)
{
    int client_socket = *(int*)arg;
    free(arg);

    int rec = 0;
    int indice_gioco=-1;
    char nome[MAX_NOME];
    char buffer[MAX];
    
    rec = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
    printf(CYAN"%s ha fatto accesso al server\n" RESET, buffer);
    strcpy(nome,buffer);

    Giocatori *giocatore = inizializza_giocatore(client_socket,-1,buffer, "-1");
    giocatore->in_partita = -1; //non è in partita

    stampa_lista_giocatori();
    aggiungi_giocatore(giocatore);

    char benvenuto[256];
    snprintf(benvenuto, sizeof(benvenuto), "%s%sCiao %.200s, benvenuto nel server!%s\n", CYAN, BOLD, buffer, RESET);
    invia_messaggi(client_socket, benvenuto);
    usleep(1);


    while (1)  // 🔁 Loop infinito: menu → partita → menu → ...
    {
       // ✅ Se sei in partita, aspetta che finisca
        if (giocatore->in_partita == 1) {
            Partita *partita = trova_partita(giocatore->id_partita);  // Assicurati di avere questa funzione
        
            if (partita == NULL) {
                printf("ERROR: partita non trovata per id %d\n", giocatore->id_partita);
                return NULL;  // Puoi gestire il caso in modo diverso, a seconda di cosa vuoi fare
            }
            
            if (partita) {
                //pthread_mutex_lock(&partita->mutex);
                
                pthread_mutex_lock(&partita->mutex);
                while (partita->stato != TERMINATA)  {
                   pthread_cond_wait(&partita->cond, &partita->mutex);
                    printf("DEBUG: Aspetto che la partita finisca\n");
                    printf("DEBUG: La partita è finita\n");
                    
                }
                pthread_mutex_unlock(&partita->mutex);
                
                printf("DEBUG: Partita terminata, ritorno al menu...\n");
                char msg[] = "Vuoi:\nC) Creare una nuova partita\nA) Giocare con un AMICO\nB) Unirti ad una partita CASUALE\nQ) Uscire\n";
                printf("DEBUG: Menu stampato dopo la fine della partita...\n");
                giocatore->id_partita = -1;
                giocatore->in_partita = -1;

            }
        }

        // ✅ Mostra il menu SOLO se non in partita
        char msg[] = "Vuoi:\nC) Creare una nuova partita\nA) Giocare con un AMICO\nB) Unirti ad una partita CASUALE\nQ) Uscire\n";
        invia_messaggi(client_socket, msg);
        printf("Attendo ricezione del messaggio...\n");

        int bytes_ricevuti = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
        if (bytes_ricevuti > 0) {
            char scelta = buffer[0];
            int esito = gestisci_scelta(giocatore, scelta);  // Processa la scelta
            printf("sono in gestisci client con esito %d\n", esito);
            // 🔁 Se la scelta ha portato all'inizio di una partita, torni in alto e aspetti
            if (esito == 1 ) {
                continue;
            }
        } 
        else {
            // Connessione chiusa o errore
            free(giocatore);
            close(client_socket);
            return NULL;
        }

        
    }

    return NULL;
}




int  gestisci_scelta(Giocatori *giocatore, char scelta)
{
    char buffer[MAX];
   

    switch (scelta)
    {
        case 'C':
        case 'c':
            printf("%s ha scelto di creare una nuova partita\n", giocatore->nome);
            printf("Creazione in corso...\n");
            creazione_partita(giocatore); // Creazione partita
            return 1;
        
        case 'A':
        case 'a':
            printf("%s ha scelto di giocare con un amico\n", giocatore->nome);
            int esito = assegnazione_amico(giocatore); // Richiesta di assegnazione con amico
            printf("Sto tornando in gestisci_client....");
            return esito;
        
        case 'B':
        case 'b':
            printf("%s ha scelto di partecipare ad una partita casuale\n", giocatore->nome);
            int esito_casuale = assegnazione_casuale(giocatore); // Partecipazione casuale
            return esito_casuale;
        

        case 'Q':
        case 'q':
        printf("Uscita in corso del giocatore %s...\n", giocatore->nome);
        rimuovi_giocatore(giocatore->socket);
        close(giocatore->socket);      
        pthread_exit(NULL);  

        default:
            invia_messaggi(giocatore->socket, "Scelta non valida\n");
            return 0;
    }
}



int assegnazione_amico(Giocatori *giocatore)
{
    char messaggio[MAX];
    int id_partita;
    Partita *partita = NULL;
   
    conversione_lista_partite(messaggio, sizeof(messaggio));
    invia_messaggi(giocatore->socket,messaggio);
        
    // Riceve l'ID della partita dal client
    ricevi_messaggi(giocatore->socket, messaggio, sizeof(messaggio));
    id_partita = atoi(messaggio);  

    // Cerca la partita
    partita = trova_partita(id_partita);

    if (partita == NULL) {
            
        snprintf(messaggio, sizeof(messaggio), "La partita con ID %d non esiste. Riprova!\n", id_partita);
        //mettere il messaggio di ricezione nel client
        invia_messaggi(giocatore->socket, messaggio);  
        return -1;
        
    }

    Giocatori *creatore = partita->giocatore[0];
    if (notifica_creatore(partita, creatore, giocatore) == 1)
    {
        giocatore->id_partita = partita->id;
        strcpy(giocatore->simbolo, "O");
        giocatore->in_partita = 1;

        partita->giocatore[1] = giocatore;
        partita->giocatore[0]->in_partita = 1;

        pthread_mutex_lock(&partita->mutex);
        pthread_cond_signal(&partita->cond);  // Svegliamo il creatore
        pthread_mutex_unlock(&partita->mutex);
        avvia_thread_partita(partita);
        printf("sto tornando in gestisci scelta\n");
        return 1;
    } 
    else {
        return -1;  // Partita rifiutata
    }
}



int assegnazione_casuale(Giocatori *giocatore)
{
    Partita *partita = cerca_partita_disponibile(giocatore);

    char msg[MAX];
    snprintf(msg, sizeof(msg),"Sei stato assegnato alla partita %d. attendiamo la risposta del creatore", partita->id);
    invia_messaggi(giocatore->socket,msg);
   
    if (partita != NULL)
    {
        //Giocatori *creatore = partita->giocatore[0];
        if (notifica_creatore(partita, partita->giocatore[0], giocatore) == 1)
        {
            giocatore->id_partita = partita->id;
            strcpy(giocatore->simbolo, "O");
            giocatore->in_partita = 1;

            partita->giocatore[1] = giocatore;
            partita->giocatore[0]->in_partita = 1;

            pthread_mutex_lock(&partita->mutex);
            pthread_cond_signal(&partita->cond); // Svegliamo il creatore
            pthread_mutex_unlock(&partita->mutex);
            avvia_thread_partita(partita);
            printf("sto tornando in gestisci scelta\n");
            return 1;
        }
        else
        {
            return -1; // Partita rifiutata
        }
    }
    else{
        printf("nessuna partita disponibile al momento \n");
        return -1;
    }

}



    int notifica_creatore(Partita *partita,Giocatori *creatore,Giocatori *giocatore)
    {
        // Invia richiesta di partecipazione al creatore della partita
        char richiesta[MAX];
        snprintf(richiesta, sizeof(richiesta), "Vuoi giocare con %s (s/n): ",giocatore->nome);
        
        // Invia il messaggio al creatore della partita
        invia_messaggi(creatore->socket, richiesta);

        //risposta
        char risposta[10];
        ricevi_messaggi(creatore->socket,risposta,sizeof(risposta));

        if(risposta[0] == 's' || risposta[0] == 'S')
        {
            printf("%s ha accettato la richiesta di %s. La partita inizia\n", creatore->nome,giocatore->nome);
            invia_messaggi(creatore->socket, "Hai accettato la richiesta! La partita inizia.\n");
            invia_messaggi(giocatore->socket, "La tua richiesta è stata accettata! La partita inizia.\n");
            return 1;

        } else if( risposta[0] == 'n' || risposta[0] == 'N')
        {
            printf("%s ha rifiutato la richiesta di %s\n", creatore->nome,giocatore->nome);
            sprintf(richiesta, "PARTITA_RIFIUTATA: %s ha rifiutato la tua richiesta.\n", giocatore->nome);
            invia_messaggi(giocatore->socket, richiesta);  // Invia il rifiuto
            usleep(1);
            invia_messaggi(creatore->socket, "Hai rifiutato la richiesta. Resterai in attesa di un altro giocatore...\n");
            return -1;

        }

    }

    Partita* trova_partita(int id_partita) {
        pthread_mutex_lock(&lista_partite->mutex);  // Blocca il mutex

        Partita *current = lista_partite->head;

        while (current != NULL) {
            if (current->id == id_partita) {
                pthread_mutex_unlock(&lista_partite->mutex);  // Sblocca il mutex
                return current;
            }
            current = current->next;
        }

        pthread_mutex_unlock(&lista_partite->mutex);  // Sblocca il mutex
        return NULL;  // Se non trova la partita
    }
