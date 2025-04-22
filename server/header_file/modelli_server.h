#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <stdatomic.h>
#include <arpa/inet.h> 


#define N 9
#define MAX_NOME 50
#define MAX_GIOCATORI 2
#define MAX_PARTITE 100
#define MAX_COLLEGATI 100
#define LARGHEZZA 60
#define MAX 1024

//mutex per la lista globale delle partita 
extern pthread_mutex_t partite_mutex;
extern atomic_int num_partite;



 //strutture
typedef struct 
{
    int socket;
    char nome[MAX_NOME];
    int id_partita;
    int stato;                  /* 0 = "non è il suo turno" | 1 = "è il suo turno" */
    int in_partita;
    char simbolo[10];              
} Giocatori;

typedef enum { NUOVA, IN_ATTESA, IN_CORSO, TERMINATA } StatoPartita;

typedef enum { NESSUNO, VITTORIA_SCONFITTA, PAREGGIO } EsitoPartita;

typedef struct Partita
{
    int id;
    Giocatori *giocatore[MAX_GIOCATORI];
    char griglia[N];
    StatoPartita stato;                    
    int turno;
    EsitoPartita risultato;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    struct Partita *next;
} Partita;



typedef struct 
{
    Partita *head;
    pthread_mutex_t mutex;
} ListaPartite;


extern ListaPartite *lista_partite;
extern Giocatori *giocatori_connessi[MAX_COLLEGATI];


//connessione col client in connessione.c
void creazione_socket(int *server_fd);
void accetta_connessioni(int server_fd);


//messaggi su stdout in impl_server.c 
void stampa_bordo();
void stampa_testo_centrato(const char *testo);
void messaggio_benvenuto();
void invia_menu_principale(int client_fd);


//scambio messaggi in impl_server.c
void invia_messaggi(int client_fd, char *msg);
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size);
void messaggio_broadcast(Giocatori *creatore, int id_partita);


//gestione delle liste in impl_servr.c
void inizializza_lista();
void aggiungi_partita(Partita *nuova_partita);
void rimuovi_partita(int id);
void stampa_partite();
int conversione_lista_partite(char *buffer, size_t dim_max);
void aggiungi_giocatore(Giocatori* nuovo);
void rimuovi_giocatore(int socket_fd);


//gestione della partita in gestione_partite.c
void inizializza_griglia(char griglia[N]);
Partita *inizializza_partita(int id_partita, Giocatori *giocatore);
Giocatori *inizializza_giocatore(int socket, int id_partita, char *nome, char *simbolo);

void *gestisci_gioco(void *arg);




//gestione client per l'assegnazione della partita in gestisci_client.c
void *gestisci_client(void *arg);
int gestisci_scelta(Giocatori *giocatore, char scelta);
Partita *creazione_partita(Giocatori *giocatore);
int generazione_id(Giocatori *giocatore);
void attendi_giocatore(int id_partita, Partita* partita, Giocatori *giocatore);
int assegnazione_amico(Giocatori *giocatore);
int assegnazione_casuale(Giocatori *giocatore);
Partita *cerca_partita_disponibile(Giocatori *giocatore);
Partita* trova_partita(int id_partita);
int notifica_creatore(Partita *partita,Giocatori *creatore,Giocatori *giocatore);


//gestione del gioco in gestisci_partite.c
void avvia_thread_partita(Partita *partita);
int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore, char *simbolo);
int controlla_vittoria(char g[N], char *simbolo);
int controlla_pareggio(char g[N]);