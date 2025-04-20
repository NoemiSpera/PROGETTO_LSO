#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

ListaPartite *lista_partite;

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



//scambio messaggi client-server
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
    
    // Se la lista è vuota, la nuova partita diventa la testa
    if (lista_partite->head == NULL) {
        lista_partite->head = nuova_partita;
    } else {
        // Trova l'ultimo nodo della lista
        Partita *current = lista_partite->head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Aggiunge la nuova partita in coda
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
    pthread_mutex_lock(&lista_partite->mutex);  // Blocca l'accesso alla lista

    Partita *corrente = lista_partite->head;  // Inizializza la variabile corrente alla testa della lista

    while (corrente != NULL) {
        if (corrente->stato == IN_ATTESA) 
        {  
            printf("ID: %d |creatore: %s\n", corrente->id,corrente->giocatore[0]->nome);
        }

        corrente = corrente->next;  // Passa alla partita successiva
    }

    pthread_mutex_unlock(&lista_partite->mutex);  // Sblocca l'accesso alla lista
}


void conversione_lista_partite(char *buffer, size_t dim_max)
{
    buffer[0] = '\0';  // svuota il buffer

    pthread_mutex_lock(&lista_partite->mutex);

    Partita *corrente = lista_partite->head;
    while (corrente != NULL) 
    {
        if (corrente->stato == IN_ATTESA && corrente->giocatore[1] == NULL) {
            char riga[256];
            snprintf(riga, sizeof(riga), "ID: %d | Giocatore: %s\n", corrente->id, corrente->giocatore[0]->nome);
            strncat(buffer, riga, dim_max - strlen(buffer) - 1);
        }
        corrente = corrente->next;
    }

    pthread_mutex_unlock(&lista_partite->mutex);
}

