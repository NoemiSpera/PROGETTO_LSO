#include "header_file/modelli_client.h"
#include "header_file/colori.h"


int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    int client_fd;
    char *nome;
    char buffer[MAX];
    char buffer_griglia[MAX];
    char scelta[10];
    
    // Usa "myserver" come default se non viene passato nulla
    const char *server_ip = (argc > 1) ? argv[1] : "myserver";
    
    // Connessione al server
    client_fd = connetti_al_server(server_ip);

    pthread_t thread_notifiche;
    if (pthread_create(&thread_notifiche, NULL, ascolta_notifiche, (void *)&client_fd) != 0) {
        perror("Errore creazione thread per le notifiche");
    }   

    // Invio del nome al server e ricezione del messaggio di benvenuto
    nome = inserisci_nome();
    invia_messaggi(client_fd, nome);
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s", buffer);

    while (1)
    {
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));
        printf(YELLOW "%s" RESET, buffer);
        printf(GREEN "Inserisci la tua scelta : " RESET);
        scanf(" %s",scelta);

        invia_messaggi(client_fd, scelta);

        int partita_disponibile = 0;
        char opzione = scelta[0];
        
        switch (opzione)
        {
            case 'C':
            case 'c':
                partita_disponibile = richiesta_partecipazione(client_fd);
                break;
            case 'A':
            case 'a':
                partita_disponibile = gioca_con_amico(client_fd);
                break;
            case 'B':
            case 'b':
                partita_disponibile = partita_casuale(client_fd);
                break;
            case 'Q':
            case 'q':
                printf("Uscita in corso......\n");
                pthread_cancel(thread_notifiche);  
                pthread_join(thread_notifiche, NULL); 
                close(client_fd);       
                free(nome);             
                exit(0);   
            default:
                ricevi_messaggi(client_fd,buffer,sizeof(buffer));
                printf(RED"%s"RESET,buffer);
                break;
        }      

        if (partita_disponibile) {
            gestisci_partita(client_fd);

        }  
    }

    close(client_fd);
    return 0;
}