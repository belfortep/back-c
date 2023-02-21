#include "error_management.h"
#include <stdbool.h>
//simplemente un placeholder


int maneja_error(ssize_t valor)
{
        if (valor < 0)
                return true;
        
        return false;
}