#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

/*variabili globali*/

//mutex
pthread_mutex_t partite_mutex;


//gestione partite e giocatori
Giocatori *giocatore[MAX_COLLEGATI];
Partita *partite[MAX_PARTITE];


//inizializzazioni
void inizializza_griglia(char griglia[N][N])
{
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            griglia[i][j]='-';
        }
    }
}



Giocatori *inizializza_giocatore(int socket, int id_partita, char* nome, char *simbolo)
{
    
    Giocatori *giocatore=(Giocatori*)malloc(sizeof(Giocatori));
    if(!giocatore) return NULL;

    giocatore->socket=socket;
    strncpy(giocatore->nome,nome,MAX_NOME);
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

   nuova_partita->giocatore[0] = giocatore1;;
   nuova_partita->giocatore[1] = NULL;

   inizializza_griglia(nuova_partita->griglia);

    nuova_partita->stato = 0;                  // 0 = nuova creazione
    nuova_partita->turno = 0;                  // 0 = non è il tuo turno
    nuova_partita->risultato =-1;              // -1 = ancora nessun risultato
    nuova_partita->thread = 0;                 // nessun thread avviato

   
   return nuova_partita;
}



void crea_partita(int client_fd, char *nome_giocatore)
{
    
    pthread_mutex_lock(&partite_mutex);
    int id_partita = -1;

   //posto libero nell'array delle partite
   for(int i = 0; i < MAX_PARTITE; i++ )
   {    
        
        if(partite[i] == NULL)
        {
            
            id_partita = i;
            break;
        }
   }
   
   if(id_partita == -1)
   {
    pthread_mutex_unlock(&partite_mutex); 
    invia_messaggi(client_fd,"Limite massimo di partite raggiunto\n");
    return;
   }

    Partita *nuova_partita=inizializza_partita(id_partita, client_fd, nome_giocatore); 
    if (!nuova_partita) {
        pthread_mutex_unlock(&partite_mutex);
        invia_messaggi(client_fd, "Errore nella creazione della partita.\n");
        return;
    }
     
    
    partite[id_partita] = nuova_partita;

    pthread_mutex_unlock(&partite_mutex);

    char creazione[MAX];
    snprintf(creazione, sizeof(creazione), "Partita creata con ID %d. In attesa di un altro giocatore...\n", id_partita);
    invia_messaggi(client_fd, creazione);
}
