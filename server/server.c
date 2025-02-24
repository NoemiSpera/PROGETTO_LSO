#include "modelli_server.h"
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
    

    printf("Inizio programma\n");
    creazione_socket(&server_socket);
    accetta_connessioni(server_socket,partite,&num_partite);


    return 0;
}