#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"
#include <sys/socket.h>
#include <sys/un.h>

#define PERCORSO_SOCKET "/tmp/socket_locale"
#define MAX 256

//variabili globali
pthread_mutex_t lock;



void creazione_socket(int *server_fd)
{
    struct sockaddr_un server_addr;
    
    *server_fd = socket(AF_LOCAL,SOCK_STREAM,0);
    if(*server_fd == -1)
    {
        perror("socket() failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_LOCAL; 
    strncpy(server_addr.sun_path, PERCORSO_SOCKET, sizeof(server_addr.sun_path) -1); 
    unlink(PERCORSO_SOCKET);

    if(bind(*server_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind() failed\n");
        exit(EXIT_FAILURE);
    }

    if(listen(*server_fd, 5) == -1)
    {
        perror("listen() failed\n");
        exit(EXIT_FAILURE);
    }
 
}



void accetta_connessioni(int server_fd, Partita *partite[],int* numero_partite)
{
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_un);
    int client_fd;

    while(1)
    {
        client_fd = accept(server_fd,(struct sockaddr*)&client_addr, &client_addr_len);

        if (client_fd == -1) 
        {
            perror("Errore nell'accept");
            exit(1);
        }

        pthread_t thread;
        int *nuova_socket = malloc(sizeof(int));
        if (nuova_socket == NULL) 
        {
            perror("Errore nell'allocazione della memoria");
            exit(1);
        }
        *nuova_socket=client_fd;
        if(pthread_create(&thread, NULL, gestisci_client, (void*)nuova_socket) != 0)
        {
            perror("Errore nella creazione del thread!\n");
            free(nuova_socket);
            close(client_fd);
            continue;
        }
        pthread_detach(thread);
    }
    
   // Chiusura del socket
    close(server_fd);
}



void *gestisci_client(void *arg)
{
    
    int client_socket = *(int*)arg;
    
    printf("Thread in esecuzione per client_fd: %d\n", client_socket);
    free(arg);

    char buffer[MAX];
    memset(buffer, 0, sizeof(buffer));
    
    int indice_gioco=-1;
    
    char msg[] = "Vuoi creare una nuova partita(C) O unirti  a una esistente (U)?\n ";
    invia_messaggi(client_socket, msg);
    
    if(ricevi_messaggi(client_socket, buffer, sizeof(buffer)) > 0)
    {
        pthread_mutex_lock(&lock);
        gestisci_scelta(client_socket, buffer[0]);
        pthread_mutex_unlock(&lock);
    }

    close(client_socket);    
}

void gestisci_scelta(int client_fd, char scelta)
{
    switch (scelta)
    {
        case 'C':
        case 'c':
            printf("creo la partita\n");
            //creazione_partita();
            break;
        
        case 'U':
        case 'u':
            printf("mi unisco ad una partita esistente\n");
            //unisciti_ad_una_partita();
            break;

        default:
            invia_messaggi(client_fd, "Scleta non valida");
            break;
    }
}

ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n;
    
    memset(buffer, 0, buf_size); // Puliamo il buffer
    n = recv(client_fd, buffer, buf_size - 1, 0);
    printf(UNDERLINE YELLOW "Messaggio del client: %s\n"RESET, buffer);
    if(n == -1)
    {
        perror("Errore nella ricezione del messaggio");
        close(client_fd);
        exit(1);
    }
    if (n > 0) 
    {
        buffer[n] = '\0';
    }
    return n;
}


void invia_messaggi(int client_fd, char *msg)
{
    if(send(client_fd, msg, strlen(msg) + 1, 0) == -1)
    {
        perror("Errore nell'invio della risposta al server\n");
        close(client_fd);
        exit(1);
    }
}

