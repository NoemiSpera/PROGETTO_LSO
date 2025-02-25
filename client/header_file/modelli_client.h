#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

//funzioni per la connessione
int connetti_al_server();


//funzioni per la cominicazione 
void ricevi_messaggi(int client_fd, char* buffer, size_t buf_size);
void invia_messaggi(int client_fd, char *msg);