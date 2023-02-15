#include "rutas.h"
#include "server.h"
#include "stdio.h"

void *funcion_de_ruta(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, 200);
        response = set_data(response, "data");

        return send_response(response);
}
void *funcion_de_otra_ruta(request_t *request, response_t *response, void *aux)
{
        response = set_status(response, 200);
        response = set_data(response, "sarasa");

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

#define PORT 4000
int main()
{
        hash_t *hash = hash_crear(10);        //cambiar que no necesite el tamanio inicial, cambiar nombre a "routes_t" o algo asi
        int numero = 200;
        crear_ruta(hash, "/index", funcion_de_ruta, NULL, 0);
        crear_ruta(hash, "/sarasa", funcion_de_otra_ruta, NULL, 0);
        crear_ruta(hash, "/api/info", funcion_info, &numero, 0);
        crear_ruta(hash, "/ola", funcion_post, NULL, 1);
        
        start_server(PORT, hash);
}