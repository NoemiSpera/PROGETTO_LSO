    #include "../header_file/modelli_client.h"
    #include "../header_file/colori.h"
    


    //scambio messaggi
    ssize_t ricevi_messaggi(int client_fd, char *buffer, size_t buf_size)
    {
        ssize_t n = 0;
        memset(buffer, 0, buf_size); // Puliamo il buffer
        n = recv(client_fd, buffer, buf_size, 0);
        if (n == 0) {

            // Connessione chiusa dal server
            printf("Connessione chiusa dal server.\n");
            return 0;  // Ritorna 0 per segnalare la chiusura
        }
        else if (n < 0) {
            // Errore nella ricezione
            perror("Errore nella ricezione del messaggio\n");
            return -1;  // Ritorna -1 per segnalare l'errore
        } /*else {
            buffer[n] = '\0';
            return n;
        }*/
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
        // Variabili per la gestione della stringa
        char *riga;
        char temp[1024];

        printf("Partite disponibili:\n");

        // Copia la stringa nel buffer temporaneo per elaborarla
        strcpy(temp, buffer);

        // Usa strtok per separare le righe (partite) tramite il carattere di nuova linea
        riga = strtok(temp, "\n");

        while (riga != NULL) {
            // Stampa ogni riga che rappresenta una partita
            printf("%s\n", riga);
            // Vai alla prossima riga
            riga = strtok(NULL, "\n");
        }
    }



int gioca_con_amico(int client_fd,char *nome)
    {
        char partite[MAX];
        char buffer[MAX];
        int id;
        char risposta;
    
        //riceve lista partite
        ricevi_messaggi(client_fd, partite, sizeof(partite));
        stampa_partite(partite);

        printf("Inserisci l'id della partita a cui vuoi partecipare: ");
        char c;
        while ((c = getchar()) != '\n' && c != EOF);
        scanf(" %d",&id);

        sprintf(buffer, "%d\n", id);
        invia_messaggi(client_fd, buffer);

        ricevi_messaggi(client_fd, buffer, sizeof(buffer));
        if (strncmp(buffer, "PARTITA_RIFIUTATA", 17) == 0) 
        {
            printf(RED"La partita è stata rifiutata. Torna al menu principale\n"RESET);
            return 0;  // Fallimento
        }else{
            printf("%s",buffer);
            return 1;
        }
    }



    int gestisci_richiesta_partecipazione(int client_fd)
    {
        char buffer[MAX];
        char risposta = 'n';
        //riceve vuoi giocare con...?
        while(1)
        {
            ricevi_messaggi(client_fd, buffer, sizeof(buffer));  
            printf(BLUE"%s\n"RESET, buffer);

        
            printf("Inserisci la tua scelta: ");
            scanf(" %c", &risposta);  // Inserisci 's' o 'n'

            //inserisce si o no
            char input[2] = {risposta, '\0'};
            invia_messaggi(client_fd, input);  // Invia al server


            ricevi_messaggi(client_fd,buffer,sizeof(buffer));
            printf("%s",buffer);

            if(risposta == 'S' || risposta == 's')
            {
                return 1;
            }
            
        }
    }

    int gestisci_partita(int client_fd) 
    {
        char buffer[MAX];   
        char buffer_griglia[N];    
        char mossa[10];
        int partita_in_corso = 1;
        
        //ricevi_messaggi(client_fd,buffer,sizeof(buffer));
        //printf("qua:%s",buffer);

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
    


