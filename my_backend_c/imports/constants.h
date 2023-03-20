#define MAXLINE 4096
#define SMALL_MAXLINE 256
#define HEADER_OK "HTTP/1.1 200 OK\n"
#define HEADER_CREATED "HTTP/1.1 201 CREATED\n"
#define HEADER_NO_CONTENT "HTTP/1.1 204 NO-CONTENT\n"
#define HEADER_BAD_REQUEST "HTTP/1.1 400 BAD-REQUEST\n"
#define HEADER_UNAUTHORIZED "HTTP/1.1 401 UNAUTHORIZED\n"
#define HEADER_FORBIDDEN "HTTP/1.1 403 FORBIDDEN\n"
#define HEADER_NOT_FOUND "HTTP/1.1 404 NOT-FOUND\n"
#define HEADER_IM_A_TEAPOT "HTTP/1.1 418 IM-A-TEAPOT\n"
#define HEADER_INTERNAL_SERVER_ERROR "HTTP/1.1 500 INTERNAL-SERVER-ERROR\n"
#define MAX_NUMBER_OF_PARAMS 10