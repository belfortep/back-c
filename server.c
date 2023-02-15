#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include "cola.h"
#include "hash.h"
#include "rutas.h"

#define SA struct sockaddr

#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 10
#define TAMANIO_HASH 10


pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;



void *thread_function(void *rutas)
{
        while (1)
        {
                int *client_pointer;
                pthread_mutex_lock(&mutex);

                if ((client_pointer = dequeue()) == NULL)
                {
                        pthread_cond_wait(&condition_var, &mutex);
                        client_pointer = dequeue();
                }
                pthread_mutex_unlock(&mutex);

                if (client_pointer != NULL)
                {
                        handle_connection(client_pointer, rutas);
                }
        }
}

int iniciar_server(uint16_t port, hash_t *rutas, void (*f)())
{
        int server_socket, client_socket;
        struct sockaddr_in server_address;
        
        if (f != NULL) 
                f();    //no se porque no se ejecuta ._.XD, tipo se ejecuta solo cuando no abre el servidor? re sus
        

        for (int i = 0; i < THREAD_POOL_SIZE; i++)
                pthread_create(&thread_pool[i], NULL, thread_function, rutas);

        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                return 0;

        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(port);

        if ((bind(server_socket, (SA *)&server_address, sizeof(server_address))) < 0)
                return 0;

        if ((listen(server_socket, SERVER_BACKLOG)) < 0)
                return 0;

        while (1 == 1)
        {
                // struct sockaddr_in addr;
                // socklen_t addr_len;

                client_socket = accept(server_socket, NULL, NULL);

                int *client_pointer = malloc(sizeof(int));
                *client_pointer = client_socket;

                pthread_mutex_lock(&mutex);
                enqueue(client_pointer);

                pthread_cond_signal(&condition_var);
                pthread_mutex_unlock(&mutex);
                
        }
}




