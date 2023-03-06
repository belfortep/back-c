#ifndef ROUTES_H_
#define ROUTES_H_

#include "../data_structures/data_structures.h"
#include <jansson.h>
typedef enum { GET, POST, PUT, DELETE, ALL } http_code;
typedef enum { OK = 200, CREATED = 201, NO_CONTENT = 204, BAD_REQUEST = 400, UNAUTHORIZED = 401, FORBIDDEN = 403, NOT_FOUND = 404, IM_A_TEAPOT = 418, INTERNAL_SERVER_ERROR = 500 } http_status;

typedef struct _request_t request_t;

typedef struct _response_t response_t;

void *create_route(hash_t *hash, char *route_name, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_code type);

void *handle_connection(void *client_pointer, void *rutas);

void *send_response(response_t *response);

response_t *set_status(response_t *response, http_status status);

response_t *set_data(response_t *response, char *data);

response_t *set_data_json(response_t *response, json_t *json_data);

response_t *set_cookies(response_t *response, json_t *cookies, json_t *properties);

char *get_param(request_t *request);

json_t *get_body(request_t *request);

json_t *get_cookies(request_t *request);


#endif // ROUTES_H_