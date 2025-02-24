#include "modelli_server.h"


//inizializzazione della griglia vuota
void inizializza_(Partita *partita)
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