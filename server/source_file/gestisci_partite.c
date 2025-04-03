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
    strcpy(giocatore->simbolo, simbolo);

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



void *gestisci_gioco(void *arg)
{
    Partita *p = (Partita *) arg;
    char buffer_griglia[MAX];
    char messaggio[MAX];

    printf("La partita con id %d è iniziata tra %s e %s\n", p->id, p->giocatore[0]->nome,p->giocatore[1]->nome);

    while(p->stato == 0)
    {
        
        int turno = p->turno;
        int avversario = (turno +1) %2;
       
        
        printf("è il turno di %s\n",p->giocatore[turno]->nome);

        //formato_griglia(buffer_griglia, p->griglia);
        
        invia_messaggi(p->giocatore[turno]->socket, "TUO_TURNO\n");
        usleep(1);
        invia_messaggi(p->giocatore[turno]->socket, p->griglia);
        
        invia_messaggi(p->giocatore[avversario]->socket, "ATTENDI\n"); 
        usleep(1);
        invia_messaggi(p->giocatore[avversario]->socket, p->griglia);
       
    

        ricevi_messaggi(p->giocatore[turno]->socket, messaggio, sizeof(messaggio));
        int mossa=atoi(messaggio);

        //protegge la modifica della griglia
        pthread_mutex_lock(&p->mutex);

        if(mossa_valida(p,mossa,p->giocatore[turno],p->giocatore[turno]->simbolo))
        {
            if (controlla_vittoria(p->griglia, p->giocatore[turno]->simbolo))
            {
                p->stato = 1; // Partita terminata

                //1 da ricevere
                invia_messaggi(p->giocatore[turno]->socket, "PARTITA_VINTA!\n");
                sleep(1);
                invia_messaggi(p->giocatore[turno]->socket, p->griglia);
                invia_messaggi(p->giocatore[avversario]->socket, "PARTITA_PERSA!\n");
                sleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, p->griglia);

                pthread_mutex_unlock(&p->mutex);
                break; // Esce dal ciclo
            }
            
            if(controlla_pareggio(p->griglia))
            {
                p->stato = 1;

                invia_messaggi(p->giocatore[turno]->socket, "PAREGGIO!\n");
                sleep(1);
                invia_messaggi(p->giocatore[turno]->socket, p->griglia);
                invia_messaggi(p->giocatore[avversario]->socket, "PAREGGIO!\n");
                sleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, p->griglia);

                pthread_mutex_unlock(&p->mutex);
                break; // Esce dal ciclo  
            }

            p->turno = avversario;
        }else
        {
            invia_messaggi(p->giocatore[turno]->socket, "MOSSA NON VALIDA\n");
        }

       
        pthread_mutex_unlock(&p->mutex);
        
    }

    return NULL;
}



int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore, char *simbolo) 
{
    int indice = mossa -1;

    if (partita->griglia[indice] == 'X' || partita->griglia[indice] == 'O') 
    {  
        printf("Mossa non valida: la cella è già occupata.\n");
        return 0;
    }
    
    partita->griglia[indice] = *simbolo;  
    return 1;
}



int controlla_vittoria(char g[N], char *simbolo) {
    // Controlla righe
    for (int i = 0; i < 9; i += 3) {
        if (g[i] == *simbolo && g[i + 1] == *simbolo && g[i + 2] == *simbolo)
            return 1;
    }

    // Controlla colonne
    for (int i = 0; i < 3; i++) {
        if (g[i] == *simbolo && g[i + 3] == *simbolo && g[i + 6] == *simbolo)
            return 1;
    }

    // Controlla diagonali
    if ((g[0] == *simbolo && g[4] == *simbolo && g[8] == *simbolo) ||
        (g[2] == *simbolo && g[4] == *simbolo && g[6] == *simbolo)) {
        return 1;
    }

    return 0; // Nessuna vittoria trovata
}



int controlla_pareggio(char g[N]) {
    for (int i = 0; i < N; i++) {
        if (g[i] != 'X' && g[i] != 'O') {
            return 0; // C'è almeno una cella libera → Non è un pareggio
        }
    }
    return 1; // Nessuna cella libera → È un pareggio
}
