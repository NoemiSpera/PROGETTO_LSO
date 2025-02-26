#include "header_file/modelli_client.h"
#include "header_file/colori.h"


#ifndef VARIABILI_H
#define VARIABILI_H

#define MAX 1024
#define MAX_NOME 50

#endif

int main()
{
    int client_fd;
    char buffer[MAX];
    char *nome = malloc(MAX_NOME * sizeof(char));
    char scelta;
    char crea_partita[1];
   
    
    //connessione al server
    client_fd=connetti_al_server();
    
    //invio del nome al server e ricezione del messaggio di benvenuto
    nome = inserisci_nome();
    invia_messaggi(client_fd,nome);
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    //richiesta di creazione o patecpazione da altre partite
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf(UNDERLINE YELLOW "%s" RESET, buffer);
    
    
    // Invio risposta al server
    printf("Inserisci la tua scelta: \n");
    printf("C) Crea partita\n");
    printf("U) UNisciti ad una partita esistente\n");
    scanf(" %c", &scelta);
    char msg[2]={scelta, '\0'};
    invia_messaggi(client_fd, msg);

   
    close(client_fd);
    return 0;
}