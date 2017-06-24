#!/bin/bash

rm server client

gcc -c client.c -o client.o -Wall
gcc -c server.c -o server.o -Wall
gcc -c library/library.c -o library.o

gcc client.o library.o -o client -Wall
gcc server.o library.o -o server -Wall

rm server.o client.o library.o
