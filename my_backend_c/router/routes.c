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
#include <jansson.h>
#include "../error_management/error_management.h"
#include "routes.h"
#define MAXLINE 4096
#define HEADER_OK "HTTP/1.1 200 OK\n"
#define HEADER_CREATED "HTTP/1.1 201 CREATED\n"
#define HEADER_NO_CONTENT "HTTP/1.1 204 NO-CONTENT\n"
#define HEADER_BAD_REQUEST "HTTP/1.1 400 BAD-REQUEST\n"
#define HEADER_UNAUTHORIZED "HTTP/1.1 401 UNAUTHORIZED\n"
#define HEADER_FORBIDDEN "HTTP/1.1 403 FORBIDDEN\n"
#define HEADER_NOT_FOUND "HTTP/1.1 404 NOT-FOUND\n"
#define HEADER_IM_A_TEAPOT "HTTP/1.1 418 IM-A-TEAPOT\n"
#define HEADER_INTERNAL_SERVER_ERROR "HTTP/1.1 500 INTERNAL-SERVER-ERROR\n"



typedef void *(*funcion_de_ruta)(request_t *request, response_t *response, void *aux);

typedef struct estructura_ruta
{
        funcion_de_ruta funcion;
        void *aux;
} estructura_ruta_t;

void free_response(response_t *response)
{
        if (!response)
                return;

        close(response->client_socket);

        if (response->json_data)
                free(response->json_data);
        
        
        free(response);
}


void *send_response(response_t *response)
{
        char *response_data = malloc(sizeof(char) * MAXLINE);

        if (response->status == OK)      
                snprintf(response_data, MAXLINE, HEADER_OK);
        if (response->status == CREATED)      
                snprintf(response_data, MAXLINE, HEADER_CREATED);
        if (response->status == NO_CONTENT)      
                snprintf(response_data, MAXLINE, HEADER_NO_CONTENT);
        if (response->status == BAD_REQUEST)      
                snprintf(response_data, MAXLINE, HEADER_BAD_REQUEST);
        if (response->status == UNAUTHORIZED)      
                snprintf(response_data, MAXLINE, HEADER_UNAUTHORIZED);
        if (response->status == FORBIDDEN)      
                snprintf(response_data, MAXLINE, HEADER_FORBIDDEN);
        if (response->status == NOT_FOUND)      
                snprintf(response_data, MAXLINE, HEADER_NOT_FOUND);
        if (response->status == IM_A_TEAPOT)      
                snprintf(response_data, MAXLINE, HEADER_IM_A_TEAPOT);
        if (response->status == INTERNAL_SERVER_ERROR)      
                snprintf(response_data, MAXLINE, HEADER_INTERNAL_SERVER_ERROR);
                
        if (response->json_data != NULL)
                strcat(response_data, "Access-Control-Allow-Origin: *\nConnection: Keep-alive\nContent-Type: application/json; charset=UTF-8\nKeep-Alive: timeout=5, max=999\r\n\r\n");

        if (response->json_data == NULL && response->status == NOT_FOUND)
                snprintf(response_data, MAXLINE, "HTTP/1.1 404 NOT-FOUND\r\n\r\nNOT-FOUND");

        if (response->data != NULL) {
                strcat(response_data, response->data);
        } else if (response->json_data != NULL) {
                strcat(response_data, response->json_data);
        }

        if(maneja_error(write(response->client_socket, response_data, strlen(response_data)))) {
                printf("rompimos todo XD");
        }

        free_response(response);
        free(response_data);
        return NULL;
}


json_t *get_url_and_method(char *headers, char *route_url, char *method, char *possible_param)
{
        char *header = strtok(headers, " ");
        
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
        }       //      

        char temp[MAXLINE/16];
        strcpy(temp, route_url);
        




        char todos_los_params[256];
        todos_los_params[0] = '\0';
        sscanf(route_url, "%*[^?]%s", todos_los_params);//messirve
        json_t *json_query_param = NULL;
        json_error_t error;
        if (todos_los_params[0] != '\0') {
                char convertilo_a_json[256];
                memset(convertilo_a_json, 0, 256);



                convertilo_a_json[0] = '{';
                convertilo_a_json[1] = '\n';


                char *tuki = strtok(todos_los_params, "?");
                tuki = strtok(tuki, "&");
                char claves[256];
                char valores[256];
                while (tuki) {
                        sscanf(tuki, "%[^=]=%[^=]", claves, valores);
                        strcat(claves, "\"");        
                        strcat(valores, "\"");
                        strcat(convertilo_a_json, "\"");
                        strcat(convertilo_a_json, claves);
                        strcat(convertilo_a_json, ":");
                        strcat(convertilo_a_json, "\"");
                        strcat(convertilo_a_json, valores);
                        strcat(convertilo_a_json, "\n");
                        tuki = strtok(NULL, "&");
                }
        

                strcat(convertilo_a_json, "}");
                //printf("%s \n", convertilo_a_json);
                json_query_param = json_loads(convertilo_a_json, 0, &error);
        }

        char *token = strtok(temp, "/");

        while (token != NULL) {
                if (token != NULL) 
                        strcpy(possible_param, token);
                
                //printf("el token es : %s \n", token);
                token = strtok(NULL, "/");
        }

        return json_query_param;
}


