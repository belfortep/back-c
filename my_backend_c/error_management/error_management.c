#include "error_management.h"
#include <stdbool.h>
//simplemente un placeholder


int handle_error(ssize_t value)
{
        if (value < 0)
                return true;
        
        return false;
}