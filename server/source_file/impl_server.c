#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"



//mutex
pthread_mutex_t partite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_giocatori = PTHREAD_MUTEX_INITIALIZER;



/*variabili globali*/
Giocatori* giocatori_connessi[MAX_COLLEGATI];
ListaPartite *lista_partite;
atomic_int num_partite = ATOMIC_VAR_INIT(0);
int numero_giocatori = 0;



//scambio messaggi 
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n;
    memset(buffer, 0, buf_size); // Puliamo il buffer
    n = recv(client_fd, buffer, buf_size - 1, 0);
    if(n == -1)
    {
        perror("Errore nella ricezione del messaggio");
        close(client_fd);
        exit(1);
    }
    if (n > 0) 
    {
        buffer[n] = '\0';
    }
    return n;
}



void invia_messaggi(int client_fd, char *msg)
{
    if(send(client_fd, msg, strlen(msg), MSG_NOSIGNAL) == -1)
    {
        perror("Errore nell'invio della risposta al server\n");
        close(client_fd);
        exit(1);
    }
    
}



void messaggio_broadcast(Giocatori *creatore, int id_partita)
{
    char msg[MAX];
    snprintf(msg, sizeof(msg),"[NOTIFICA] %s ha creato una nuova partita! ID: %d\nUnisciti!",creatore->nome, id_partita);

    pthread_mutex_lock(&mutex_giocatori);
    int notifiche_inviate = 0;

    for (int i = 0; i < numero_giocatori; i++) {
        Giocatori *g = giocatori_connessi[i];

        if (g != NULL && g->in_partita == -1 && g->socket != creatore->socket) {

            invia_messaggi(g->socket, msg);
            notifiche_inviate++;
        }
    }

    if (notifiche_inviate == 0) {
        printf("Nessun giocatore libero a cui inviare la notifica\n");
    }
    pthread_mutex_unlock(&mutex_giocatori);
}


void notifica_occupazione_partita(int id_occupato)
{
    char buffer[MAX*2];
    char notifica[MAX*3];
    Giocatori *g;

    // Ricava la lista aggiornata
    conversione_lista_partite(buffer, sizeof(buffer));
    if (buffer[0] == '\0') {
        snprintf(notifica, sizeof(notifica), 
        "[NOTIFICA] La partita %d è stata occupata.\nNessuna partita è piu disponibile. Attendi...",id_occupato);
    }else{
   
        snprintf(notifica, sizeof(notifica), 
            "[NOTIFICA] La partita %d è stata occupata.\nLista aggiornata:\n%s", id_occupato, buffer);
    }

    pthread_mutex_lock(&mutex_giocatori);
    
    for (int i = 0; i < MAX_COLLEGATI; i++) 
    {
        Giocatori *g = giocatori_connessi[i];
        if (g != NULL && g->scelta == IN_SELEZIONE_ID) 
        {
            invia_messaggi(g->socket, notifica);
        }
    }
    pthread_mutex_unlock(&mutex_giocatori);

}


//funzioni per la gestione della lista delle partite
void inizializza_lista()
{
    lista_partite = malloc(sizeof(ListaPartite));
    if (!lista_partite)
    {
        printf("ERRORE nell'allocazione della lista\n");
        exit(EXIT_FAILURE); 
    }

    lista_partite->head = NULL;
    
    if (pthread_mutex_init(&lista_partite->mutex, NULL) != 0) {
        perror("Errore init mutex lista");
        exit(EXIT_FAILURE);
    }   
}



void aggiungi_partita(Partita *nuova_partita)
{
   
    pthread_mutex_lock(&lista_partite->mutex);
    
    nuova_partita->next = NULL;
    
    if (lista_partite->head == NULL) {
        lista_partite->head = nuova_partita;
    } else {
       
        Partita *current = lista_partite->head;
        while (current->next != NULL) {
            current = current->next;
        }
        
        current->next = nuova_partita;
    }
    
    pthread_mutex_unlock(&lista_partite->mutex);
}



