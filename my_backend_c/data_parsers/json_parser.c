#include "json_parser.h"
#include "string_parser.h"

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
        
        
        if (max_age) {                  //intentar usar un solo strcat, creo que asi seria mas rapido
                strcat(request_data, "; Max-Age=");             
                strcat(request_data, json_string_value(max_age));
        }
        if (path) {
                strcat(request_data, "; Path=");
                strcat(request_data, json_string_value(path));
        }
        if (domain) {
                strcat(request_data, "; Domain=");
                strcat(request_data, json_string_value(domain));
        }
        if (secure) {
                strcat(request_data, "; Secure");
        }
        if (http_only) {
                strcat(request_data, "; HttpOnly");
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
 * Get the query param of a route and convert it to json_t
 * 
 */
json_t *get_query_param(char *all_the_params, json_error_t error)
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
                strcat(creating_json, ":\"");
                strcat(creating_json, valores);
                strcat(creating_json, ",\n");
                token = strtok(NULL, "&");
        }
        size_t len = strlen(creating_json);
        creating_json[len -2] = '\n';
        creating_json[len -1] = '}';
        creating_json[len] = '\0';

        return json_loads(creating_json, 0, &error);
}

/*
 *
 * Get the key-value pair of the cookies if exists
 * 
 */
json_t *parse_cookies(char *headers, json_error_t error)
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
                strcat(creating_json, ":\"");
                strcat(creating_json, values);
                strcat(creating_json, ",\n");
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
json_t *parse_request(char *headers, char *route_url, char *method, json_t **cookies)
{
        if (!headers || !route_url || !method)
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

        
        return json_query_param;
}

/*
 *
 * Convert a body of a request into json_t
 * 
 */
json_t *convert_body_to_json(char *request_data, char *content_length)
{
        if (!request_data || !content_length)
                return NULL;

        int length = 0;
        char *token = strtok(content_length, ":");
        token = strtok(NULL, ":");
        sscanf(token, "%i", &length);

        if (length > (MAXLINE * 1024))
                return NULL;

        char my_json[length];
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
 * Create a json with the keys from the route params (example, receive user/:id/:name
 * It will create a json like {"id" : "", "name" : ""}    
 * 
 */
json_t *create_json_from_params(char *route, int *number_of_params)
{
        json_t *json = json_object();
        json_t *string = json_string("");       //siempre me pierde 1 byte de memoria, supongo que es aceptable (?
        int i = 0;
        int j = 0;
        char temp_for_param_name[SMALL_MAXLINE];

        while (route[i] != '\0') {
                if (route[i] == ':') {
                        i++;
                        (*number_of_params)++;
                        while((route[i] != '\0') && (route[i] != ':') && (route[i] != '/')) {
                                temp_for_param_name[j] = route[i];
                                j++;
                                i++;
                        }
                        temp_for_param_name[j] = '\0';
                        json_object_set(json, temp_for_param_name, string);
                        j = 0;
                }

                i++;
        }

        return json;
}