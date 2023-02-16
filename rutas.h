#ifndef RUTAS_H_
#define RUTAS_H_

#include "hash.h"
typedef enum { GET, POST, PUT, DELETE } http_header;

typedef struct request{
        hash_t *body;
        hash_t *params;
        hash_t *cookies;
        hash_t *query;
} request_t;

typedef struct response{
        int status;
        int client_socket;
        char *data;
        char *claves[100];
        char *valores[100];
        int cantidad_pares;
} response_t;

void *crear_ruta(hash_t *hash, char *nombre_ruta, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_header tipo);
void *handle_connection(void *client_pointer, void *rutas);

void *send_response(response_t *response);

response_t *set_status(response_t *response, int status);
response_t *set_data(response_t *response, char *data);

#endif // RUTAS_H_