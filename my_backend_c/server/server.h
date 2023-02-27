#ifndef SERVER_H_
#define SERVER_H_

#include "../data_structures/data_structures.h"
#include "../router/routes.h"
#include <stdint.h>

int init_server(uint16_t port, hash_t *routes);


#endif // SERVER_H_