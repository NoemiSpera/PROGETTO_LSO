#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"
 

//scambio messaggi
ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
{
    ssize_t n;
    memset(buffer, 0, buf_size); // Puliamo il buffer
    n = recv(client_fd, buffer, buf_size - 1, 0);
    if (n == 0) {

        // Connessione chiusa dal server
        printf("Connessione chiusa dal server.\n");
        return 0;  // Ritorna 0 per segnalare la chiusura
    }
    else if (n < 0) {
        // Errore nella ricezione
        perror("Errore nella ricezione del messaggio\n");
        return -1;  // Ritorna -1 per segnalare l'errore
    } else {
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
    printf(LIGHT_GREEN "Inserisci il tuo nome: " RESET);
    nascondi_input();
    scanf("%s",nome);
    ripristina_input();
    printf("\r\033[K");

    return nome;
}



void nascondi_input() 
{
    struct termios new_attr, old_attr;
    
    // Ottieni i settings attuali della terminale
    tcgetattr(STDIN_FILENO, &old_attr);
    
    // Imposta i nuovi settings per disabilitare l'eco
    new_attr = old_attr;
    new_attr.c_lflag &= ~ECHO;  // Disabilita l'eco

    // Applica i nuovi settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);
}



void ripristina_input() 
{
    struct termios old_attr;
    
    // Ripristina i settings originali della terminale
    tcgetattr(STDIN_FILENO, &old_attr);
    old_attr.c_lflag |= ECHO;  // Riabilita l'eco
    tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
}




void stampa_griglia(char griglia[N]) 
{
    // Stampa la griglia in un formato simile a una tabella
    printf("%c | %c | %c\n", griglia[0], griglia[1], griglia[2]);
    printf("---------\n");
    printf("%c | %c | %c\n", griglia[3], griglia[4], griglia[5]);
    printf("---------\n");
    printf("%c | %c | %c\n", griglia[6], griglia[7], griglia[8]);
}



void gestisci_partita(int client_fd) 
{
    char buffer[MAX];   
    char buffer_griglia[N];    
    int mossa;

    // Giocatore due si è unito
    ricevi_messaggi(client_fd, buffer, sizeof(buffer));
    printf("%s\n", buffer);

    while (1)
    {   
        // turno
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));

        //griglia
        ricevi_messaggi(client_fd,buffer_griglia,sizeof(buffer_griglia));
        printf("=== Griglia di Gioco ===\n");
        stampa_griglia(buffer_griglia);
        

        if (strncmp(buffer, "TUO_TURNO", 9) == 0)
        {
            printf("È IL TUO TURNO!\n");

            do
            {
                printf("Scegli una mossa (1-9): ");
                scanf("%d", &mossa);
            } while (mossa < 1 || mossa > 9);

            char risposta[10];
            sprintf(risposta, "%d", mossa);
            invia_messaggi(client_fd, risposta);
        }
        else if (strncmp(buffer, "ATTENDI", 7) == 0)
        {
            printf("ATTENDI IL TUO TURNO...\n");
        }
        else if (strncmp(buffer, "PARTITA_VINTA", 12) == 0)
        {
            printf("Hai vinto!\n");
            break;
        }
        else if (strncmp(buffer, "PARTITA_PERSA", 13) == 0)
        {
            printf("Hai perso!\n");
            break;
        }
        else if (strncmp(buffer, "PAREGGIO", 8) == 0)
        {
            printf("La partita è finita in pareggio!\n");
            break;
        }
        else if (strncmp(buffer, "MOSSA_NON_VALIDA", 16) == 0)
        {
            printf("Mossa non valida, riprova.\n");
        }
        else
        {
            printf("Errore: messaggio sconosciuto ricevuto: %s\n", buffer);
            break;
        }
    }
}