#include "header_file/modelli_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>


#define MAX_PARTITE 100

int main()
{
    int server_socket;
    Partita *partite[MAX_PARTITE];
    int num_partite = 0;
    pthread_mutex_init(&lock, NULL);
    

    creazione_socket(&server_socket);

    messaggio_benvenuto();
    
    accetta_connessioni(server_socket,partite,&num_partite);

    
    pthread_mutex_destroy(&lock);
    return 0;
}