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
#define SMALL_MAXLINE 256
#define HEADER_OK "HTTP/1.1 200 OK\n"
#define HEADER_CREATED "HTTP/1.1 201 CREATED\n"
#define HEADER_NO_CONTENT "HTTP/1.1 204 NO-CONTENT\n"
#define HEADER_BAD_REQUEST "HTTP/1.1 400 BAD-REQUEST\n"
#define HEADER_UNAUTHORIZED "HTTP/1.1 401 UNAUTHORIZED\n"
#define HEADER_FORBIDDEN "HTTP/1.1 403 FORBIDDEN\n"
#define HEADER_NOT_FOUND "HTTP/1.1 404 NOT-FOUND\n"
#define HEADER_IM_A_TEAPOT "HTTP/1.1 418 IM-A-TEAPOT\n"
#define HEADER_INTERNAL_SERVER_ERROR "HTTP/1.1 500 INTERNAL-SERVER-ERROR\n"

struct _request_t{
        json_t *body;
        char params[256];
        json_t *query;
        json_t *cookies;
};

struct _response_t{
        int status;
        int client_socket;
        char *data;
        char *json_data;
        json_t *cookies;
        json_t *cookies_properties;
};

typedef void *(*route_function)(request_t *request, response_t *response, void *aux);

typedef struct route_structure
{
        route_function function;
        void *aux;
} route_structure_t;

/*
 *
 * Close the socket associated to the response, and free all the memory
 * 
 */
static void free_response(response_t *response)
{
        if (!response)
                return;
        
        close(response->client_socket);

        if (response->json_data)
                free(response->json_data);
        
        if (response->cookies_properties)
                json_decref(response->cookies_properties);
        
        
        free(response);
}

/*
 *
 * Add properties to the cookies
 * 
 */
void add_properties(char *request_data, json_t *properties)
{
        if (!properties || !request_data)
                return;

        json_t *max_age = json_object_get(properties, "Max-Age");
        json_t *secure = json_object_get(properties, "Secure");
        json_t *http_only = json_object_get(properties, "HttpOnly");    
        json_t *path = json_object_get(properties, "Path");
        json_t *domain = json_object_get(properties, "Domain");
        
        
        if (max_age) {
                strcat(request_data, "; ");
                strcat(request_data, "Max-Age=");
                strcat(request_data, json_string_value(max_age));
        }
        if (path) {
                strcat(request_data, "; ");
                strcat(request_data, "Path=");
                strcat(request_data, json_string_value(path));
        }
        if (domain) {
                strcat(request_data, "; ");
                strcat(request_data, "Domain=");
                strcat(request_data, json_string_value(domain));
        }
        if (secure) {
                strcat(request_data, "; ");
                strcat(request_data, "Secure");
        }
        if (http_only) {
                strcat(request_data, "; ");
                strcat(request_data, "HttpOnly");
        }

}

/*
 *
 * Add the cookies and properties if exists to the response
 * 
 */
void add_cookies_response(char *request_data, json_t *cookies, json_t *properties)
{
        const char *key;
        json_t *value;

        json_object_foreach(cookies, key, value) {
                if (json_is_string(value)) {
                        strcat(request_data, "Set-Cookie: ");
                        strcat(request_data, key);
                        strcat(request_data, "=");
                        strcat(request_data, json_string_value(value));
                        add_properties(request_data, properties);
                        strcat(request_data, "\n");
                }
        }
}

/*
 *
 * Write into a socket the response, including the headers
 * 
 */
void *send_response(response_t *response)
{
        if (!response)
                return NULL;

        char response_data[MAXLINE];

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
                

        if (response->cookies)
                add_cookies_response(response_data, response->cookies, response->cookies_properties);

        if (response->data) {
                strcat(response_data, "Access-Control-Allow-Origin: *\nConnection: Keep-alive\nContent-Type: text/html; charset=UTF-8\nKeep-Alive: timeout=5, max=999\r\n\r\n");
                strcat(response_data, response->data); 
        }

        if (response->json_data && !response->data) {
                strcat(response_data, "Access-Control-Allow-Origin: *\nConnection: Keep-alive\nContent-Type: application/json; charset=UTF-8\nKeep-Alive: timeout=5, max=999\r\n\r\n");
                strcat(response_data, response->json_data);
        }

        if (!response->json_data && !response->data && response->status == NOT_FOUND)
                snprintf(response_data, MAXLINE, "HTTP/1.1 404 NOT-FOUND\r\n\r\nNOT-FOUND");


        if(maneja_error(write(response->client_socket, response_data, strlen(response_data))))
                printf("oops");
        

        free_response(response);

        return NULL;
}

