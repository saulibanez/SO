#!/bin/sh
gcc -c -Wall -Wshadow -g tok.c
gcc -o tok tok.c

gcc -c -Wall -Wshadow -g bigrams.c
gcc -o bigrams bigrams.o -lpthread
