#include "load_html.h"

/*
 *
 * Load a html file as a string in the second parameter.
 * 
 */
char *load_html(char *file_name, char *html_data, size_t size)
{
        if (!file_name || !html_data)
                return NULL;

        FILE *fd = fopen(file_name, "r");

        if (!fd)
                return NULL;

        memset(html_data, 0, size);

        if(fread(html_data, sizeof(char), size, fd) == 0)
                return NULL;

        fclose(fd);

        return html_data;
}