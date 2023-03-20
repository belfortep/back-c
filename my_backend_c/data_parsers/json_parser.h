#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_
#include "../imports/common_imports.h"
#include "../imports/constants.h"

void add_properties(char *request_data, json_t *properties);

void add_cookies_response(char *request_data, json_t *cookies, json_t *properties);

json_t *get_query_param(char *all_the_params, json_error_t error);

json_t *parse_cookies(char *headers, json_error_t error);

json_t *parse_request(char *headers, char *route_url, char *method, json_t **cookies);

json_t *convert_body_to_json(char *request_data);

json_t *create_json_from_params(char *route, int *number_of_params);

#endif // JSON_PARSER_H_