/*
 *
 * Separate the route_url and method of a header
 * 
 */
static void get_url_and_method(char *headers, char *method, char *route_url)
{
        if (!headers || !method || !route_url)
                return;

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
        }
}

/*
 *
 * Get the query param of a route and convert it to json_t
 * 
 */
static json_t *get_query_param(char *all_the_params, json_error_t error)
{
        if (!all_the_params)
                return NULL;

        char creating_json[SMALL_MAXLINE];
        creating_json[0] = '{';
        creating_json[1] = '\n';
        creating_json[2] = '\0';

        char *token = strtok(all_the_params, "?");
        token = strtok(token, "&");
        char claves[SMALL_MAXLINE];
        char valores[SMALL_MAXLINE];

        while (token) {
                sscanf(token, "%[^=]=%[^=]", claves, valores);
                strcat(claves, "\"");        
                strcat(valores, "\"");
                strcat(creating_json, "\"");
                strcat(creating_json, claves);
                strcat(creating_json, ":");
                strcat(creating_json, "\"");
                strcat(creating_json, valores);
                strcat(creating_json, ",");
                strcat(creating_json, "\n");
                token = strtok(NULL, "&");
        }
        size_t len = strlen(creating_json);
        creating_json[len -2] = '\n';
        creating_json[len -1] = '\0';
        strcat(creating_json, "}");
        
        return json_loads(creating_json, 0, &error);
}

/*
 *
 * Try to get the possible param of a request
 * 
 */
static void get_possible_param(char *temp, char *possible_param)
{
        if (!temp || !possible_param)
                return;

        char *token = strtok(temp, "/");

        while (token != NULL) {
                if (token != NULL) 
                        strcpy(possible_param, token);
                
                token = strtok(NULL, "/");
        }
}

/*
 *
 * Get the key-value pair of the cookies if exists
 * 
 */
static json_t *parse_cookies(char *headers, json_error_t error)
{
        char *token = strstr(headers, "Cookie");

        if (!token)
                return NULL;

        token = strtok(token, ":");

        token = strtok(NULL, ":");
        char cookie[MAXLINE];
        sscanf(token, "%[^\n]", cookie);
        cookie[strlen(cookie) -1] = '\0';

        token = strtok(cookie, " ");
        char keys[SMALL_MAXLINE];
        char values[SMALL_MAXLINE];
        char creating_json[MAXLINE];
        creating_json[0] = '{';
        creating_json[1] = '\n';
        creating_json[2] = '\0';
        size_t len;
        while (token != NULL) {
                sscanf(token, "%[^=]=%[^=]", keys, values);
                len = strlen(values);
                if (values[len -1] == ';')
                        values[len -1] = '\0';
                strcat(keys, "\"");        
                strcat(values, "\"");
                strcat(creating_json, "\"");
                strcat(creating_json, keys);
                strcat(creating_json, ":");
                strcat(creating_json, "\"");
                strcat(creating_json, values);
                strcat(creating_json, ",");
                strcat(creating_json, "\n");
                token = strtok(NULL, " ");
        }
        len = strlen(creating_json);
        creating_json[len -2] = '\n';
        creating_json[len -1] = '}';
        creating_json[len] = '\0';

        return json_loads(creating_json, 0, &error);
}

/*
 *
 * Parse the request headers to get the route, method, params and query params
 * 
 */
