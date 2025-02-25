#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#ifndef VARIABILI_H
#define VARIABILI_H

#define MAX 256

#endif

int main()
{
    int client_fd;
    char buffer[MAX];
    char scelta;
    
    //connessione al server
    client_fd=connetti_al_server();
    
    //messaggio dal server
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf(UNDERLINE YELLOW "Messaggio del server: %s\n"RESET, buffer);
    
    
    // Invio risposta al server
    printf("Inserisci la tua scelta: \n");
    printf("C) Crea partita\n");
    printf("U) UNisciti ad una partita esistente\n");
    scanf(" %c", &scelta);
    char msg[2]={scelta, '\0'};
    invia_messaggi(client_fd, msg);

    //ricezione risposta dal server
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
   

   
    close(client_fd);
    return 0;
}