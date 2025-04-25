#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


//gestione client
void *gestisci_client(void *arg)
{
    int client_socket = *(int*)arg;
    free(arg);

    int rec = 0;
    char nome[MAX_NOME];
    char buffer[MAX];

    rec = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
    printf(CYAN"%s ha fatto accesso al server\n" RESET, buffer);
    strcpy(nome,buffer);

    Giocatori *giocatore = inizializza_giocatore(client_socket,-1,buffer, "-1");
    giocatore->in_partita = -1; //non è in partita


    aggiungi_giocatore(giocatore);

    char benvenuto[256];
    snprintf(benvenuto, sizeof(benvenuto), "%s%sCiao %.200s, benvenuto nel server!%s\n", CYAN, BOLD, buffer, RESET);
    invia_messaggi(client_socket, benvenuto);
    usleep(1);


    while (1)  
    {
        
        if (giocatore->in_partita == 1) {
            Partita *partita = trova_partita(giocatore->id_partita);  
        
            if (partita == NULL) {
                printf("ERROR: partita non trovata per id %d\n", giocatore->id_partita);
                return NULL;  
            }
            
            if (partita) {
                
                pthread_mutex_lock(&partita->mutex);
                while (partita->stato != TERMINATA)  {
                    
                    pthread_cond_wait(&partita->cond, &partita->mutex);

                }
                pthread_mutex_unlock(&partita->mutex);
                rimuovi_partita(partita->id);
                
                char msg[] = "Vuoi:\nC) Creare una nuova partita\nA) Giocare con un AMICO\nB) Unirti ad una partita CASUALE\nQ) Uscire\n";
                
                giocatore->id_partita = -1;
                giocatore->in_partita = -1;

            }
        }

        
        char msg[] = "Vuoi:\nC) Creare una nuova partita\nA) Giocare con un AMICO\nB) Unirti ad una partita CASUALE\nQ) Uscire\n";
        invia_messaggi(client_socket, msg);

        int bytes_ricevuti = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
        if (bytes_ricevuti > 0) {
            char scelta = buffer[0];
            int esito = gestisci_scelta(giocatore, scelta);  
            
        } 
        else {
            
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
            giocatore->scelta=IN_SELEZIONE_ID;
            printf("%s ha scelto di giocare con un amico\n", giocatore->nome);
            int esito = assegnazione_amico(giocatore); // Richiesta di assegnazione con amico
            return esito;
        
        case 'B':
        case 'b':
            giocatore->scelta=IN_CASUALE;
            printf("%s ha scelto di partecipare ad una partita casuale\n", giocatore->nome);
            int esito_casuale = assegnazione_casuale(giocatore); // Partecipazione casuale
            return esito_casuale;
        

        case 'Q':
        case 'q':
        printf(GREEN"Uscita in corso del giocatore %s...\n"RESET, giocatore->nome);
        rimuovi_giocatore(giocatore->socket);
        close(giocatore->socket);      
        pthread_exit(NULL);  

        default:
            invia_messaggi(giocatore->socket, "Scelta non valida\n");
            return 0;
    }
}




Partita *creazione_partita(Giocatori *giocatore)
{
    
    pthread_mutex_lock(&partite_mutex);

    int id_partita = generazione_id(giocatore);

    Partita* nuova_partita = inizializza_partita(id_partita,giocatore);
    nuova_partita->stato=IN_ATTESA;

    inizializza_griglia(nuova_partita->griglia);

    aggiungi_partita(nuova_partita);
   
    /*char msg[MAX];
    snprintf(msg, sizeof(msg), "Partita creata con ID %d. In attesa di un altro giocatore...\n", id_partita);
    invia_messaggi(giocatore->socket, msg);*/

    pthread_mutex_unlock(&partite_mutex);
    
    messaggio_broadcast(giocatore, nuova_partita->id);

    attendi_giocatore(id_partita, nuova_partita, giocatore);

    return nuova_partita;
}



int generazione_id(Giocatori *giocatore)
{

    int id_partita = atomic_fetch_add(&num_partite, 1);  // Incrementa atomico e restituisce il valore precedente
    if (id_partita >= MAX_PARTITE)
    {
        invia_messaggi(giocatore->socket, YELLOW "Limite massimo di partite raggiunto\n" RESET);
        return -1;
    }

    return id_partita;
}



void attendi_giocatore(int id_partita, Partita* partita, Giocatori *giocatore)
{
    pthread_mutex_lock(&partita->mutex);
    
    while (partita->giocatore[1] == NULL)
    {
        printf("%s ha creato la partita %d ed è in attesa di un altro giocatore...\n", giocatore->nome, id_partita);
        pthread_cond_wait(&partita->cond, &partita->mutex);  // Sospendi il thread
    }
    pthread_mutex_unlock(&partita->mutex);

}



int assegnazione_amico(Giocatori *giocatore)
{
    char messaggio[MAX];
    int id_partita;
    Partita *partita = NULL;

    if (conversione_lista_partite(messaggio, sizeof(messaggio)) == 0) {
        snprintf(messaggio, sizeof(messaggio), RED"Nessuna partita disponibile al momento. Torno al menu"RESET);
        invia_messaggi(giocatore->socket, messaggio);
        usleep(1);
        return -1;
    
    }else{
        invia_messaggi(giocatore->socket,messaggio);
         // Riceve l'ID della partita dal client
        ricevi_messaggi(giocatore->socket, messaggio, sizeof(messaggio));
        id_partita = atoi(messaggio);  

        // Cerca la partita
        partita = trova_partita(id_partita);

        if (partita == NULL) {
                
            snprintf(messaggio, sizeof(messaggio), "La partita con ID %d non esiste. Riprova dal menu!\n", id_partita);
            invia_messaggi(giocatore->socket, messaggio);  
            return -1;
            
        }

        Giocatori *creatore = partita->giocatore[0];
        pthread_mutex_lock(&partita->mutex);
        partita->stato = IN_ACCETTAZIONE;
        pthread_mutex_unlock(&partita->mutex);
        if (notifica_creatore(partita, creatore, giocatore) == 1)
        {
            giocatore->id_partita = partita->id;
            strcpy(giocatore->simbolo, "O");
            giocatore->in_partita = 1;
            giocatore->scelta = NESSUNA;

            partita->giocatore[1] = giocatore;
            partita->giocatore[0]->in_partita = 1;
            partita->giocatore[0]->scelta = NESSUNA;
            
            notifica_occupazione_partita(partita->id);
            pthread_mutex_lock(&partita->mutex);
            pthread_cond_signal(&partita->cond);  // Sveglio il creatore
            pthread_mutex_unlock(&partita->mutex);
            avvia_thread_partita(partita);
            
            return 1;
        } 
        else {
            return -1;  // Partita rifiutata
        }
    }  
}



int numero_partite_disponibili() {
    int count = 0;
    Partita *curr;

    pthread_mutex_lock(&lista_partite->mutex);
    
    curr = lista_partite->head;

    while (curr != NULL) {

        if (curr->stato == IN_ATTESA) {
            count++;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&lista_partite->mutex);

    return count;
}



int assegnazione_casuale(Giocatori *giocatore)
{
    Partita *partita = cerca_partita_disponibile(giocatore);

    char msg[MAX];

    if (partita != NULL)
    {
        snprintf(msg, sizeof(msg),"Sei stato assegnato alla partita %d. attendiamo la risposta del creatore\n", partita->id);
        invia_messaggi(giocatore->socket,msg);

        pthread_mutex_lock(&partita->mutex);
        partita->stato = IN_ACCETTAZIONE;
        pthread_mutex_unlock(&partita->mutex);
        if (notifica_creatore(partita, partita->giocatore[0], giocatore) == 1)
        {
            giocatore->id_partita = partita->id;
            strcpy(giocatore->simbolo, "O");
            giocatore->in_partita = 1;
            giocatore->scelta = NESSUNA;

            partita->giocatore[1] = giocatore;
            partita->giocatore[0]->in_partita = 1;
            partita->giocatore[0]->scelta = NESSUNA;

            notifica_occupazione_partita(partita->id);
            pthread_mutex_lock(&partita->mutex);
            pthread_cond_signal(&partita->cond); // Sveglio il creatore
            pthread_mutex_unlock(&partita->mutex);
            avvia_thread_partita(partita);
            return 1;
        }
        else
        {
            return -1; // Partita rifiutata
        }
    }
    else{
        invia_messaggi(giocatore->socket,RED"Nessuna partita disponibile al momento. Torno al menu\n"RESET);
        usleep(1);
        return -1;
    }

}



Partita *cerca_partita_disponibile(Giocatori *giocatore)
{
    pthread_mutex_lock(&lista_partite->mutex);

    Partita *current = lista_partite->head;

    while (current != NULL)
    {
        if (current->stato == IN_ATTESA)
        {
            pthread_mutex_unlock(&lista_partite->mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&lista_partite->mutex); 
    return NULL;     
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
        pthread_mutex_lock(&partita->mutex);
        partita->stato = IN_CORSO;
        pthread_mutex_unlock(&partita->mutex);
        return 1;

    } else if( risposta[0] == 'n' || risposta[0] == 'N')
    {
        printf("%s ha rifiutato la richiesta di %s\n", creatore->nome,giocatore->nome);
        sprintf(richiesta, "La tua richiesta è stata rifiutata da %s. Torna al menu", giocatore->nome);
        invia_messaggi(giocatore->socket, richiesta);  // Invia il rifiuto
        usleep(1);
        invia_messaggi(creatore->socket, "Hai rifiutato la richiesta. Resterai in attesa di un altro giocatore...\n");
        pthread_mutex_lock(&partita->mutex);
        partita->stato = IN_ATTESA;
        pthread_mutex_unlock(&partita->mutex);
        return -1;

    }

}



Partita* trova_partita(int id_partita) {
    pthread_mutex_lock(&lista_partite->mutex);  

    Partita *current = lista_partite->head;

    while (current != NULL) {
        if (current->id == id_partita) {
            pthread_mutex_unlock(&lista_partite->mutex); 
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&lista_partite->mutex);  
    return NULL;  
}