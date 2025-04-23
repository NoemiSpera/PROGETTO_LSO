#LA PARTITA DI TRIS

Il progetto _"La partita di Tris"_ è stato ideato per mettere in pratica concetti quali gestione della sincronizzazione, della concorrenza e delle risorse. 
Prevede la simulazione di partite di tris tra due giocatori. Ad ogni partita verrà attribuito il risultato finale (vittoria-sconfitta-pareggio). 
Il tutto è gestito facendo uso di thread, variabili di condizione, mutex e liste per la gestione delle partite.

#STRUTTURA DEL PROGETTOIl progetto è strutturato nella seguente struttura composta da directory e file.
├── README.md
├── docker-compose.yml
├── Avvio.sh
├── client
│   ├── client.c
│   ├── header_file
│   │   ├── colori.h
│   │   └── modelli_client.h
│   ├── source_file
│   │   ├── connessione.c
│   │   ├── impl_client.c
│   │   └── gestione_gioco.c
│   ├── dockerfile
│   └── makefile
├── documentazione
│   ├── DOCUMENTAZIONE.pdf
└── server
    ├── dockerfile
    ├── makefile
    ├── server.c
    ├── header_file
    │   ├── colori.h
    │   └── modelli_server.h
    └── source_file
        ├── connessione.c
        ├── impl_server.c
        ├── gestisci_giocatore.c
        └── gestisci_partite.c

#_ISTRUZIONI PER L'ESECUZIONE_
Per avviare il docker-compose in maniera interattiva cosi da vedere la comunicazione client server, bisogna eseguire lo script di avvio, il quale:
avvierà il server
chiedera da linea di comando quanti client si vuole aprire
e tutto è avviato
chmod +x Avvio.sh
./Avvio.sh
