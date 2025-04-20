#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
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


 //strutture
typedef struct 
{
    int socket;
    char nome[MAX_NOME];
    int id_partita;
    int stato;                  /* 0 = "non è il suo turno" | 1 = "è il suo turno" */
    int in_partita;
    char simbolo[10];                /* 0 = "O" | 1 = "X" */
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


//connessione col client in connessione.c
void creazione_socket(int *server_fd);
void accetta_connessioni(int server_fd);


//messaggi su stdout
void stampa_bordo();
void stampa_testo_centrato(const char *testo);
void messaggio_benvenuto();
void invia_menu_principale(int client_fd);



//scambio messaggi
void invia_messaggi(int client_fd, char *msg);
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size);



//inizializzazioni
void inizializza_griglia(char griglia[N]);
Giocatori *inizializza_giocatore(int socket, int id_partita, char *nome, char *simbolo);
Partita *inizializza_partita(int id_partita, Giocatori *giocatore);
int generazione_id(Giocatori *giocatore);
void attendi_giocatore(int id_partita, Partita* partita, Giocatori *giocatore);
Partita *cerca_partita_disponibile(Giocatori *giocatore);
void unisci_a_partita(Partita *partita, Giocatori *giocatore);


//gestione delle liste
void inizializza_lista();
void aggiungi_partita(Partita *nuova_partita);
void rimuovi_partita(int id);
void stampa_partite();
void conversione_lista_partite(char *buffer, size_t dim_max);

//gestione client
void *gestisci_client(void *arg);
void *gestisci_gioco(void *arg);

//gestione assegnazione partita
int gestisci_scelta(Giocatori *giocatore, char scelta);
int assegnazione_amico(Giocatori *giocatore);
Partita* trova_partita(int id_partita);
int notifica_creatore(Partita *partita,Giocatori *creatore,Giocatori *giocatore);


//gestione del gioco
Partita *creazione_partita(Giocatori *giocatore);
void avvia_thread_partita(Partita *partita);
int ricevi_mossa(Giocatori *g);
int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore, char *simbolo);
int controlla_vittoria(char g[N], char *simbolo);
int controlla_pareggio(char g[N]);
void chiedi_nuova_partita(Giocatori *giocatore);



void fine_partita(Giocatori *giocatore);