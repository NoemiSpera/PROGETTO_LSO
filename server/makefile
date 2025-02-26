#regola principale
server: server.o connessione.o impl_modelli.o 
	gcc -o server server.o connessione.o impl_modelli.o 

#regole per la compilazione di ogni singolo file sorgente
server.o:
	gcc -c server.c -o server.o

connessione.o:
	gcc -c source_file/connessione.c -o connessione.o

impl_modelli.o:
	gcc -c source_file/impl_modelli.c -o impl_modelli.o

#rimozione dei file compilati
clean:
	rm -f server.o connessione.o impl_modelli.o server

run_server: server
	./server

#per compilare usare il comando make run_server
#per pulire i file generati usare il comando make clean