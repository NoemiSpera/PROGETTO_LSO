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


//funzione per la connessione
int connetti_al_server();


//funzioni per la comunicazione 
void ricevi_messaggi(int client_fd, char* buffer, size_t buf_size);
void invia_messaggi(int client_fd, char *msg);

//funzioni per input da tastiera
void nascondi_input();
void ripristina_input();
char *inserisci_nome();