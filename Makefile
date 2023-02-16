VALGRIND_FLAGS=--leak-check=full --track-origins=yes --show-reachable=yes --error-exitcode=2 --show-leak-kinds=all --trace-children=yes
CFLAGS =-g -std=c99 -Wall -Wconversion -Wtype-limits -pedantic -Werror -O2
CC = gcc

all: clean valgrind


valgrind-compilar: compilar
	valgrind $(VALGRIND_FLAGS) ./server 2>&1

compilar: cola.c hash.c rutas.c server.c maneja_error.c ejemplo_programa.c
	$(CC) $(CFLAGS) cola.c hash.c rutas.c maneja_error.c server.c ejemplo_programa.c -o server 2>&1
