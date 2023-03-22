#include "../imports/common_imports.h"
#include "../imports/constants.h"
#include "../error_management/error_management.h"
#include "routes.h"
#include "../data_parsers/string_parser.h"
#include "../data_parsers/json_parser.h"


struct _request_t{
        json_t *body;
        json_t *query;
        json_t *cookies;
        json_t *params;
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
        json_t *params;
} route_structure_t;



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
        request->params = NULL;

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
 * Malloc the memory needed for a route_structure
 * 
 */
static route_structure_t *create_route_structure(void *(*f)(request_t *request, response_t *response, void *aux), void *aux, json_t *params)
{
        if (!f)
                return NULL;

        route_structure_t *route_structure = malloc(sizeof(route_structure_t));

        if (!route_structure)
                return NULL;

        route_structure->function = f;
        route_structure->aux = aux;
        route_structure->params = params;

        return route_structure;
}

/*
 *
 * Free the memory associated to the json values in the route_structure
 * 
 */
static void free_route_structure_json_values(route_structure_t *route_structure)
{
        const char *key;
        json_t *value;
        json_object_foreach(route_structure->params, key, value) {
                json_decref(value);
        }

}

/*
 *
 * Send the response to the socket
 * 
 */
void *send_response(response_t *response)
{
        if (!response)
                return NULL;

        char response_data[MAXLINE];
        char header_and_response[MAXLINE];

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
                
        if (!response->json_data && !response->data && response->status == NOT_FOUND)
                snprintf(response_data, MAXLINE, "HTTP/1.1 404 NOT-FOUND\r\n\r\nNOT-FOUND");

        if (response->cookies)
                add_cookies_response(response_data, response->cookies, response->cookies_properties);

        if (response->data) {
                snprintf(header_and_response, MAXLINE, "Access-Control-Allow-Origin: *\nConnection: Keep-alive\nContent-Type: text/html; charset=UTF-8\nKeep-Alive: timeout=5, max=999\r\n\r\n");
                strcat(header_and_response, response->data);
                strcat(response_data, header_and_response);
        }

        if (response->json_data && !response->data) {
                snprintf(header_and_response, MAXLINE, "Access-Control-Allow-Origin: *\nConnection: Keep-alive\nContent-Type: application/json; charset=UTF-8\nKeep-Alive: timeout=5, max=999\r\n\r\n");
                strcat(header_and_response, response->json_data);
                strcat(response_data, header_and_response);
        }


        if(handle_error(write(response->client_socket, response_data, strlen(response_data))))
                printf("oops");
        

        free_response(response);

        return NULL;
}

/*
 *
 * Try to get the route_structure from the routes hash
 * 
 */
static route_structure_t *get_route(hash_t *routes, request_t *request, char *route_of_hash)
{
        route_structure_t *route_structure = hash_obtener(routes, route_of_hash);

        if (!route_structure) {
                char array_of_params[MAX_NUMBER_OF_PARAMS][SMALL_MAXLINE];
                size_t length = strlen(route_of_hash);
                int number_of_params = 0;
                char last_param[SMALL_MAXLINE];
                char *last_param_in_route = strrchr(route_of_hash, '/');
                strcpy(last_param, last_param_in_route);
                size_t last_param_lenght = strlen(last_param);
                char temp[SMALL_MAXLINE];
                
                while (route_structure == NULL || number_of_params > MAX_NUMBER_OF_PARAMS || last_param == NULL) {

                        strcpy(array_of_params[number_of_params], last_param);
                        number_of_params++;

                        while (last_param_lenght > 0) {
                                route_of_hash[length-1] = '\0';
                                length--;
                                last_param_lenght--;
                        }

                        sprintf(temp, "%d", number_of_params);
                        strcat(route_of_hash, temp);
                        length++;
                        route_structure = hash_obtener(routes, route_of_hash);
                        if (route_structure) {
                                const char *key;
                                json_t *value;
                                json_t *string;
                                number_of_params--;
                                json_object_foreach(route_structure->params, key, value) {
                                        string = json_string((array_of_params[number_of_params])+1);
                                        json_object_set(route_structure->params, key, string);
                                        number_of_params--;
                                }

                                request->params = route_structure->params;
                                break;
                        }
                        
                        route_of_hash[length-1] = '\0';
                        length--;
                        last_param_in_route = strrchr(route_of_hash, '/');

                        if (!last_param_in_route) {
                                strcpy(route_of_hash, "GET/");
                                strcat(route_of_hash, temp);
                                route_structure = hash_obtener(routes, route_of_hash);
                                break;
                        }

                        strcpy(last_param, last_param_in_route);
                        last_param_lenght = strlen(last_param);
                }
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
        char headers[MAXLINE];
        json_t *cookies = NULL;
        printf("request_data es: %s", request_data);
        //Content-Length: un_numero /n
        char *content_length = strstr(request_data, "Content-Length: ");
        char *token = strtok(request_data, "{");

        if (!token)
                return NULL;

        strcpy(headers, token);
        token = strtok(NULL, "{");
        
        json_t *json_query_param = parse_request(headers, route_url, method, &cookies);

        char *route_of_hash = get_route_of_hash(method, route_url);
        response_t *response = create_response(client_socket);
        request_t *request = create_request(convert_body_to_json(token, content_length), json_query_param, cookies);

        if (!request || !response || !route_of_hash)
                return NULL;

        route_structure_t *route_structure = get_route(routes, request, route_of_hash);
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
        free_route_structure_json_values(route_structure);
        
        return NULL;
}

/*
 *
 * Create the route key to add in the routes hash
 * 
 */
static void create_route_key_for_hash(char *route_of_hash, char *route_name, int number_of_params, http_code type)
{
        if (!route_of_hash || !route_name)
                return;

        if (type == GET) {
                strcpy(route_of_hash, "GET");
        } else if (type == POST) {
                strcpy(route_of_hash, "POST");
        } else if (type == PUT) {
                strcpy(route_of_hash, "PUT");
        } else if (type == DELETE) {
                strcpy(route_of_hash, "DELETE");
        } else if (type == ALL) {
                strcpy(route_of_hash, "ALL");
        }

        if (number_of_params == 0) {
                strcat(route_of_hash, route_name);
        } else {
                char token[SMALL_MAXLINE];
                create_route_with_no_params(route_name, token, number_of_params);
                strcat(route_of_hash, token);
        }
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

        int number_of_params = 0;
        route_structure_t *route_structure = create_route_structure(f, aux, create_json_from_params(route_name, &number_of_params));

        if (!route_structure)
                return NULL;
        
        char route_of_hash[SMALL_MAXLINE];
        create_route_key_for_hash(route_of_hash, route_name, number_of_params, type);

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
json_t *get_params(request_t *request) 
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

/*
 *
 * Get the url querys of a request
 * 
 */
json_t *get_querys(request_t *request)
{
        if (!request)
                return NULL;

        return request->query;
}

