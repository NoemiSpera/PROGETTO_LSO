#include "header_file/modelli_client.h"
#include "header_file/colori.h"


int main()
{
    int client_fd;
    char buffer[MAX];
    char *nome;
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
    printf("Inserisci la tua scelta : ");
    
    while (1)
    {
        //Invio risposta al server
        scanf(" %c", &scelta);
        
        
        if (scelta == 'Q' || scelta == 'q') {
            printf("Uscendo dal programma...\n");
            break;
        }

        char input[2]={scelta, '\0'};
        invia_messaggi(client_fd, input);
        

        //messaggio partita creata
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));
        printf("%s", buffer);

    }
   

    close(client_fd);
    free(nome);
    return 0;
}