json_t *convert_to_json(char *request_data)
{
        if (request_data == NULL)
                return NULL;

        char *my_json = malloc(sizeof(char) * MAXLINE);
        memset(my_json, 0, MAXLINE);
        my_json[0] = '{'; 
        strcat(my_json, request_data);

        json_t *root;
        json_error_t error;

        root = json_loads(my_json, 0, &error);
        free(my_json);
        if (!root)
                return NULL;

        return root;
}


response_t *create_response(int client_socket)
{
        response_t *response = malloc(sizeof(response_t));

        if (!response)
                return NULL;

        response->data = NULL;
        response->status = 404;
        response->client_socket = client_socket;
        response->json_data = NULL;

        return response;
}


request_t *create_request(json_t *body, json_t *query)
{
        request_t *request = malloc(sizeof(request_t));

        if (!request)
                return NULL;

        request->body = NULL;
        request->query = NULL;

        if (body != NULL) 
                request->body = body;
        
        if (query != NULL)
                request->query = query;

        return request;
}

void free_request(request_t *request)
{
        if (!request)
                return;

        if (request->body != NULL) 
                json_decref(request->body);
        if (request->query != NULL)
                json_decref(request->query);
        
        free(request);
}

void *handle_connection(void *client_pointer, void *rutas)
{
        int client_socket = *((int *)client_pointer);
        free(client_pointer);

        char request_data[MAXLINE];
        memset(request_data, 0, MAXLINE);       
        
        if (read(client_socket, request_data, MAXLINE) < 0)
                return NULL;
        
        char *method = malloc(sizeof(char) * (MAXLINE/16));
        char *route_url= malloc(sizeof(char) * (MAXLINE/16));
        char *possible_param = malloc(sizeof(char) * (MAXLINE/16));
        memset(possible_param , 0, (MAXLINE/16));
        if (!method || !route_url)
                return NULL;
        //printf("request_data %s \n", request_data);

        char *token = strtok(request_data, "{");       
        if (!token) 
                return NULL;

        char *headers = malloc(sizeof(char) * MAXLINE);
        if (!headers)
                return NULL;

        memset(headers, 0, MAXLINE);
        strcpy(headers, token);
        token = strtok(NULL, "{");     
        
        json_t *json_query_param = get_url_and_method(headers, route_url, method, possible_param);
        
        
        for (size_t i = strlen(route_url); i > 0; i--) {
                if (route_url[i] == '/')
                        break;
                route_url[i] = '\0';
        }
        strcat(route_url, ":id");


        char la_ruta[512];
        memset(la_ruta, 0, 512);
        strcat(la_ruta, method);        
        strcat(la_ruta, route_url);
        response_t *response = create_response(client_socket);
        request_t *request = create_request(convert_to_json(token), json_query_param);  //valgrind dice que esto esta sin inicializar?
        estructura_ruta_t *estructura_ruta;
        possible_param = strtok(possible_param, "?");
        for (int i = 0; i < 2; i++) {
                estructura_ruta = hash_obtener(rutas, la_ruta);
                //printf("la_ruta es : %s \n", la_ruta);
                //printf("possible param es : %s \n", possible_param);
                if (estructura_ruta != NULL) {
                        if (i == 0) 
                                strcpy(request->params, possible_param);
                        
                        break;
                }
                if (la_ruta != NULL) 
                        strtok(la_ruta, ":");
                        
                
                
                //printf("possible param es : %s \n", possible_param);
                
                strcat(la_ruta, possible_param);
        }

        if (!response || !request)
                return NULL;
        
        free(route_url);
        free(method);
        free(headers);
        free(possible_param);

        if (estructura_ruta == NULL) {
                response->status = NOT_FOUND;
                send_response(response);
        }


        if (estructura_ruta != NULL)
                estructura_ruta->funcion(request, response, estructura_ruta->aux);

        free_request(request);
        
        return NULL;
}

void *crear_ruta(hash_t *hash, char *nombre_ruta, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_header tipo) //tipo, por si es get, post, put, etc
{
        estructura_ruta_t *estructura_ruta = malloc(sizeof(estructura_ruta_t));
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

response_t *set_status(response_t *response, http_status status)
{
        response->status = status;
        return response;
}

response_t *set_data_json(response_t *response, json_t *json_data)
{
        response->json_data = json_dumps(json_data, JSON_ENSURE_ASCII);
        return response;
}

char *get_param(request_t *request) 
{
        return request->params;
}