static json_t *parse_request(char *headers, char *route_url, char *method, char *possible_param, json_t **cookies)
{
        if (!headers || !route_url || !method || !possible_param)
                return NULL;

        json_error_t error;
        *cookies = parse_cookies(headers, error);
        get_url_and_method(headers, method, route_url);

        char temp[SMALL_MAXLINE];
        strcpy(temp, route_url);
        char all_the_params[SMALL_MAXLINE];
        all_the_params[0] = '\0';
        sscanf(route_url, "%*[^?]%s", all_the_params);
        json_t *json_query_param = NULL;
        

        if (all_the_params[0] != '\0')
                json_query_param = get_query_param(all_the_params, error);
        
        get_possible_param(temp, possible_param);

        return json_query_param;
}

/*
 *
 * Convert a body of a request into json_t
 * 
 */
static json_t *convert_to_json(char *request_data)
{
        if (!request_data)
                return NULL;

        char my_json[MAXLINE];
        my_json[0] = '{'; 
        my_json[1] = '\0';
        strcat(my_json, request_data);

        json_t *root;
        json_error_t error;

        root = json_loads(my_json, 0, &error);
        if (!root)
                return NULL;

        return root;
}

/*
 *
 * Malloc the memory needed for a response
 * 
 */
static response_t *create_response(int client_socket)
{
        response_t *response = malloc(sizeof(response_t));

        if (!response)
                return NULL;

        response->data = NULL;
        response->json_data = NULL;
        response->cookies = NULL;
        response->cookies_properties = NULL;
        response->status = 404;
        response->client_socket = client_socket;

        return response;
}

/*
 *
 * Malloc the memory needed for a request
 * 
 */
static request_t *create_request(json_t *body, json_t *query, json_t *cookies)
{
        request_t *request = malloc(sizeof(request_t));

        if (!request)
                return NULL;

        request->body = body;
        request->query = query;
        request->cookies = cookies;

        return request;
}

/*
 *
 * Free the memory associated to the request
 * 
 */
static void free_request(request_t *request)
{
        if (!request)
                return;

        if (request->body != NULL) 
                json_decref(request->body);
        if (request->query != NULL)
                json_decref(request->query);
        if (request->cookies != NULL)
                json_decref(request->cookies);
        
        free(request);
}

/*
 *
 * Inner function needed to get the possible param
 * 
 */
static void reduce_route_url(char *route_url)
{       
        if (!route_url)
                return;

        for (size_t i = strlen(route_url); i > 0; i--) {
                if (route_url[i] == '/')
                        break;
                route_url[i] = '\0';
        }
        
        strcat(route_url, ":id");
}

/*
 *
 * Make an union of the method and the route_url to pass to the routes hash 
 * 
 */
static char *get_route_of_hash(char *method, char *route_url)
{
        if (!method || !route_url)
                return NULL;

        char *route_of_hash = malloc(sizeof(char) * (SMALL_MAXLINE * 2));

        if (!route_of_hash)
                return NULL;

        strcpy(route_of_hash, method);        
        strcat(route_of_hash, route_url);

        return route_of_hash;
}

/*
 *
 * Try to get the route_structure from the routes hash
 * 
 */
static route_structure_t *get_route(hash_t *routes, request_t *request, char *route_of_hash, char *possible_param)
{
        if (!routes || !request || !route_of_hash || !possible_param)
                return NULL;

        possible_param = strtok(possible_param, "?");
        if (!possible_param) {
                strtok(route_of_hash, ":");
                return hash_obtener(routes, route_of_hash);
        }
                
        
        route_structure_t *route_structure;
        for (int i = 0; i < 2; i++) {
                route_structure = hash_obtener(routes, route_of_hash);
                if (route_structure != NULL) {
                        if (i == 0) 
                                strcpy(request->params, possible_param);
                        
                        break;
                }
                if (route_of_hash != NULL) 
                        strtok(route_of_hash, ":");
                        
                strcat(route_of_hash, possible_param);
        }

        return route_structure;
}

/*
 *
 * Receive the connection socket, reads the request and use the associated function to the route in the request
 * 
 */
