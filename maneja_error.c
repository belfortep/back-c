#include "maneja_error.h"

//simplemente un placeholder


int maneja_error(ssize_t valor)
{
        if (valor < 0)
                return -1;
        
        return 1;
}