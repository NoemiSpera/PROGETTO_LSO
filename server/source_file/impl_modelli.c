#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

#define LARGHEZZA 60
#define MAX 1024

//variabili globali
pthread_mutex_t lock;


//inizializzazioni
void inizializza_griglia(Partita *partita)
{
    for(int i = 0; i < N; i++)
    {
        for(int j = 0; j < N; j++)
        {
            partita->griglia[i][j]='-';
        }
    }
}



void inizializza_giocatore(Giocatore *giocatore, int socket, int id_partita, const char* nome)
{

    giocatore->socket=socket;
    strncpy(giocatore->nome,nome,MAX_NOME -1);
    giocatore->nome[MAX_NOME - 1]= '\0';
    giocatore->id_partita= id_partita;
    giocatore->stato=0;
    giocatore->simbolo=-1;

}



void inizializza_logica_partita(Logica_partita *logica, Partita *partita)
{
    logica->partita=partita;
    logica->stato=0;
    logica->turno=0;
    logica->risultato=-1;
    pthread_mutex_init(&logica->lock,NULL);   // inizializza il mutex a logica->lock
}



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



//gestione client
void *gestisci_client(void *arg)
{
    
    int client_socket = *(int*)arg;
    free(arg);

    int rec = 0;
    int indice_gioco=-1;
    char nome[MAX_NOME];
    char buffer[MAX];
    memset(buffer, 0, sizeof(buffer));
    
    rec = ricevi_messaggi(client_socket, buffer, sizeof(buffer));
    printf("%s ha fatto accesso al server\n", buffer);
    strcpy(nome,buffer);

    char benvenuto[256];
    snprintf(benvenuto, sizeof(benvenuto), "Ciao %.200s, benvenuto nel server!", buffer);
    invia_messaggi(client_socket, benvenuto);
    usleep(500000);
    
    char msg[] = "Vuoi creare una nuova partita(C) O unirti  a una esistente (U)?\n";
    invia_messaggi(client_socket, msg);
    
    memset(buffer, 0, sizeof(buffer));
    if(ricevi_messaggi(client_socket, buffer, sizeof(buffer)) > 0)
    {
        pthread_mutex_lock(&lock);
        gestisci_scelta(client_socket, buffer[0], nome);
        pthread_mutex_unlock(&lock);
    }

    close(client_socket);    
}

void gestisci_scelta(int client_fd, char scelta, char *nome_client)
{
    switch (scelta)
    {
        case 'C':
        case 'c':
            printf("%s ha scelto di creare una nuova partita\n", nome_client);
            //creazione_partita();
            break;
        
        case 'U':
        case 'u':
            printf("%s ha scelto di unirsi ad una partita esistente\n", nome_client);
            //unisciti_ad_una_partita();
            break;

        default:
            invia_messaggi(client_fd, "Scleta non valida");
            break;
    }
}



//scambio messaggi
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
    if(send(client_fd, msg, strlen(msg) + 1, 0) == -1)
    {
        perror("Errore nell'invio della risposta al server\n");
        close(client_fd);
        exit(1);
    }
}