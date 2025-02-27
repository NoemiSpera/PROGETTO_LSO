#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"


#define PERCORSO_SOCKET "/tmp/socket_locale"
#define MAX_BUF 256



int connetti_al_server()
{

    int client_fd;
    struct sockaddr_un server_addr;

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) 
    {
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

    return client_fd;
}