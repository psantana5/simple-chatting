#!/bin/bash

echo "Installing dependencies..."

while true; do
    sudo apt update
    sudo apt install build-essential
    sudo apt install libsqlite3-dev libssl-dev libwebsockets-dev
done
