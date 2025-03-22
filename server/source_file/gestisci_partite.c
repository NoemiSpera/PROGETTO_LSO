#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


//mutex
pthread_mutex_t partite_mutex = PTHREAD_MUTEX_INITIALIZER;


/*variabili globali*/

//gestione partite e giocatori
//Giocatori *giocatore[MAX_COLLEGATI];
Partita *partite[MAX_PARTITE];
atomic_int num_partite = ATOMIC_VAR_INIT(0);


//inizializzazioni
void inizializza_griglia(char griglia[N][N])
{
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            griglia[i][j] = '-';
        }
    }
}



Giocatori *inizializza_giocatore(int socket, int id_partita, char* nome, char *simbolo)
{
    Giocatori *giocatore = (Giocatori*)malloc(sizeof(Giocatori));
    if(!giocatore) return NULL;

    giocatore->socket = socket;
    strncpy(giocatore->nome, nome, MAX_NOME);
    giocatore->id_partita = id_partita;
    giocatore->stato = 0;
    giocatore->simbolo = -1;

    return giocatore;
}



Partita* inizializza_partita(int id_partita, int socket_giocatore, char *nome_giocatore)
{
    Partita *nuova_partita = (Partita*)malloc(sizeof(Partita));
    if(!nuova_partita)
    {
        perror(YELLOW"Errore nell'allocazione della memoria\n"RESET);
        free(nuova_partita);
        return NULL;
    }

    nuova_partita->id = id_partita;

    Giocatori *giocatore1 = inizializza_giocatore(socket_giocatore, id_partita, nome_giocatore, "X");
    if (!giocatore1)
    {
        free(nuova_partita);
        return NULL;
    }

    nuova_partita->giocatore[0] = giocatore1;
    nuova_partita->giocatore[1] = NULL;

    inizializza_griglia(nuova_partita->griglia);

    nuova_partita->stato = 0;                  // 0 = nuova creazione
    nuova_partita->turno = 0;                  // 0 = non è il tuo turno
    nuova_partita->risultato = -1;             // -1 = ancora nessun risultato

    return nuova_partita;
}



int generazione_id(Giocatori *giocatore)
{

    // Genera un ID univoco per la nuova partita
    int id_partita = atomic_fetch_add(&num_partite, 1);  // Incrementa atomico e restituisce il valore precedente
    if (id_partita >= MAX_PARTITE)
    {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(giocatore->socket, YELLOW "Limite massimo di partite raggiunto\n" RESET);
        return -1;
    }

    return id_partita;
}



Partita *genera_partita(int id_partita, Giocatori *giocatore)
{
     // Crea la partita
     Partita *nuova_partita = inizializza_partita(id_partita, giocatore->socket, giocatore->nome);
     if (!nuova_partita)
     {
         pthread_mutex_unlock(&partite_mutex);
         invia_messaggi(giocatore->socket, YELLOW "Errore nella creazione della partita.\n" RESET);
         return NULL;
     }

     //inserisco la nuova partita nell'array globale
     partite[id_partita] = nuova_partita;

     return nuova_partita;
}



void attendi_giocatore(int id_partita, Partita* partita, Giocatori *giocatore)
{
    //inizializzo il mutex e la variabile di condizione della struttura partita
    pthread_mutex_init(&partita->mutex, NULL);
    pthread_cond_init(&partita->cond, NULL);
    
    while (partita->giocatore[1] == NULL)
    {
        printf("%s ha creato la partita %d ed è in attesa di un altro giocatore...\n", giocatore->nome, id_partita);
        //protegge l'accesso alla partita specifica
        pthread_mutex_lock(&partita->mutex);
        pthread_cond_wait(&partita->cond, &partita->mutex);  // Sospendi il thread
    }
    pthread_mutex_unlock(&partita->mutex);

    printf("Il giocatore %s ha ricevuto il segnale ed esce dall'attesa!\n", giocatore->nome);

}



Partita *gestisci_creazione_partita(Giocatori *giocatore)
{
    //protegge l'accesso alla lista globale delle partite
    pthread_mutex_lock(&partite_mutex);

    int id_partita = generazione_id(giocatore);

    Partita* nuova_partita = genera_partita(id_partita,giocatore);

    //invio messaggio al client 
    char msg[MAX];
    snprintf(msg, sizeof(msg), "Partita creata con ID %d. In attesa di un altro giocatore...\n", id_partita);
    invia_messaggi(giocatore->socket, msg);

    pthread_mutex_unlock(&partite_mutex);

    attendi_giocatore(id_partita, nuova_partita, giocatore);

    return nuova_partita;
}



Partita *cerca_partita_disponibile(Giocatori *giocatore)
{
    int id_partita = -1;

    //trovar en'altra soluzione
    // Troviamo una partita con solo un giocatore
    for (int i = 0; i < MAX_PARTITE; i++)
    {
        if (partite[i] != NULL && partite[i]->giocatore[1] == NULL)
        {
            id_partita = i;
            break;
        }
    }

    if (id_partita == -1)
    {
        invia_messaggi(giocatore->socket, "Nessuna partita disponibile al momento.\n");
        pthread_mutex_unlock(&partite_mutex);
        return NULL;
    }

    Partita *partita = partite[id_partita];

    return partita;
}



void unisci_a_partita(Partita *partita, Giocatori *giocatore)
{
    Giocatori *giocatore2 = inizializza_giocatore(giocatore->socket, partita->id, giocatore->nome, "O");
    if (!giocatore2)
    {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(giocatore->socket, "Errore nell'aggiunta del giocatore.\n");
        return;
    }
    partita->giocatore[1] = giocatore2;

    printf("%s si sta unendo alla partita %d...\n", giocatore->nome, giocatore2->id_partita);
}



void gestisci_ingresso_partita(Giocatori *giocatore)
{
    pthread_mutex_lock(&partite_mutex);

    Partita *partita = cerca_partita_disponibile(giocatore);
    
    pthread_mutex_lock(&partita->mutex);

    unisci_a_partita(partita,giocatore);
   
    char msg[MAX];
    snprintf(msg, sizeof(msg), "Ti sei unito alla partita %d. Inizia il gioco!\n", partita->id);
    invia_messaggi(giocatore->socket, msg);
    invia_messaggi(partita->giocatore[0]->socket, "Il secondo giocatore si è unito! Inizia il gioco!\n");

    pthread_cond_signal(&partita->cond);  // Sveglia il primo giocatore
    pthread_mutex_unlock(&partita->mutex);
    pthread_mutex_unlock(&partite_mutex);   
}