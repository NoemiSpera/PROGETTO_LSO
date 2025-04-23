client: client.o connessione.o impl_client.o gestione_gioco.o
	gcc -o client client.o connessione.o  impl_client.o gestione_gioco.o

client.o:
	gcc -c client.c -o client.o

connessione.o:
	gcc -c source_file/connessione.c -o connessione.o

impl_client.o:
	gcc -c source_file/impl_client.c -o impl_client.o

gestione_gioco.o:
	gcc -c source_file/gestione_gioco.c -o gestione_gioco.o

clean:
	rm -f client.o connessione.o impl_client.o gestione_gioco.o client

run_client: client
	./client 127.0.0.1