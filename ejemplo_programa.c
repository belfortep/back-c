#include "my_backend_c/server/server.h"
#include "stdio.h"
//#include "my_backend_c/database/database.h"
#define PORT 7000

void *get_users(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, OK);
        //response = set_data_json(response, get_body(request));
        //MYSQL_RES *result = get_all(aux, "cars");     despues vemos bien que hacer con esto, pero es problema del usuario (?
        response = set_data(response, "some users");


        return send_response(response);
}

void *get_user(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, OK);
        //response = set_data_json(response, request->body);
        //response = set_data(response, "un usuario");
        //MYSQL_RES *result = get_by_id(aux, "cars", request->params);
        json_t *cookies = get_cookies(request);
        if (cookies) {
                response = set_data_json(response, cookies);
                json_t *properties = json_pack("{ssssss}", "Max-Age", "100000", "Secure", "no importa que este aca", "HttpOnly", "sarasa");
                response = set_cookies(response, cookies, properties);
        }
        //response = set_data_json(response, get_cookies(request));


        return send_response(response);
}

void *create_user(request_t *request, response_t *response, void *aux)
{
        //insert_into(aux, "cars", request->body);
        response = set_status(response, CREATED);
        response = set_data_json(response, get_body(request));

        return send_response(response);
}

void *delete_user(request_t *request, response_t *response, void *aux)
{
        //delete_by_id(aux, "cars", request->params);
        response = set_status(response, OK);
        response = set_data_json(response, get_params(request));

        return send_response(response);
}

void *update_user(request_t *request, response_t *response, void *aux)
{
        //update_by_id(aux, "cars", request->params, request->body);
        response = set_status(response, OK);
        response = set_data_json(response, get_body(request));

        return send_response(response);
}

void *not_found_function(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, NOT_FOUND);
        
        char html_data[4096];

        load_html("file_name.html", html_data, 4096);

        response = set_data(response, html_data);
        
        return send_response(response);
}


int main()
{
        /*char *host = "localhost";
        char *user = "paolo";
        char *password = "34klq*";
        char *db = "testdb";*/
        hash_t *hash = hash_crear(10);        //cambiar que no necesite el tamanio inicial, cambiar nombre a "routes_t" o algo asi
        //MYSQL *connection = connect_db(host, user, password, db);
        create_route(hash, "/", get_users, NULL, GET);
        create_route(hash, "/users", get_users, NULL, GET);
        create_route(hash, "/user/:id", get_user, NULL, GET);
        create_route(hash, "/user", create_user, NULL, POST);
        create_route(hash, "/user/:id", delete_user, NULL, DELETE);
        create_route(hash, "/user/:id", update_user, NULL, PUT);
        create_route(hash, "/*", not_found_function, NULL, ALL);
        
        init_server(PORT, hash);
        hash_destruir(hash);
        //disconnect(connection);
}