#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"

#define LARGHEZZA 60

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