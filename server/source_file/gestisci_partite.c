#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

/*variabili globali*/

//mutex
pthread_mutex_t partite_mutex;

//gestione partite e giocatori
Giocatori *giocatore[MAX_COLLEGATI];
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
        perror("Errore nell'allocazione della memoria\n");
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

Partita *crea_partita(Giocatori *giocatore)
{
    //protegge l'accesso alla lista globale delle partite
    pthread_mutex_lock(&partite_mutex);

    // Genera un ID univoco per la nuova partita
    int id_partita = atomic_fetch_add(&num_partite, 1);  // Incrementa atomico e restituisce il valore precedente
    if (id_partita >= MAX_PARTITE)
    {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(giocatore->socket, "Limite massimo di partite raggiunto\n");
        return NULL;
    }

    // Crea la partita
    Partita *nuova_partita = inizializza_partita(id_partita, giocatore->socket, giocatore->nome);
    if (!nuova_partita)
    {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(giocatore->socket, "Errore nella creazione della partita.\n");
        return NULL;
    }

    pthread_mutex_init(&nuova_partita->mutex, NULL);
    pthread_cond_init(&nuova_partita->cond, NULL);

    partite[id_partita] = nuova_partita;

    char creazione[MAX];
    snprintf(creazione, sizeof(creazione), "Partita creata con ID %d. In attesa di un altro giocatore...\n", id_partita);
    invia_messaggi(giocatore->socket, creazione);

    pthread_mutex_unlock(&partite_mutex);

    while (nuova_partita->giocatore[1] == NULL)
    {
        printf("Il giocatore %s ha creato la partita %d ed è in attesa...\n", giocatore->nome, id_partita);
        //protegge l'accesso alla partita specifica
        pthread_mutex_lock(&nuova_partita->mutex);
        pthread_cond_wait(&nuova_partita->cond, &nuova_partita->mutex);  // Sospendi il thread
    }
    pthread_mutex_unlock(&nuova_partita->mutex);

    printf("Il giocatore %s ha ricevuto il segnale ed esce dall'attesa!\n", giocatore->nome);

    pthread_mutex_unlock(&partite_mutex);
    gestisci_partita(nuova_partita);

    return nuova_partita;
}

void unisci_a_partita(Giocatori *giocatore)
{
    pthread_mutex_lock(&partite_mutex);

    int id_partita = -1;

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
        return;
    }

    Partita *partita = partite[id_partita];

    pthread_mutex_lock(&partita->mutex);
    Giocatori *giocatore2 = inizializza_giocatore(giocatore->socket, id_partita, giocatore->nome, "O");
    if (!giocatore2)
    {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(giocatore->socket, "Errore nell'aggiunta del giocatore.\n");
        return;
    }
    partita->giocatore[1] = giocatore2;

    printf("Il giocatore %s si sta unendo alla partita %d...\n", giocatore->nome, giocatore2->id_partita);

    char unione[MAX];
    snprintf(unione, sizeof(unione), "Ti sei unito alla partita %d. Inizia il gioco!\n", id_partita);
    invia_messaggi(giocatore->socket, unione);

    invia_messaggi(partita->giocatore[0]->socket, "Il secondo giocatore si è unito! Inizia il gioco!\n");

    pthread_cond_signal(&partita->cond);  // Sveglia il primo giocatore
    printf("Il secondo giocatore ha sbloccato il primo con pthread_cond_signal!\n");
    pthread_mutex_unlock(&partita->mutex);
    pthread_mutex_unlock(&partite_mutex);
}

void *gestisci_partita(Partita *partita)
{
    if (!partita || !partita->giocatore[0] || !partita->giocatore[1])
    {
        printf("Errore: Partita %d non valida!\n", partita->id);
        return NULL;
    }

    // La partita inizia
    partita->stato = 1;
    partita->giocatore[0]->stato = 1; // Il primo giocatore inizia il turno
    partita->giocatore[1]->stato = 0; // Il secondo aspetta

    printf("Partita %d iniziata tra %s (X) e %s (O)\n", partita->id, partita->giocatore[0]->nome, partita->giocatore[1]->nome);

    // Invia messaggio ai giocatori
    char msg[256];
    snprintf(msg, sizeof(msg), "La partita è iniziata! %s (X) inizia per primo.\n", partita->giocatore[0]->nome);
    invia_messaggi(partita->giocatore[0]->socket, msg);
    invia_messaggi(partita->giocatore[1]->socket, msg);

    return NULL;
}
