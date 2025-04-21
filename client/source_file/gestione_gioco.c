#include "../header_file/modelli_client.h"
#include "../header_file/colori.h"


int richiesta_partecipazione(int client_fd)
{
    char buffer[MAX];
    char risposta = 'n';
    
    while(1)
    {
        //richiesta di partecipazione
        ricevi_messaggi(client_fd, buffer, sizeof(buffer));  
        printf(BLUE"%s\n"RESET, buffer);

    
        printf("Inserisci la tua scelta: ");
        scanf(" %c", &risposta);  // Inserisci 's' o 'n'

        //inserisce si o no
        char input[2] = {risposta, '\0'};
        invia_messaggi(client_fd, input);  // Invia al server

        //risposta alla richiesta
        ricevi_messaggi(client_fd,buffer,sizeof(buffer));
        printf("%s",buffer);

        if(risposta == 'S' || risposta == 's')
        {
            return 1;
        }
        
    }
}



int gioca_con_amico(int client_fd)
{
    char buffer[MAX];
    char id[10];
    
    //lista partite
    ricevi_messaggi(client_fd,buffer,sizeof(buffer));
    printf("%s",buffer);
    printf(GREEN "Inserisci l'id della partita a cui vuoi partecipare: "RESET);
    scanf(" %s",id);
    invia_messaggi(client_fd,id);

    //accettazione rifiuto o errore
    ricevi_messaggi(client_fd,buffer,sizeof(buffer));

    if (strstr(buffer, "accettata!") != NULL)
    {
        printf("%s",buffer);
        return 1;

    }else if( strstr(buffer,"rifiutata") != NULL){

        printf(RED "%s" RESET, buffer);
        return 0;
    }
    else if (strstr(buffer, "non esiste") != NULL) 
    {
        printf(RED "%s" RESET, buffer);
        return 0; 
    }
}


int partita_casuale(int client_fd)
{
    char buffer[MAX];
    ricevi_messaggi(client_fd,buffer,sizeof(buffer));
    printf("%s\n",buffer);

    ricevi_messaggi(client_fd,buffer,sizeof(buffer));
    printf("%s",buffer);
    if (strstr(buffer, "accettata!") != NULL)    {
                
        return 1;
    }else if( strstr(buffer,"rifiutata") != NULL){

        printf(RED "%s" RESET, buffer);
        return 0;
    }

}



int gestisci_partita(int client_fd) 
{
    char buffer[MAX];   
    char buffer_griglia[N];    
    char mossa[10];
    int partita_in_corso = 1;
    
    while (partita_in_corso)
    {   

        ricevi_messaggi(client_fd, buffer, sizeof(buffer));

        ricevi_messaggi(client_fd, buffer_griglia, sizeof(buffer_griglia));
        


        printf("=== Griglia di Gioco ===\n");
        stampa_griglia(buffer_griglia);
        
        if (strncmp(buffer, "TUO_TURNO", 9) == 0)
        {
            printf("È IL TUO TURNO!\n");
            printf("Scegli una mossa (1-9): ");
            scanf(" %s", mossa);

            invia_messaggi(client_fd, mossa);
        }
        else if (strncmp(buffer, "ATTENDI", 7) == 0)
        {
            printf("ATTENDI IL TUO TURNO...\n");
            
        }
        else if (strncmp(buffer, "PARTITA_VINTA", 13) == 0)
        {
            printf("Hai vinto!\n");
            partita_in_corso = 0;
            return 0;
        
        }
        else if (strncmp(buffer, "PARTITA_PERSA", 13) == 0)
        {
            printf("Hai perso!\n");
            partita_in_corso = 0;
            return 0;
        }
        else if (strncmp(buffer, "PAREGGIO", 8) == 0)
        {
            printf("La partita è finita in pareggio!\n");
            partita_in_corso = 0;
            return 0;
        }
        else if (strncmp(buffer, "MOSSA_NON_VALIDA", 16) == 0)
        {
            printf("Mossa non valida, riprova.\n");
            printf("Scegli una mossa (1-9): ");
            scanf(" %s", mossa);

            invia_messaggi(client_fd, mossa);
        }
        else
        {
            printf("Errore: messaggio sconosciuto ricevuto: %s\n", buffer);
            break;
        }
    }
}


void stampa_griglia(char griglia[N]) 
{
    for (int i = 0; i < 9; i++) {
        char simbolo = griglia[i];
        if (simbolo == 'X') {
            printf(CYAN"%c"RESET, simbolo); 
        } else if (simbolo == 'O') {
            printf(YELLOW"%c"RESET, simbolo); 
        } else {
            printf("%c", simbolo);
        }

        if (i % 3 != 2) {
            printf(" | ");
        } else if (i != 8) {
            printf("\n---------\n");
        } else {
            printf("\n");
        }
    }
}



void stampa_partite(char *buffer) {
    
    char *riga;
    char temp[1024];

    printf("Partite disponibili:\n");
    strcpy(temp, buffer);

    //separa le righe tramite \n
    riga = strtok(temp, "\n");

    while (riga != NULL) {
    
        printf("%s\n", riga);
        riga = strtok(NULL, "\n");
    }
}