void *handle_connection(void *client_pointer, void *routes)
{
        if (!client_pointer || !routes)
                return NULL;

        int client_socket = *((int *)client_pointer);
        free(client_pointer);

        char request_data[MAXLINE];
        memset(request_data, 0, MAXLINE);       
        
        if (read(client_socket, request_data, MAXLINE) < 0)
                return NULL;
        
        char method[SMALL_MAXLINE];
        char route_url[SMALL_MAXLINE];
        char possible_param[SMALL_MAXLINE];
        possible_param[0] = '\0';
        char headers[MAXLINE];
        json_t *cookies = NULL;
        char *token = strtok(request_data, "{");

        if (!token)
                return NULL;

        strcpy(headers, token);
        token = strtok(NULL, "{");
        
        json_t *json_query_param = parse_request(headers, route_url, method, possible_param, &cookies);
        reduce_route_url(route_url);

        char *route_of_hash = get_route_of_hash(method, route_url);
        response_t *response = create_response(client_socket);
        request_t *request = create_request(convert_to_json(token), json_query_param, cookies);

        if (!request || !response || !route_of_hash)
                return NULL;

        route_structure_t *route_structure = get_route(routes, request, route_of_hash, possible_param);
        free(route_of_hash);

        if (route_structure == NULL) {
                if ((route_structure = hash_obtener(routes, "ALL/*")) != NULL) {
                        route_structure->function(request, response, route_structure->aux);
                } else {
                        response->status = NOT_FOUND;
                        send_response(response);
                }
                
                free_request(request);
                return NULL;
        }

        if (route_structure != NULL)
                route_structure->function(request, response, route_structure->aux);

        free_request(request);
        
        return NULL;
}


route_structure_t *create_route_structure(void *(*f)(request_t *request, response_t *response, void *aux), void *aux)
{
        if (!f)
                return NULL;

        route_structure_t *route_structure = malloc(sizeof(route_structure_t));

        if (!route_structure)
                return NULL;

        route_structure->function = f;
        route_structure->aux = aux;

        return route_structure;
}

/*
 *
 * Create a new possible route with the associated http_code
 * 
 */
void *create_route(hash_t *hash, char *route_name, void *(*f)(request_t *request, response_t *response, void *aux), void *aux, http_code type)
{
        if (!hash || !route_name || !f)
                return NULL;

        route_structure_t *route_structure = create_route_structure(f, aux);

        if (!route_structure)
                return NULL;
        
        char route_of_hash[SMALL_MAXLINE];

        if (type == GET) {
                strcpy(route_of_hash, "GET");
                strcat(route_of_hash, route_name);
        } else if (type == POST) {
                strcpy(route_of_hash, "POST");
                strcat(route_of_hash, route_name);
        } else if (type == PUT) {
                strcpy(route_of_hash, "PUT");
                strcat(route_of_hash, route_name);
        } else if (type == DELETE) {
                strcpy(route_of_hash, "DELETE");
                strcat(route_of_hash, route_name);
        } else if (type == ALL) {
                strcpy(route_of_hash, "ALL/*");
        }

        return hash_insertar(hash, route_of_hash, route_structure, NULL);
}

/*
 *
 * Set a string data to send as a response
 * 
 */
response_t *set_data(response_t *response, char *data)
{
        if (!response || !data)
                return NULL;

        response->data = data;
        return response;
}

/*
 *
 * Set the status of the response
 * 
 */
response_t *set_status(response_t *response, http_status status)
{
        if (!response)
                return NULL;

        response->status = status;
        return response;
}

/*
 *
 * Set a json data to send as a response
 * 
 */
response_t *set_data_json(response_t *response, json_t *json_data)
{
        if (!response || !json_data)
                return NULL;

        response->json_data = json_dumps(json_data, JSON_ENSURE_ASCII);
        return response;
}

/*
 *
 * Set cookies to send as a response
 * 
 */
response_t *set_cookies(response_t *response, json_t *cookies, json_t *properties)      
{
        if (!response || !cookies)
                return NULL;

        if (properties)
                response->cookies_properties = properties;

        response->cookies = cookies;
        return response;
}

/*
 *
 * Get the param of the request if exists
 * 
 */
char *get_param(request_t *request) 
{
        if (!request)
                return NULL;

        return request->params;
}

/*
 *
 * Get the body of a request
 * 
 */
json_t *get_body(request_t *request)
{
        if (!request)
                return NULL;

        return request->body;
}

/*
 *
 * Get the cookies of a request
 * 
 */
json_t *get_cookies(request_t *request)
{
        if (!request)
                return NULL;

        return request->cookies;
}