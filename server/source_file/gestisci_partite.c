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
void inizializza_griglia(char griglia[N])
{
    for (int i = 0; i < 9; i++) 
        griglia[i] = '1' + i; 
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

    nuova_partita->stato = -1;                  // 0 = nuova creazione
    nuova_partita->turno = 0;                  // 0 = turno primo giocatore | 1 = turno secondo giocatore
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

}



Partita *gestisci_creazione_partita(Giocatori *giocatore)
{
    //protegge l'accesso alla lista globale delle partite
    pthread_mutex_lock(&partite_mutex);

    int id_partita = generazione_id(giocatore);

    Partita* nuova_partita = genera_partita(id_partita,giocatore);

    inizializza_griglia(nuova_partita->griglia);

   
    /*char msg[MAX];
    snprintf(msg, sizeof(msg), "Partita creata con ID %d. In attesa di un altro giocatore...\n", id_partita);
    invia_messaggi(giocatore->socket, msg);*/

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
    partita->stato = 0;

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

    if (partita->giocatore[0] != NULL && partita->giocatore[1] != NULL)
    {

        pthread_t thread_partita;
        pthread_create(&thread_partita, NULL, gestisci_gioco, (void *)partita);
        pthread_detach(thread_partita);
    }

    pthread_cond_signal(&partita->cond);  // Sveglia il primo giocatore
    pthread_mutex_unlock(&partita->mutex);
    pthread_mutex_unlock(&partite_mutex);   
}




void formato_griglia(char *buffer, char grid[N]) {
    sprintf(buffer, 
        "%c | %c | %c\n"
        "---------\n"
        "%c | %c | %c\n"
        "---------\n"
        "%c | %c | %c\n",
        grid[0], grid[1], grid[2], grid[3], grid[4], grid[5], grid[6], grid[7], grid[8]);

}


int ricevi_mossa(Giocatori *g)
{
    int mossa;
    
        if(recv(g->socket,&mossa, sizeof(mossa),0) <=0)
        {
            printf("Errore nella ricezione della mossa\n");
            return -1;
        }
        return ntohl(mossa);
}


void *gestisci_gioco(void *arg)
{
    Partita *p = (Partita *) arg;
    char buffer_griglia[MAX];
    char messaggio[MAX];

    while(1)
    {
        pthread_mutex_lock(&p->mutex);
        if(p->stato == 1)
        {
            pthread_mutex_unlock(&p->mutex);
            break;
        }

        int turno = p->turno;
        int avversario = (turno +1) %2;
       
        //printf("La partita con id %d è iniziata tra %s e %s\n", p->id, p->giocatore[0]->nome,p->giocatore[1]->nome);
        //printf("è il turno di %s\n",g_attivo->nome);

        formato_griglia(buffer_griglia, p->griglia);
        
        invia_messaggi(p->giocatore[turno]->socket, "TUO_TURNO\n");
        invia_messaggi(p->giocatore[turno]->socket, buffer_griglia);
        
        invia_messaggi(p->giocatore[avversario]->socket, "ATTENDI\n"); 
        invia_messaggi(p->giocatore[avversario]->socket, buffer_griglia);
    

        ricevi_messaggi(p->giocatore[turno]->socket, messaggio, sizeof(messaggio));
        int mossa=atoi(messaggio);
        printf("La mossa ricevuta è %d\n",mossa);

        p->turno = avversario;
        pthread_mutex_unlock(&p->mutex);
        
    }

    return NULL;
}

int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore) {
    // Verifica che la mossa sia un numero valido (da 1 a 9)
    if (mossa < 1 || mossa > 9) {
        printf("Mossa non valida: deve essere un numero tra 1 e 9.\n");
        return 1;
    }

    // Verifica che la casella selezionata sia libera
    if (partita->griglia[mossa - 1] != (mossa + '0')) {
        printf("La casella %d è già occupata.\n", mossa);
        return 1;
    }

    // Se la mossa è valida, aggiorna la griglia con il simbolo del giocatore
    partita->griglia[mossa - 1] = (giocatore->simbolo == 0) ? 'O' : 'X';

    return 0 ;
}
