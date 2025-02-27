#include "header_file/modelli_client.h"
#include "header_file/colori.h"


#define MAX 1024
#define MAX_NOME 50



int main()
{
    int client_fd;
    char buffer[MAX];
    char *nome = malloc(MAX_NOME * sizeof(char));
    char scelta;

    
    //connessione al server
    client_fd=connetti_al_server();
    
    //invio del nome al server e ricezione del messaggio di benvenuto
    nome = inserisci_nome();
    invia_messaggi(client_fd,nome);
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    //richiesta di creazione o patecpazione ad altre partite
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf(UNDERLINE YELLOW "%s" RESET, buffer);
    
    
    // Invio risposta al server
    printf("Inserisci la tua scelta: \n");
    printf("C) Crea partita\n");
    printf("U) UNisciti ad una partita esistente\n");
    scanf(" %c", &scelta);
    char msg[2]={scelta, '\0'};
    invia_messaggi(client_fd, msg);

    //messaggio partita creata
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s", buffer);

    
   
    close(client_fd);
    return 0;
}