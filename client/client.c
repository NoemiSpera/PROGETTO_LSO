#include "header_file/modelli_client.h"
#include "header_file/colori.h"


int main()
{
    int client_fd;
    char *nome;
    char buffer[MAX];
    char buffer_griglia[MAX];
    char scelta[10];
    char id[10];

    // Connessione al server
    client_fd = connetti_al_server();

    // Invio del nome al server e ricezione del messaggio di benvenuto
    nome = inserisci_nome();
    invia_messaggi(client_fd, nome);
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s", buffer);

    while (1)
    {
        // Menu principale
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));
        printf(YELLOW "%s" RESET, buffer);
        printf(GREEN "Inserisci la tua scelta : " RESET);
        scanf(" %s",scelta);

        invia_messaggi(client_fd, scelta);

        int partita_disponibile = 0;

        if(scelta[0] == 'C' || scelta[0] == 'c')
        {   
            partita_disponibile = gestisci_richiesta_partecipazione(client_fd);

        } else if ( scelta[0] == 'A' || scelta[0] == 'a')
        {
            //lista partite
            ricevi_messaggi(client_fd,buffer,sizeof(buffer));
            printf("%s",buffer);
            printf(GREEN "Inserisci l'id della partita a cui vuoi partecipare: "RESET);
            scanf(" %s",id);
            invia_messaggi(client_fd,id);
            ricevi_messaggi(client_fd,buffer,sizeof(buffer));
            printf("%s",buffer);
            if (strstr(buffer, "accettata!") != NULL)
            {
                partita_disponibile = 1;
            }
        }

        
        if(scelta[0] == 'Q' || scelta[0] == 'q')
        {   
            printf("Uscita in corso......\n");
            close(client_fd);       
            free(nome);             
            exit(0);   
        }
               

        if (partita_disponibile) {
            gestisci_partita(client_fd);

        }  
    }

    close(client_fd);
    free(nome);
    return 0;
}
