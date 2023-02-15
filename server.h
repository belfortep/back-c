#ifndef SERVER_H_
#define SERVER_H_

#include "hash.h"
#include <stdint.h>

int iniciar_server(uint16_t port, hash_t *rutas, void (*f)());


#endif // SERVER_H_