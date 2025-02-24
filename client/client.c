#include "modelli_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

int main()
{

    int client_fd=connetti_al_server();

    char scelta;
    printf("Vuoi (C)reare o (U)nirti a una partita? ");
    scanf(" %c", &scelta);  // Nota lo spazio prima di %c per consumare eventuali newline

    // Invia la scelta al server
    write(client_fd, &scelta, sizeof(scelta));

    // In base alla risposta del server, puoi continuare la logica del gioco
    // ...

    close(client_fd);


    return 0;
}