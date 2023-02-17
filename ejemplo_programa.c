#include "rutas.h"
#include "server.h"
#include "stdio.h"
#define PORT 4000

void *funcion_de_ruta(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, 200);
        response = set_data(response, "data");

        return send_response(response);
}
void *funcion_de_otra_ruta(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, 200);
        //json_t *data = json_object_get(request->body, "clave1");
        //response = set_data(response, (char *)json_string_value(data));
        json_t *mando_info =json_pack("{sisi}", "foo", 100, "bar", 10);
        response = set_data_json(response, mando_info);


        return send_response(response);
}

void *funcion_info(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, *(int *)aux);
        response = set_data(response, "ola que tal");
        
        return send_response(response);
}

void *funcion_post(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, 200);
        response = set_data(response, "ola solo mirame en post");
        
        return send_response(response);
}



int main()
{
        hash_t *hash = hash_crear(10);        //cambiar que no necesite el tamanio inicial, cambiar nombre a "routes_t" o algo asi
        int numero = 200;
        crear_ruta(hash, "/index", funcion_de_ruta, NULL, GET);
        crear_ruta(hash, "/sarasa", funcion_de_otra_ruta, NULL, POST);
        crear_ruta(hash, "/api/info", funcion_info, &numero, PUT);
        crear_ruta(hash, "/ola", funcion_post, NULL, DELETE);
        
        iniciar_server(4000, hash);
        hash_destruir(hash);
}