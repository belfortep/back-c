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
                snprintf((char *)response_data, sizeof(response_data), "HTTP/1.0 200 OK \r\n\r\n ");
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

void parse_request(char *request_data, char *claves[100], char *valores[100], char *route_url, char *method, int *cantidad_pares)
{
        int posicion_insertar_clave = 0;
        int posicion_insertar_valor = 0;
        int iteraciones = 0;
        bool va_en_valor = false;
        char *token = strtok((char *)request_data, "\n");
        char http_header[101];

        memcpy(http_header, token, 100);

        while(token != NULL) { 
                
                if (iteraciones > 1 && iteraciones % 2 == 0) {
                        
                        if (iteraciones % 2 == 0 && !va_en_valor) {
                                claves[posicion_insertar_clave] = token;
                                //printf("EN CLAVE GUARDE %s \n", claves[posicion_insertar_clave]);
                                posicion_insertar_clave++;
                        }else if (iteraciones % 2 == 0 && va_en_valor) {
                                valores[posicion_insertar_valor] = token;
                                //printf("EN VALOR GUARDE %s \n", valores[posicion_insertar_valor]);
                                posicion_insertar_valor++;   
                                (*cantidad_pares)++;
                        }
                        
                        va_en_valor = !va_en_valor;
                }

                token = strtok(NULL, "\"");
                iteraciones++;
        }
        
        char *header = strtok(http_header, " ");

        int header_parse_counter = 0;
        while (header != NULL)
        {
                switch (header_parse_counter)
                {
                case 0:
                        strcpy(method, header);
                case 1:
                        strcpy(route_url, header);
                }
                header = strtok(NULL, " ");
                header_parse_counter++;
        }
}




response_t *create_response(int client_socket)
{
        response_t *response = malloc(sizeof(response_t));

        if (!response)
                return NULL;

        response->data = "";
        response->status = 404;
        response->client_socket = client_socket;

        return response;
}


request_t *create_request()
{
        request_t *request = malloc(sizeof(request_t));

        if (!request)
                return NULL;

        request->body = hash_crear(10);
        request->cookies = hash_crear(10);
        request->params = hash_crear(10);
        request->query = hash_crear(10);

        if (request->body == NULL || request->cookies == NULL || request->params == NULL || request->query == NULL)
                return NULL;

        return request;
}

void free_request(request_t *request)
{
        hash_destruir(request->body);
        hash_destruir(request->cookies);
        hash_destruir(request->params);
        hash_destruir(request->query);
        free(request);
}

void *handle_connection(void *client_pointer, void *rutas)
{
        int client_socket = *((int *)client_pointer);
        free(client_pointer);
        ssize_t bytes_read;
        uint8_t request_data[MAXLINE + 1];
        memset(request_data, 0, MAXLINE);


        bytes_read = read(client_socket, request_data, MAXLINE);
        if (bytes_read < 0)
                return NULL; 

        response_t *response = create_response(client_socket);
        request_t *request = create_request();

        if (!response || !request)
                return NULL;
        
        char *claves[100];
        char *valores[100];
        char method[100];
        char route_url[100];
        int cantidad_pares = 0;
        
        parse_request((char *)request_data, claves, valores, route_url, method, &cantidad_pares);

        for (int i = 0; i < cantidad_pares; i++)
                hash_insertar(request->body, claves[i], valores[i], NULL);
        
        
        char la_ruta[MAXLINE];
        memset(la_ruta, 0, MAXLINE);
        strcat(la_ruta, method);
        strcat(la_ruta, route_url); 

        estructura_ruta_t *estructura_ruta = hash_obtener(rutas, la_ruta);

        if (estructura_ruta == NULL)
                not_found(response);

        if (estructura_ruta != NULL)
                estructura_ruta->funcion(request, response, estructura_ruta->aux);

        free_request(request);

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

response_t *set_data(response_t *response, char *data)
{
        response->data = data;
        return response;
}

response_t *set_status(response_t *response, int status)
{
        response->status = status;
        return response;
}

response_t *set_data_json(response_t *response, char *clave, char *valor)
{
        response->claves[response->cantidad_pares] = clave;
        response->valores[response->cantidad_pares] = valor;
        response->cantidad_pares++;
        return response;
}