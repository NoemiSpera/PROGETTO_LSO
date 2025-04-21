#regola principale
server: server.o connessione.o impl_server.o gestisci_giocatore.o gestisci_partite.o
	gcc -o server server.o connessione.o impl_server.o  gestisci_giocatore.o gestisci_partite.o

#regole per la compilazione di ogni singolo file sorgente
server.o:
	gcc -c server.c -o server.o

connessione.o:
	gcc -c source_file/connessione.c -o connessione.o

impl_server.o:
	gcc -c source_file/impl_server.c -o impl_server.o

gestisci_giocatore.o:
	gcc -c source_file/gestisci_giocatore.c -o gestisci_giocatore.o

gestisci_partite.o:
	gcc -c source_file/gestisci_partite.c -o gestisci_partite.o

#rimozione dei file compilati
clean:
	rm -f server.o connessione.o impl_server.o  gestisci_giocatore.o gestisci_partite.o server

run_server: server
	./server

#per compilare usare il comando make run_server
#per pulire i file generati usare il comando make clean