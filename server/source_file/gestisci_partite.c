#include "../header_file/modelli_server.h"
#include "../header_file/colori.h"



//inizializzazioni
void inizializza_griglia(char griglia[N])
{
    for (int i = 0; i < 9; i++) 
        griglia[i] = '1' + i; 
}



Giocatori *inizializza_giocatore(int socket, int id_partita, char* nome, char *simbolo)
{
    Giocatori *giocatore = (Giocatori*)malloc(sizeof(Giocatori));
    if(!giocatore) return NULL;

    giocatore->socket = socket;
    strncpy(giocatore->nome, nome, MAX_NOME);
    giocatore->id_partita = id_partita;
    giocatore->stato = 0;
    giocatore->in_partita=-1;
    strcpy(giocatore->simbolo, simbolo);

    return giocatore;
}



Partita* inizializza_partita(int id_partita, Giocatori *giocatore)
{

    Partita *nuova_partita = (Partita*)malloc(sizeof(Partita));
    if(!nuova_partita)
    {
        perror(YELLOW"Errore nell'allocazione della memoria\n"RESET);
        free(nuova_partita);
        return NULL;
    }

    nuova_partita->id = id_partita;

    giocatore->id_partita = id_partita;
    strcpy(giocatore->simbolo, "X");
    giocatore->in_partita = 1;

    nuova_partita->giocatore[0] = giocatore;
    nuova_partita->giocatore[1] = NULL;

    inizializza_griglia(nuova_partita->griglia);

    nuova_partita->stato = NUOVA;                 
    nuova_partita->turno = 0;                  // 0 = turno primo giocatore | 1 = turno secondo giocatore
    nuova_partita->risultato = NESSUNO;            

    pthread_mutex_init(&nuova_partita->mutex, NULL);
    pthread_cond_init(&nuova_partita->cond, NULL);

    return nuova_partita;
}



void avvia_thread_partita(Partita *partita)
{
    pthread_t thread_partita;

    pthread_mutex_lock(&partita->mutex);

    if (partita->giocatore[0] != NULL && partita->giocatore[1] != NULL)
    {
        pthread_create(&thread_partita, NULL, gestisci_gioco, (void *)partita);
        pthread_detach(thread_partita);  
    }

    partita->stato=IN_CORSO;
    pthread_mutex_unlock(&partita->mutex);
}



void *gestisci_gioco(void *arg)
{
    Partita *p = (Partita *) arg;
    char messaggio[MAX];

    printf("La partita con id %d è iniziata tra %s e %s\n", p->id, p->giocatore[0]->nome,p->giocatore[1]->nome);
    pthread_mutex_lock(&p->mutex);

    while(p->stato == IN_CORSO)
    {
        int turno = p->turno;
        int avversario = (turno +1) % 2;
        
        //invio del messaggio del turno e della griglia
        invia_messaggi(p->giocatore[turno]->socket, "TUO_TURNO\n");
        usleep(1);
        invia_messaggi(p->giocatore[avversario]->socket, "ATTENDI\n"); 
        usleep(1);
        invia_messaggi(p->giocatore[turno]->socket, p->griglia);
        usleep(1);
        invia_messaggi(p->giocatore[avversario]->socket, p->griglia);
        
        //ricezione della mossa effettuata dal giocatore
        ricevi_messaggi(p->giocatore[turno]->socket, messaggio, sizeof(messaggio));
        int mossa=atoi(messaggio);

       
        if(mossa_valida(p,mossa,p->giocatore[turno],p->giocatore[turno]->simbolo))
        {
        
            if (controlla_vittoria(p->griglia, p->giocatore[turno]->simbolo) == 1)
            {
                
                p->stato = TERMINATA;
                
                pthread_cond_broadcast(&p->cond); 

                invia_messaggi(p->giocatore[turno]->socket, "PARTITA_VINTA\n");
                usleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, "PARTITA_PERSA\n");
                usleep(1);
                invia_messaggi(p->giocatore[turno]->socket, p->griglia);
                usleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, p->griglia);
                printf("La partita con id %d tra %s e %s è terminata e ha vinto %s\n", p->id, p->giocatore[turno]->nome, p->giocatore[avversario]->nome, p->giocatore[turno]->nome);

                p->giocatore[0]->in_partita = -1;
                p->giocatore[0]->id_partita = -1;
                p->giocatore[1]->in_partita = -1;
                p->giocatore[1]->id_partita = -1;

                continue;
               
            }
            
            if(controlla_pareggio(p->griglia))
            {
                
                p->stato = TERMINATA;
               
                pthread_cond_broadcast(&p->cond); 

                invia_messaggi(p->giocatore[turno]->socket, "PAREGGIO\n");
                usleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, "PAREGGIO\n");
                usleep(1);
                invia_messaggi(p->giocatore[turno]->socket, p->griglia);
                usleep(1);
                invia_messaggi(p->giocatore[avversario]->socket, p->griglia);
                printf("La partita con id %d tra %s e %s è terminata. È un pareggio\n", p->id, p->giocatore[turno]->nome, p->giocatore[avversario]->nome);


                p->giocatore[0]->in_partita = -1;
                p->giocatore[0]->id_partita = -1;
                p->giocatore[1]->in_partita = -1;
                p->giocatore[1]->id_partita = -1;

                continue; 
                
            }

            p->turno = avversario;
           
        }else
        {       
            invia_messaggi(p->giocatore[turno]->socket, "MOSSA_NON_VALIDA\n");
            
        } 
               
        
    }
    pthread_mutex_unlock(&p->mutex);
    return NULL;
}



int mossa_valida(Partita *partita, int mossa, Giocatori *giocatore, char *simbolo) 
{
    int indice = mossa -1;
    
    if (partita->griglia[indice] == 'X' || partita->griglia[indice] == 'O') 
    {  
        return 0;
    }
    
    partita->griglia[indice] = *simbolo;  
    return 1;
}



int controlla_vittoria(char g[N], char *simbolo) 
{
    // Controlla righe
    for (int i = 0; i < 9; i += 3) {
        if (g[i] == *simbolo && g[i + 1] == *simbolo && g[i + 2] == *simbolo)
            return 1;
    }

    // Controlla colonne
    for (int i = 0; i < 3; i++) {
        if (g[i] == *simbolo && g[i + 3] == *simbolo && g[i + 6] == *simbolo)
            return 1;
    }

    // Controlla diagonali
    if ((g[0] == *simbolo && g[4] == *simbolo && g[8] == *simbolo) ||
        (g[2] == *simbolo && g[4] == *simbolo && g[6] == *simbolo)) {
        return 1;
    }

    return 0; 
}



int controlla_pareggio(char g[N]) 
{
    for (int i = 0; i < N; i++) {
        if (g[i] != 'X' && g[i] != 'O') {
            return 0; 
        }
    }
    return 1;
}