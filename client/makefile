#regola principale
client: client.o connessione.o impl_client.o gestione_gioco.o
	gcc -o client client.o connessione.o  impl_client.o gestione_gioco.o

#regole per la compilazione di ogni singolo file sorgente
client.o:
	gcc -c client.c -o client.o

connessione.o:
	gcc -c source_file/connessione.c -o connessione.o

impl_client.o:
	gcc -c source_file/impl_client.c -o impl_client.o

gestione_gioco.o:
	gcc -c source_file/gestione_gioco.c -o gestione_gioco.o
#rimozione dei file compilati
clean:
	rm -f client.o connessione.o impl_client.o gestione_gioco.o client

run_client: client
	./client

#per compilare usare il comando make run_client
#per pulire i file generati usare il comando make clean