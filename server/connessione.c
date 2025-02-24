#include "modelli_server.h"
#include <sys/socket.h>
#include <sys/un.h>

#define PERCORSO_SOCKET "/tmp/socket_locale"

void creazione_socket(int *server_fd)
{
    struct sockaddr_un server_addr;
    
    *server_fd=socket(AF_LOCAL,SOCK_STREAM,0);
    if(*server_fd==-1)
    {
        perror("socket() failed\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family= AF_LOCAL; 
    strncpy(server_addr.sun_path, PERCORSO_SOCKET, sizeof(server_addr.sun_path) -1); 
    unlink(PERCORSO_SOCKET);

    if(bind(*server_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un))== -1)
    {

        perror("bind() failed\n");
        exit(EXIT_FAILURE);

    }

    if(listen(*server_fd, 5)==-1)
    {

        perror("listen() failed\n");
        exit(EXIT_FAILURE);

    }

    printf("Server in ascolto su %s\n", PERCORSO_SOCKET);  //solo per prova
 
}


void accetta_connessioni(int server_fd, Partita *partite[],int* numero_partite)
{
    struct sockaddr_un client_addr;
    socklen_t client_addr_len=sizeof(struct sockaddr_un);
    int client_fd=accept(server_fd,(struct sockaddr*)&client_addr, &client_addr_len);

    if (client_fd == -1) {
        perror("Errore nell'accept");
        exit(1);
    }

    printf("Connessione accettata\n");

    char nome[MAX_NOME];
    int len = recv(client_fd, nome, sizeof(nome), 0);
    if (len == -1) {
        perror("Errore nella ricezione del nome");
        close(client_fd);
        return;
    }

    nome[len] = '\0'; // Assicurarsi che la stringa sia terminata
    printf("Nome ricevuto dal client: %s\n", nome);

    // Chiusura del socket
    close(client_fd);

}

void *gestisci_partita(void *arg)
{
    Partita *partita = (Partita *)arg;
    printf("Partita %d iniziata!\n", partita->id_partita);

    // Simulazione del gioco (da implementare con logica completa)
    sleep(10);

    printf("Partita %d terminata!\n", partita->id_partita);
    
}