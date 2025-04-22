#include "header_file/modelli_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>




int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0); 
    int server_socket;

    inizializza_lista();
    
    creazione_socket(&server_socket);

    messaggio_benvenuto();
    
    accetta_connessioni(server_socket);

    close(server_socket);
    return 0;
}