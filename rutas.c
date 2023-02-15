#include "rutas.h"
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
#include "maneja_error.h"
#define MAXLINE 4096

typedef void *(*funcion_de_ruta)(request_t *request, response_t *response, void *aux);

typedef struct estructura_ruta
{
        funcion_de_ruta funcion;
        void *aux;
} estructura_ruta_t;

void *send_response(response_t *response)
{
        uint8_t response_data[MAXLINE + 1];

        if (response->status == 200)    //mejorar esta parte de los response
        {
                snprintf((char *)response_data, sizeof(response_data), "HTTP/1.0 200 OK \r\n\r\n");
        }

        strcat((char *)response_data, response->data);

        if(maneja_error(write(response->client_socket, (char *)response_data, strlen((char *)response_data)))) {

        }
        close(response->client_socket);
        memset(response_data, 0, MAXLINE);
        free(response);
        return NULL;
}

void *not_found(response_t *response)
{
        uint8_t response_data[MAXLINE + 1];

        snprintf((char *)response_data, sizeof(response_data), "HTTP/1.0 404 NOT FOUND \r\n\r\nNOT FOUND PAGE");

        if(maneja_error(write(response->client_socket, (char *)response_data, strlen((char *)response_data)))){

        }
        close(response->client_socket);
        memset(response_data, 0, MAXLINE);
        free(response);
        return NULL;
}

char *parse_request(char **claves[100], char **valores[100])
{
        
}

void *handle_connection(void *client_pointer, void *rutas)
{
        int client_socket = *((int *)client_pointer);
        free(client_pointer);
        //ssize_t message_size = 0;
        ssize_t bytes_read;

        uint8_t request_data[MAXLINE + 1];

        memset(request_data, 0, MAXLINE);

        /*while ((bytes_read = read(client_socket, request_data + message_size, (sizeof(request_data) - (size_t)message_size))) > 0)
        {
                printf("%s", request_data);
                message_size += bytes_read;

                if (message_size > MAXLINE - 1 || request_data[message_size - 1] == '\n')
                        break;

                //if (bytes_read == 0)
                //        break;
        }*/

        bytes_read = read(client_socket, request_data, MAXLINE);
        //printf("%s", request_data);

        


        if (bytes_read < 0)
                return NULL; 

        response_t *response = malloc(sizeof(response_t));      //verificar los malloc
        request_t *request = malloc(sizeof(request_t));
        response->client_socket = client_socket;


        /*char *http_header = strtok((char *)request_data, "\n"); //ESTA MANERA DE SACAR ES PROVISIONAL, NECESITO ALGO TIPO REGEX CUANDO TENGA QUE
        //SACAR COSAS DEL BODY O ALGO ASI
        char *method = "";
        char *route_url = "";
        char *header = strtok(http_header, " ");

        int header_parse_counter = 0;
        while (header != NULL)
        {
                switch (header_parse_counter)
                {
                case 0:
                        method = header;
                case 1:
                        route_url = header;
                }
                header = strtok(NULL, " ");
                header_parse_counter++;
        }*/



        char *token = strtok((char *)request_data, "\n");

        char http_header[101];
        memcpy(http_header, token, 100);
        char *claves[100];
        char *valores[100];
        int posicion_insertar_clave = 0;
        int posicion_insertar_valor = 0;
        int iteraciones = 0;
        bool va_en_valor = false;

        while(token != NULL) { 
                
                if (iteraciones > 1 && iteraciones % 2 == 0) {
                        
                        if (iteraciones % 2 == 0 && !va_en_valor) {
                                claves[posicion_insertar_clave] = token;
                                posicion_insertar_clave++;
                        }else if (iteraciones % 2 == 0 && va_en_valor) {
                                valores[posicion_insertar_valor] = token;
                                posicion_insertar_valor++;   
                        }
                        
                        va_en_valor = !va_en_valor;
                }

                token = strtok(NULL, "\"");
                iteraciones++;
        }
        
        char *header = strtok(http_header, " ");
        char *method = "";
        char *route_url = "";

        int header_parse_counter = 0;
        while (header != NULL)
        {
                switch (header_parse_counter)
                {
                case 0:
                        method = header;
                case 1:
                        route_url = header;
                }
                header = strtok(NULL, " ");
                header_parse_counter++;
        }

        


        char la_ruta[MAXLINE];
        memset(la_ruta, 0, MAXLINE);
        strcat(la_ruta, method);
        strcat(la_ruta, route_url);        

        estructura_ruta_t *estructura_ruta = hash_obtener(rutas, la_ruta);

        if (estructura_ruta == NULL)
                not_found(response);

        if (estructura_ruta != NULL)
                estructura_ruta->funcion(request, response, estructura_ruta->aux);

        free(request);

        return NULL;
}

void *crear_ruta(hash_t *hash, char *nombre_ruta, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_header tipo) //tipo, por si es get, post, put, etc
{
        estructura_ruta_t *estructura_ruta = malloc(sizeof(estructura_ruta_t)); // donde te libero? XD
        estructura_ruta->funcion = f;
        estructura_ruta->aux = aux;
        char nombre[MAXLINE];
        memset(nombre, 0, MAXLINE);
        if (tipo == GET) {
                strcat(nombre, "GET");
                strcat(nombre, nombre_ruta);
        } else if (tipo == POST) {
                strcat(nombre, "POST");
                strcat(nombre, nombre_ruta);
        } else if (tipo == PUT) {
                strcat(nombre, "PUT");
                strcat(nombre, nombre_ruta);
        } else if (tipo == DELETE) {
                strcat(nombre, "DELETE");
                strcat(nombre, nombre_ruta);
        }

        return hash_insertar(hash, nombre, estructura_ruta, NULL);
}

response_t *set_data(response_t *response, void *data)
{
        response->data = data;
        return response;
}

response_t *set_status(response_t *response, int status)
{
        response->status = status;
        return response;
}