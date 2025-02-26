#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

#define PERCORSO_SOCKET "/tmp/socket_locale"
#define MAX 1024
#define NMAX_NOME 50



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