void rimuovi_partita(int id)
{
    pthread_mutex_lock(&lista_partite->mutex);

    Partita *corrente = lista_partite->head;
    Partita *precedente = NULL;

    while (corrente != NULL)
    {
        if (corrente->id == id)
        {
            if (precedente == NULL)
            {
                lista_partite->head = corrente->next;
            }
            else
            {
                precedente->next = corrente->next;
            }

            pthread_mutex_destroy(&corrente->mutex);
            pthread_cond_destroy(&corrente->cond);
            free(corrente);
            pthread_mutex_unlock(&lista_partite->mutex);
            return;
        }
        precedente = corrente;
        corrente = corrente->next;
    }

    pthread_mutex_unlock(&lista_partite->mutex);
}



void stampa_partite() 
{
    pthread_mutex_lock(&lista_partite->mutex);  

    Partita *corrente = lista_partite->head;  

    while (corrente != NULL) {
        if (corrente->stato == IN_ATTESA) 
        {  
            printf("ID: %d |creatore: %s\n", corrente->id,corrente->giocatore[0]->nome);
        }

        corrente = corrente->next;  
    }

    pthread_mutex_unlock(&lista_partite->mutex);  
}



int conversione_lista_partite(char *buffer, size_t dim_max)
{
    buffer[0] = '\0';  

    pthread_mutex_lock(&lista_partite->mutex);

    Partita *corrente = lista_partite->head;
    int partite_disponibili = 0;

    while (corrente != NULL) 
    {
        if (corrente->stato == IN_ATTESA && corrente->giocatore[1] == NULL) {
            partite_disponibili++;
            char riga[256];
            snprintf(riga, sizeof(riga), "ID: %d | Giocatore: %s\n", corrente->id, corrente->giocatore[0]->nome);
            strncat(buffer, riga, dim_max - strlen(buffer) - 1);
        }
        corrente = corrente->next;
    }
    
    pthread_mutex_unlock(&lista_partite->mutex);
    return partite_disponibili;
}



//funzione per la getsione della lista dei giocatori
void aggiungi_giocatore(Giocatori* nuovo) 
{
    
    pthread_mutex_lock(&mutex_giocatori);
    
    if (numero_giocatori < MAX_COLLEGATI) {
        giocatori_connessi[numero_giocatori++] = nuovo;
    } else {
        printf("ERRORE: Limite massimo di giocatori raggiunto\n");
    }

    pthread_mutex_unlock(&mutex_giocatori);
}



void rimuovi_giocatore(int socket_fd) 
{
    
    pthread_mutex_lock(&mutex_giocatori);

    for (int i = 0; i < numero_giocatori; i++) {
        if (giocatori_connessi[i]->socket == socket_fd) {
            
            free(giocatori_connessi[i]);

            // Shift a sinistra per mantenere compattezza
            for (int j = i; j < numero_giocatori - 1; j++) {
                giocatori_connessi[j] = giocatori_connessi[j + 1];
            }

            numero_giocatori--;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_giocatori);
}



//messaggi su stdout
void messaggio_benvenuto()
{
    stampa_bordo();
    printf("\n");
    stampa_testo_centrato(BOLD CYAN"BENVENUTO NEL SERVER DEL TRIS" RESET);
    stampa_testo_centrato(BOLD BLUE"TRE SEGNI, UN VINCITORE. SARAI TU?\n" RESET);
    stampa_bordo();
}



void stampa_bordo()
{
    for(int i=0; i < LARGHEZZA; i++)
    {
        printf("=");
    }
    printf("\n");
}



void stampa_testo_centrato(const char *testo)
{
    int lunghezza_testo = strlen(testo);
    int centro = (LARGHEZZA - lunghezza_testo) / 2;
    for(int i = 0; i < centro - 1; i++)
    printf(" ");
    {
        printf(" ");
    }
    printf("%s", testo);
    for(int i=0; i < (LARGHEZZA - (lunghezza_testo - 1) - centro); i++)
    {
        printf(" ");
    }
    printf("\n");
}