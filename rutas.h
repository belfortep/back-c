#ifndef RUTAS_H_
#define RUTAS_H_

#include "hash.h"
typedef enum { GET, POST, PUT, DELETE } http_header;

typedef struct request{
        char *body;
        char *params;
        char *cookies;
        char *query;
} request_t;

typedef struct response{
        int status;
        void *data;
        int client_socket;
} response_t;

void *crear_ruta(hash_t *hash, char *nombre_ruta, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_header tipo);
void *handle_connection(void *client_pointer, void *rutas);

void *send_response(response_t *response);

response_t *set_status(response_t *response, int status);
response_t *set_data(response_t *response, void *data);

#endif // RUTAS_H_