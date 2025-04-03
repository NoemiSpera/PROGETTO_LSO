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
    char simbolo[10];                /* 0 = "O" | 1 = "X" */
} Giocatori;



typedef struct 
{
    int id;
    Giocatori *giocatore[MAX_GIOCATORI];
    char griglia[N];
    int stato;                    /* 1 = "terminata"  |  0 = "in corso" |  -1 = creata */
    int turno;
    int risultato;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Partita;



//connessione col client in connessione.c
void creazione_socket(int *server_fd);
void accetta_connessioni(int server_fd);


//messaggi su stdout
void stampa_bordo();
void stampa_testo_centrato(const char *testo);
void messaggio_benvenuto();



//scambio messaggi
void invia_messaggi(int client_fd, char *msg);
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size);



//inizializzazioni
void inizializza_griglia(char griglia[N]);
Giocatori *inizializza_giocatore(int socket, int id_partita, char *nome, char *simbolo);
Partita *inizializza_partita(int id_partita, int socket_giocatore, char *nome_giocatore);
int generazione_id(Giocatori *giocatore);
Partita *genera_partita(int id_partita, Giocatori *giocatore);
void attendi_giocatore(int id_partita, Partita* partita, Giocatori *giocatore);
Partita *cerca_partita_disponibile(Giocatori *giocatore);
void unisci_a_partita(Partita *partita, Giocatori *giocatore);


//gestione client
void *gestisci_client(void *arg);
void *gestisci_gioco(void *arg);
void gestisci_scelta(Giocatori *giocatore, char scelta);

//gestione del gioco
Partita *gestisci_creazione_partita(Giocatori *giocatore);
void gestisci_ingresso_partita(Giocatori *giocatore);
int ricevi_mossa(Giocatori *g);
int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore, char *simbolo);
int controlla_vittoria(char g[N], char *simbolo);
int controlla_pareggio(char g[N]);