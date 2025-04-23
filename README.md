# LA PARTITA DI TRIS

Il progetto **"La partita di Tris"** è stato ideato per mettere in pratica concetti quali **gestione della sincronizzazione**, **della concorrenza** e delle **risorse**.  
Prevede la simulazione di partite di tris tra due giocatori. Ad ogni partita verrà attribuito il risultato finale (vittoria-sconfitta-pareggio).  
Il tutto è gestito facendo uso di **thread**, **variabili di condizione**, **mutex** e **liste** per la gestione delle partite.

---

# STRUTTURA DEL PROGETTO

Il progetto è strutturato nella seguente struttura composta da directory e file:

```plaintext
├── README.md
├── docker-compose.yml
├── Avvio.sh
├── client
│   ├── dockerfile
│   └── makefile
│   ├── client.c
│   ├── header_file
│   │   ├── colori.h
│   │   └── modelli_client.h
│   ├── source_file
│   │   ├── connessione.c
│   │   ├── impl_client.c
│   │   └── gestione_gioco.c
├── documentazione
│   ├── DOCUMENTAZIONE.pdf
└── server
    ├── dockerfile
    ├── makefile
    ├── server.c
    ├── header_file
    │   ├── colori.h
    │   └── modelli_server.h
    └── source_file
        ├── connessione.c
        ├── impl_server.c
        ├── gestisci_giocatore.c
        └── gestisci_partite.c
```

# ISTRUZIONI PER L'ESECUZIONE

Per avviare il **docker-compose** in maniera interattiva così da vedere la comunicazione client-server, bisogna eseguire lo script di avvio, il quale si occuperà in modo automatico di creare le immagini e i container in docker. In particolare:
1. Costruirà le immagini docker usando il comando ```docker-compose build```
2. Creerà la rete su cui client e server comunicheranno tramite il comando ```docker network create tris-network```
3. Avvierà il server utilizzando il comando ```docker compose up server```
4. Viene chiesto di inserire da linea di comando quanti client si vogliono avviare
5. Ogni client verrà avviato su una shell separata tramite il comando ```docker compose run -it --rm client ./client server 10000``` che lo renderà interattivo

Per avviare lo script che si occupa dell'esecuzione digitare i seguenti comandi:
```bash
chmod +x Avvio.sh
./Avvio.sh
