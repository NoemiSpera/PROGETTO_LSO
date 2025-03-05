#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"
 

//scambio messaggi
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n;
    
    memset(buffer, 0, buf_size); // Puliamo il buffer
    n = recv(client_fd, buffer, buf_size - 1, 0);
    if (n == 0) 
    {
        // Connessione chiusa dal server
        printf("Connessione chiusa dal server.\n");
        return 0;  // Ritorna 0 per segnalare la chiusura
    }
    else if (n < 0) 
    {
        // Errore nella ricezione
        perror("Errore nella ricezione del messaggio");
        return -1;  // Ritorna -1 per segnalare l'errore
    } 
    else
    {
        buffer[n] = '\0';
        return n;
    }
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



//input da tastiera
char *inserisci_nome()
{
    char *nome=malloc(MAX_NOME * sizeof(char));
    printf("Inserisci il tuo nome: ");
    nascondi_input();
    scanf("%s",nome);
    ripristina_input();
    printf("\r\033[K");

    return nome;
}



void nascondi_input() {
    struct termios new_attr, old_attr;
    
    // Ottieni i settings attuali della terminale
    tcgetattr(STDIN_FILENO, &old_attr);
    
    // Imposta i nuovi settings per disabilitare l'eco
    new_attr = old_attr;
    new_attr.c_lflag &= ~ECHO;  // Disabilita l'eco

    // Applica i nuovi settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);
}



void ripristina_input() {
    struct termios old_attr;
    
    // Ripristina i settings originali della terminale
    tcgetattr(STDIN_FILENO, &old_attr);
    old_attr.c_lflag |= ECHO;  // Riabilita l'eco
    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
}