#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"


//scambio messaggi
void *ascolta_notifiche(void *arg) 
{
    int client_fd = *((int *)arg);
    char buffer[MAX];
    
    while (1) 
    {
        int n = recv(client_fd, buffer, sizeof(buffer) - 1, MSG_PEEK);
        if (n > 0) 
        {
            buffer[n] = '\0';

            if (strncmp(buffer, "[NOTIFICA]", 10) == 0) {
                n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                buffer[n] = '\0';
                printf(MAGENTA "\n%s\n" RESET, buffer);  
            } else {
               
                usleep(100 * 1000); 
            }
        }
    }
    return NULL;
}




ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n = 0;
    memset(buffer, 0, buf_size);
    n = recv(client_fd, buffer, buf_size, 0);
    if (n == 0) {

        printf("Connessione chiusa dal server.\n");
        return 0;  
    }
    else if (n < 0) {
       
        perror("Errore nella ricezione del messaggio\n");
        return -1;  
    }else {
        buffer[n] = '\0';
        return n;
    }
}



void invia_messaggi(int client_fd, char *msg)
{
    if(send(client_fd, msg, strlen(msg), 0) == -1)
    {
        perror("Errore nell'invio della risposta al server\n");
        close(client_fd);
        exit(1);
    }
    
}



//input da tastiera
char *inserisci_nome()
{
    char *nome=malloc(MAX_NOME * sizeof(char));
    printf(LIGHT_GREEN "Inserisci il tuo nome: " RESET);
    scanf("%s",nome);
    return nome;
}