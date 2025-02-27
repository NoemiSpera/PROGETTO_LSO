#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"


//messaggi su stdout
void messaggio_benvenuto()
{
    stampa_bordo();
    printf("\n");
    stampa_testo_centrato(BOLD CYAN"BENVENUTO NEL SERVER DEL TRIS" RESET);
    stampa_testo_centrato(BOLD BLUE"TRE SEGNI, UN VINCITORE. SARAI TU?\n" RESET);
    stampa_bordo();
}

void stampa_bordo()
{
    for(int i=0; i < LARGHEZZA; i++)
    {
        printf("=");
    }
    printf("\n");
}

void stampa_testo_centrato(const char *testo)
{
    int lunghezza_testo = strlen(testo);
    int centro = (LARGHEZZA - lunghezza_testo) / 2;
    for(int i = 0; i < centro - 1; i++)
    printf(" ");
    {
        printf(" ");
    }
    printf("%s", testo);
    for(int i=0; i < (LARGHEZZA - (lunghezza_testo - 1) - centro); i++)
    {
        printf(" ");
    }
    printf("\n");
}



//scambio messaggi client-server
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n;
    
    memset(buffer, 0, buf_size); // Puliamo il buffer
    n = recv(client_fd, buffer, buf_size - 1, 0);
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
    if(send(client_fd, msg, strlen(msg) + 1, MSG_NOSIGNAL) == -1)
    {
        perror("Errore nell'invio della risposta al server\n");
        close(client_fd);
        exit(1);
    }
}