#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <arpa/inet.h> 

#define MAX_NOME 50
#define MAX 1024
#define N 20

//funzione per la connessione
int connetti_al_server();


//funzioni per la comunicazione 
ssize_t ricevi_messaggi(int client_fd, char* buffer, size_t buf_size);
void invia_messaggi(int client_fd, char *msg);


//funzioni per input da tastiera
char *inserisci_nome();

//funzioni per l'assegnazione della partita
int scelta_partecipazione(int client_fd,char *nome);
int gioca_con_amico(int client_fd, char *nome);
void stampa_partite(char *buffer);
int gestisci_richiesta_partecipazione(int client_fd);

//funzioni per la gestione partita della partita
void stampa_griglia(char griglia[N]);
int gestisci_partita(int client_fd);
void gestisci_opzioni_post_partita(int client_fd);

void *ascolta_notifiche(void *arg);


