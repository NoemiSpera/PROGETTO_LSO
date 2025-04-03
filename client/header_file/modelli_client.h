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
void nascondi_input();
void ripristina_input();
char *inserisci_nome();

//funzioni per la gestione partita della partita
void stampa_griglia(char griglia[N]);
void gestisci_partita(int client_fd);


