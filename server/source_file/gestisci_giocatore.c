#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


/*variabili globali*/

//mutex
pthread_mutex_t lock;


//gestione client
void *gestisci_client(void *arg)
{
    
    int client_socket = *(int*)arg;
    free(arg);

    int rec = 0;
    int indice_gioco=-1;
    char nome[MAX_NOME];
    char buffer[MAX];
    memset(buffer, 0, sizeof(buffer));
    
    rec = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
    printf("%s ha fatto accesso al server\n", buffer);
    strcpy(nome,buffer);

    char benvenuto[256];
    snprintf(benvenuto, sizeof(benvenuto), "Ciao %.200s, benvenuto nel server!", buffer);
    invia_messaggi(client_socket, benvenuto);
    usleep(500000);
    
    char msg[] = "Vuoi creare una nuova partita(C) o unirti  a una esistente U)?\n";
    invia_messaggi(client_socket, msg);
    
    memset(buffer, 0, sizeof(buffer));
    if(ricevi_messaggi(client_socket, buffer, sizeof(buffer)) > 0)
    {
        pthread_mutex_lock(&lock);
        gestisci_scelta(client_socket, buffer[0], nome);
        pthread_mutex_unlock(&lock);
    }

    close(client_socket);    
}



void gestisci_scelta(int client_fd, char scelta, char *nome_client)
{
    char buffer[MAX];
    switch (scelta)
    {
        case 'C':
        case 'c':
            printf("%s ha scelto di creare una nuova partita\n", nome_client);
            printf("Creazione in corso...\n");
            crea_partita(client_fd, nome_client);
            printf("La partita è stata creata con successo!\n");
            printf("%s è in attesa di un altro giocatore...\n", nome_client);
            break;
        
        case 'U':
        case 'u':
            printf("%s ha scelto di unirsi ad una partita esistente\n", nome_client);
            //unisciti_ad_una_partita();
            break;

        default:
            invia_messaggi(client_fd, "Scleta non valida");
            break;
    }
}
