#include "header_file/modelli_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>



int main()
{
    int server_socket;

    inizializza_lista();
    
    creazione_socket(&server_socket);

    messaggio_benvenuto();
    
    accetta_connessioni(server_socket);

    close(server_socket);
    return 0;
}