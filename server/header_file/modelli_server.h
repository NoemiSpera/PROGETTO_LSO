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

#ifndef VARIABILI_H
#define VARIABILI_H

#define N 3
#define MAX_NOME 50
#define MAX_GIOCATORI 2

#endif

//variabili
extern pthread_mutex_t lock;




//strutture
typedef struct 
{
    int socket;
    char nome[MAX_NOME];
    int id_partita;
    int stato;                  /* 0 = "non è il suo turno" | 1 = "è il suo turno" */
    int simbolo;                /* 0 = "O" | 1 = "X" | -1 = "nessun simbolo assegnato" */
} Giocatore;



typedef struct 
{
    int id_partita;
    Giocatore *giocatori[MAX_GIOCATORI];
    char griglia[N][N];
    pthread_t thread;
} Partita;



typedef struct 
{
    Partita *partita;
    int stato;                  /* 0 = "nuova_creazione" | 1 = "in corso" | 2 = "terminata" */
    int turno;                  /* 0 = "non è il tuo turno" | 1 = "è il tuo turno" */
    int risultato;              /* 0 = "pareggio" | 1 = "vittoria giocatore 1" | 2 = "vittoria giocatore 2" | -1 = "ancora nessun risultato" */
    pthread_mutex_t lock;
} Logica_partita;



//connessione col client in connessione.c
void creazione_socket(int *server_fd);
void accetta_connessioni(int server_fd,Partita *partite[],int* numero_partite);


//messaggi su stdout
void stampa_bordo();
void stampa_testo_centrato(const char *testo);
void messaggio_benvenuto();

//inizializzazioni
void inizializza_griglia(Partita *partita);
void inizializza_giocatore(Giocatore *giocatore, int socket, int id_partita, const char *nome);
void inizializza_logica_partita(Logica_partita *logica, Partita *partita);


//gestione client
void *gestisci_client(void *arg);
void gestisci_scelta(int client_fd, char scelta, char *nome_client);


//scambio messaggi
void invia_messaggi(int client_fd, char *msg);
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size);


