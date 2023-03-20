
#include "string_parser.h"

/*
 *
 * Make an union of the method and the route_url to pass to the routes hash 
 * 
 */
char *get_route_of_hash(char *method, char *route_url)
{
        if (!method || !route_url)
                return NULL;

        char *route_of_hash = malloc(sizeof(char) * (SMALL_MAXLINE * 2));

        if (!route_of_hash)
                return NULL;

        strtok(route_url, "?");
        strcpy(route_of_hash, method);        
        strcat(route_of_hash, route_url);

        return route_of_hash;
}


/*
 *
 * Separate the route_url and method of a header
 * 
 */
void get_url_and_method(char *headers, char *method, char *route_url)
{
        if (!headers || !method || !route_url)
                return;

        char *header = strtok(headers, " ");
        
        int header_parse_counter = 0;
        while (header != NULL)
        {
                switch (header_parse_counter)
                {
                case 0:
                        strcpy(method, header);
                case 1:
                        strcpy(route_url, header);
                }
                header = strtok(NULL, " ");     
                header_parse_counter++;
        }
}

/*
 *
 * Eliminate the :param_name from the route and add the number of this params to the route_name, to use in the hash
 * 
 */
void create_route_with_no_params(char *route_name, char *token, int number_of_params)
{
        char temp[SMALL_MAXLINE];
        int i = 0;
        while (route_name[i] != '\0' && route_name[i] != ':') {
                token[i] = route_name[i];
                i++;
        }
        token[i] = '\0';
        sprintf(temp, "%d", number_of_params);
        token[strlen(token)-1] = '\0';
        strcat(token, temp);
}