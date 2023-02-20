#ifndef ROUTES_H_
#define ROUTES_H_

#include "../data_structures/data_structures.h"
#include <jansson.h>
typedef enum { GET, POST, PUT, DELETE } http_header;
typedef enum { OK = 200, CREATED = 201, NO_CONTENT = 204, BAD_REQUEST = 400, UNAUTHORIZED = 401, FORBIDDEN = 403, NOT_FOUND = 404, IM_A_TEAPOT = 418, INTERNAL_SERVER_ERROR = 500 } http_status;

typedef struct request{
        json_t *body;
        char params[256];
        hash_t *cookies;
        json_t *query;
} request_t;

typedef struct response{
        int status;
        int client_socket;
        char *data;
        char *json_data;
} response_t;

void *crear_ruta(hash_t *hash, char *nombre_ruta, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_header tipo);
void *handle_connection(void *client_pointer, void *rutas);

void *send_response(response_t *response);

response_t *set_status(response_t *response, http_status status);
response_t *set_data(response_t *response, char *data);
response_t *set_data_json(response_t *response, json_t *json_data);
char *get_param(request_t *request);


#endif // ROUTES_H_