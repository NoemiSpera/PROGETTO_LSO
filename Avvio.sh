#!/bin/bash


echo "Costruisco le immagini Docker..."
docker-compose build

docker network create tris-network 2>/dev/null || true

echo "Avvio del server..."
gnome-terminal -- bash -c "docker compose up server; exec bash"



sleep 2


read -p "Quanti client vuoi avviare? " NUM_CLIENTS

# Cicla per ogni client e apri un terminale nuovo
for ((i=1; i<=NUM_CLIENTS; i++)); do
    echo "Avvio client $i..."
    gnome-terminal -- bash -c "docker compose run -it --rm client ./client server 10000; exec bash"


done

