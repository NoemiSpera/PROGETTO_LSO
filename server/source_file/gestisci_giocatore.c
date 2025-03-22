#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


//gestione client
void *gestisci_client(void *arg)
{
    
    int client_socket = *(int*)arg;
    free(arg);

    int rec = 0;
    int indice_gioco=-1;
    char nome[MAX_NOME];
    char buffer[MAX];
    
    
    rec = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
    printf("%s%s ha fatto accesso al server%s\n",CYAN, buffer, RESET);
    strcpy(nome,buffer);

    Giocatori *giocatore = inizializza_giocatore(client_socket,-1,buffer, "-1");

    char benvenuto[256];
    snprintf(benvenuto, sizeof(benvenuto), "%s%sCiao %.200s, benvenuto nel server!%s", CYAN, BOLD, buffer, RESET);

    invia_messaggi(client_socket, benvenuto);
    usleep(500000);
    
    char msg[] = "Vuoi creare una nuova partita(C) o unirti  a una esistente (U)?\n";
    invia_messaggi(client_socket, msg);
   
    printf("Attendo ricezione del messaggio...\n");

    int bytes_ricevuti = ricevi_messaggi(client_socket, buffer, sizeof(buffer));

   

    if (bytes_ricevuti > 0) {
        
        buffer[bytes_ricevuti] = '\0';  // Assicura una stringa valida
        gestisci_scelta(giocatore, buffer[0]);
        
    } else {
        printf("Errore nel ricevere la scelta\n");
        free(giocatore);
        close(client_socket);
        return NULL;
    }

    //close(client_socket);    
}



void gestisci_scelta(Giocatori *giocatore, char scelta)
{
        switch (scelta)
        {
            case 'C':
            case 'c':
                printf("%s ha scelto di creare una nuova partita\n", giocatore->nome);
                printf("Creazione in corso...\n");
                gestisci_creazione_partita(giocatore);
                break;
        
            case 'U':
            case 'u':
                printf("%s ha scelto di unirsi ad una partita esistente\n", giocatore->nome);
                gestisci_ingresso_partita(giocatore);
                break;

            default:
                invia_messaggi(giocatore->socket, "Scelta non valida\n");
                break;
        }
}
