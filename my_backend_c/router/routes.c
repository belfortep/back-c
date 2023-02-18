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


typedef void *(*funcion_de_ruta)(request_t *request, response_t *response, void *aux);

typedef struct estructura_ruta
{
        funcion_de_ruta funcion;
        void *aux;
} estructura_ruta_t;

void *send_response(response_t *response)
{
        char *response_data = malloc(sizeof(char) * MAXLINE);

        if (response->status == 200)    //mejorar esta parte de los response
        {
                snprintf(response_data, MAXLINE, "HTTP/1.0 200 OK \r\n\r\n ");
        }

        if (response->data != NULL && response->json_data != NULL) {
                strcat(response_data, response->data);
                strcat(response_data, "\n");
                strcat(response_data, response->json_data);
        }
        else if (response->data != NULL) {
                strcat(response_data, response->data);
        } else if (response->json_data != NULL) {
                strcat(response_data, response->json_data);
        }

        //if(maneja_error(write(response->client_socket, response_data, strlen(response_data)))) {
        //}
                if(write(response->client_socket, response_data, strlen(response_data))){

                }
        close(response->client_socket);
        free(response);
        free(response_data);
        return NULL;
}

void *not_found(response_t *response)
{
        char *response_data = malloc(sizeof(char) * MAXLINE);

        snprintf(response_data, MAXLINE, "HTTP/1.0 404 NOT FOUND \r\n\r\nNOT FOUND PAGE");

        //if(maneja_error(write(response->client_socket, response_data, strlen(response_data)))){

        //}
        if(write(response->client_socket, response_data, strlen(response_data))){

        }
        close(response->client_socket);
        //memset(response_data, 0, MAXLINE);
        free(response);
        free(response_data);
        return NULL;
}


int get_url_and_method(char *headers, char *route_url, char *method)
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

        return 0;
}


json_t *convert_to_json(char *request_data)
{
        if (request_data == NULL)
                return NULL;

        char *my_json = malloc(sizeof(char) * MAXLINE);
        my_json[0] = '{';       //ver si esto anda o que onda
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


request_t *create_request(json_t *body)
{
        request_t *request = malloc(sizeof(request_t));

        if (!request)
                return NULL;

        request->body = body;

        return request;
}

void free_request(request_t *request)
{
        //hash_destruir(request->cookies);
        //hash_destruir(request->params);
        //hash_destruir(request->query);
        free(request);
}

void *handle_connection(void *client_pointer, void *rutas)
{
        int client_socket = *((int *)client_pointer);
        free(client_pointer);

        char request_data[MAXLINE];     //veamos ahora si no rompe el stack asi (?
        memset(request_data, 0, MAXLINE);       
        
        if (read(client_socket, request_data, MAXLINE) < 0)
                return NULL;
        
        char *method = malloc(sizeof(char) * (MAXLINE/16));
        char *route_url= malloc(sizeof(char) * (MAXLINE/16));
        if (!method || !route_url)
                return NULL;

        char *token = strtok(request_data, "{");       
        if (!token) 
                return NULL;

        char *headers = malloc(sizeof(char) * MAXLINE);
        if (!headers)
                return NULL;

        memset(headers, 0, MAXLINE);
        strcpy(headers, token);
        token = strtok(NULL, "{");     
        get_url_and_method(headers, route_url, method);
        
        char *la_ruta = malloc(sizeof(char) * (MAXLINE/8));    
        memset(la_ruta, 0, (MAXLINE/8));
        strcat(la_ruta, method);        
        strcat(la_ruta, route_url);
        //idea para obtener el :id, digamos que en route_url yo voy a tener algo del estilo
        //      /api/users/123456
        //      puedo ir haciendo strtok con los / hasta llegar al ultimo (que ese seria el :id)
        //      pero, y si me llega algo que no tiene un :id? digamos me llega /api/users
        //      y ese tambien es un endpoint valido, tipo digamos tengo las rutas GET /api/users y la GET /api/users/:id
        //      y tambien esta la ruta GET /api
        //      ahora, como verifico?
        //      quiza cambiar algo en la implementacion del hash, para verificar si tengo un : al insertar la clave?
        //por como esta ahora, si yo guardo un /user/:id
        //el hacer hash_obtener solo acepta si tenia un :id
        //AH, una idea, yo tengo mi /api/users, con mi idea de los strtok voy a obtener "users"
        //y si guardo un entero en el hash, tipo la estructura ruta guarde cuantas "/" tiene en el nombre
        //entonces yo cuando obtenga de por ejemplo
        // /api/users   aca pase 2 "/"
        //en cambio cuando agarre de algo tipo  /api/users/un_id
        //aca pase por 3 "/", por lo tanto la que quiero es la de :id, creo que tiene sentido y puedo implementarlo
        // asi se, se tiene 2 "/" es el normal, si tiene 3 es el :id
        //osea, en hash obtener tengo que verificar el string y la cantidad de / que pase?
        estructura_ruta_t *estructura_ruta = hash_obtener(rutas, la_ruta);

        response_t *response = create_response(client_socket);
        request_t *request = create_request(convert_to_json(token));

        if (!response || !request)
                return NULL;
        
        free(la_ruta);
        free(route_url);
        free(method);
        free(headers);

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