#include "header_file/modelli_client.h"
#include "header_file/colori.h"


int main()
{
    int client_fd;
    char buffer[MAX];
    char *nome;
    char scelta;
    char buffer_griglia[MAX];
   


    //connessione al server
    client_fd=connetti_al_server();
    
    //invio del nome al server e ricezione del messaggio di benvenuto
    nome = inserisci_nome();
    invia_messaggi(client_fd,nome);
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);


    //richiesta di creazione o partecpazione ad altre partite
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf(YELLOW "%s" RESET, buffer);
    printf(GREEN "Inserisci la tua scelta : " RESET);
    
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
        
     
        gestisci_partita(client_fd);
        //messaggio partita creata
         //invio messaggio al client il problema è che dovrebbero essere inviati 2 messaggi
        //per entrambi i client, inviando un solo mex la ricezione dei mex tra un client e l'altro
        //risulta sbagliata
        //soluzione: in questo punto inviare un mex al client che la partita è stata creata
        //e a tutti gli altri client anche. il mex è nella funzione gestisci_creazione_partita
        /*printf("DEBUG: RICEZIONE DEL MESSAGGIOo...\n");
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));
        printf(YELLOW BOLD "%s" RESET, buffer);
        printf("DEBUG: mex ric...\n");*/  

    }

    close(client_fd);
    free(nome);
    return 0;
}