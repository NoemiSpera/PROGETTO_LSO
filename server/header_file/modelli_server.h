#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>



#define N 3
#define MAX_NOME 50
#define MAX_GIOCATORI 2
#define MAX_PARTITE 100
#define MAX_COLLEGATI 100
#define MAX_LOGICA 100
#define LARGHEZZA 60
#define MAX 1024

//variabili globali
 extern pthread_mutex_t lock;

//strutture
typedef struct 
{
    int socket;
    char nome[MAX_NOME];
    int id_partita;
    int stato;                  /* 0 = "non è il suo turno" | 1 = "è il suo turno" */
    int simbolo;                /* 0 = "O" | 1 = "X" | -1 = "nessun simbolo assegnato" */
} Giocatori;



typedef struct 
{
    int id;
    Giocatori *giocatore[MAX_GIOCATORI];
    char griglia[N][N];
    int stato; 
    int turno;
    int risultato;
    pthread_t thread;
} Partita;



//connessione col client in connessione.c
void creazione_socket(int *server_fd);
void accetta_connessioni(int server_fd);


//messaggi su stdout
void stampa_bordo();
void stampa_testo_centrato(const char *testo);
void messaggio_benvenuto();

//inizializzazioni
void inizializza_griglia(char griglia[N][N]);
Giocatori *inizializza_giocatore(int socket, int id_partita, char *nome, char *simbolo);
Partita *inizializza_partita(int id_partita, int socket_giocatore, char *nome_giocatore);


//gestione client
void *gestisci_client(void *arg);
void gestisci_scelta(int client_fd, char scelta, char *nome_client);

//gestione del gioco
void crea_partita(int client_fd, char *nome);

//scambio messaggi
void invia_messaggi(int client_fd, char *msg);
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size);


