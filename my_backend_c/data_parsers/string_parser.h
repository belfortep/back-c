#ifndef STRING_PARSER_H_
#define STRING_PARSER_H_

#include "../imports/common_imports.h"
#include "../imports/constants.h"


void get_url_and_method(char *headers, char *method, char *route_url);

char *get_route_of_hash(char *method, char *route_url);

void create_route_with_no_params(char *route_name, char *token, int number_of_params);

#endif // STRING_PARSER_H_