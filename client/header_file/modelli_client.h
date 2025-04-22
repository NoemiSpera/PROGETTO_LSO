#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <arpa/inet.h> 

#define MAX_NOME 50
#define MAX 1024
#define N 20

//funzione per la connessione in connessione.c
int connetti_al_server(const char *server_ip);


//funzioni per la comunicazione in impl_client.c
void *ascolta_notifiche(void *arg);
ssize_t ricevi_messaggi(int client_fd, char* buffer, size_t buf_size);
void invia_messaggi(int client_fd, char *msg);


//funzioni per input da tastiera in impl_client.c
char *inserisci_nome();

//funzioni per l'assegnazione della partita n gestisci_gioco.c
int richiesta_partecipazione(int client_fd);
int gioca_con_amico(int client_fd);
int partita_casuale(int client_fd);
int gestisci_partita(int client_fd);
void stampa_griglia(char griglia[N]);
void stampa_partite(char *buffer);