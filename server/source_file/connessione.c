#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

#define PORTA 12345
#define MAX 1024
#define NMAX_NOME 50



void creazione_socket(int *server_fd) {
    struct sockaddr_in server_addr;

    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == -1) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORTA);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind() failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, 5) == -1) {
        perror("listen() failed");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
}

void accetta_connessioni(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd == -1) {
            perror("accept() failed");
            continue;
        }

        pthread_t thread;
        int *nuova_socket = malloc(sizeof(int));
        if (nuova_socket == NULL) {
            perror("malloc() failed");
            close(client_fd);
            continue;
        }

        *nuova_socket = client_fd;
        if (pthread_create(&thread, NULL, gestisci_client, (void *)nuova_socket) != 0) {
            perror("pthread_create() failed");
            free(nuova_socket);
            close(client_fd);
            continue;
        }
        pthread_detach(thread);
    }

    close(server_fd);
}

