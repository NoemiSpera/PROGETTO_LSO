#include "modelli_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <pthread.h>

#define PERCORSO_SOCKET "/tmp/socket_locale"
#define MAX_NOME 50

int connetti_al_server()
{

    int client_fd;
    struct sockaddr_un server_addr;
    char nome[MAX_NOME];

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Errore nella creazione del socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, PERCORSO_SOCKET, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Connessione fallita");
        close(client_fd);
        exit(1);
    }

    printf("Connesso al server!\n");

    // Richiedi al giocatore di inserire il proprio nome
    printf("Inserisci il tuo nome: ");
    fgets(nome, MAX_NOME, stdin);
    nome[strcspn(nome, "\n")] = 0;  // Rimuovi il carattere di newline

    // Invia il nome al server
    write(client_fd, nome, strlen(nome) + 1);

    return client_fd;
}