#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"

#define PORTA 12345

int connetti_al_server(const char *server_ip) {
    int client_fd;
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORTA);

    // Prova prima come IP, poi come hostname
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        // Non è un IP, prova come hostname
        struct hostent *he = gethostbyname(server_ip);
        if (he == NULL) {
            perror("gethostbyname fallita");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
        memcpy(&server_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connessione fallita");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}