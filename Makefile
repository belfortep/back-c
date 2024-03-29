VALGRIND_FLAGS=--leak-check=full --track-origins=yes --show-reachable=yes --error-exitcode=2 --show-leak-kinds=all --trace-children=yes
CFLAGS =-g -std=c99 -Wall -Wconversion -Wtype-limits -pedantic -Werror -O2
CC = gcc

all: clean valgrind


valgrind-compilar: compilar
	valgrind $(VALGRIND_FLAGS) ./server 2>&1

compilar: my_backend_c/data_structures/*.c my_backend_c/router/*.c my_backend_c/server/*.c my_backend_c/error_management/*.c my_backend_c/data_parsers/*.c my_backend_c/data_load/*.c ejemplo_programa.c
	$(CC) $(CFLAGS) my_backend_c/data_structures/*.c my_backend_c/router/*.c my_backend_c/server/*.c my_backend_c/data_parsers/*.c my_backend_c/error_management/*.c my_backend_c/data_load/*.c my_backend_c/database/*.c ejemplo_programa.c -ljansson `mysql_config --cflags --libs` -o server 2>&1
