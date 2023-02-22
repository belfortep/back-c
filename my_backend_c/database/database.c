#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
//la idea es que json_t *object podria ser el request->body (?
int insert_into(MYSQL *connection, char *table_name, json_t *object)
{
        char query[4096];

        strcpy(query, "INSERT INTO ");
        strcat(query, table_name);
        strcat(query, "(");
        
        //aca obtendria las llaves del json, y haria strcat(query, las_llaves)

        strcat(query, ")");
        strcat(query, " VALUES(");

        //aca obtendria los values del json y haria strcat(query, los_valores);
        strcat(query, ")");

        //creo que algo asi deberia funcionar?
        
        mysql_query(connection, query); 

        return 1;
}



MYSQL *connect_db(char *host, char *user, char *password, char *db_name)
{
        MYSQL *connection = mysql_init(NULL);

        if (!connection)
                return NULL;
        
        if (!mysql_real_connect(connection, host, user, password, db_name, 0, NULL, 0))
                return NULL;

        return connection;
}

void disconnect(MYSQL *connection)
{
        mysql_close(connection);
}