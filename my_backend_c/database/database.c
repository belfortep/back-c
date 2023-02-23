#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>



int create_db(MYSQL *connection, char *db_name)
{
        if (!connection || !db_name)
                return -1;

        char query[4096];
        sprintf(query, "CREATE DATABASE ");
        strcat(query, db_name);

        if (mysql_query(connection, query))
                return -1;
        
        return 0;
}


MYSQL_RES *get_all(MYSQL *connection, char *table_name)
{
        if (!connection || !table_name)
                return NULL;
        
        char query[4096];
        sprintf(query, "SELECT * FROM ");
        strcat(query, table_name);

        if (mysql_query(connection, query))
                return NULL;
        
        MYSQL_RES *result = mysql_store_result(connection);
        if (!result)
                return NULL;

        return result;
}





int insert_into(MYSQL *connection, char *table_name, json_t *object)
{
        if (!connection || !table_name || !object)
                return -1;
        
        char query[4096];
        const char *key;
        json_t *value;
        char all_keys[4096];
        memset(all_keys, 0, 4096);
        char all_values[4096];
        memset(all_values, 0, 4096);
        char fake_value[256];

        json_object_foreach(object, key, value) {
                if (json_is_array(value)) {     //creo que mysql no acepta arrays (?

                } else if (json_is_boolean(value)) {
                        if (json_boolean_value(value) == 1) {
                                strcat(all_values, "true");
                        } else {
                                strcat(all_values, "false");
                        }

                } else if (json_is_integer(value)) {
                        sprintf(fake_value, "%lld", json_integer_value(value));
                        strcat(all_values, fake_value);
                        
                } else if (json_is_string(value)) {
                        strcat(all_values, "'");  //para que la query lo acepte como string
                        strcat(all_values, json_string_value(value));
                        strcat(all_values, "'");
                        
                }
                strcat(all_keys, key);
                strcat(all_keys, ",");
                strcat(all_values, ",");
        }

        strcpy(query, "INSERT INTO ");
        strcat(query, table_name);
        strcat(query, "(");
        strcat(query, all_keys);
        query[strlen(query) - 1] = '\0';
        strcat(query, ")");
        strcat(query, " VALUES(");
        strcat(query, all_values);
        query[strlen(query) -1] = '\0';
        strcat(query, ")");
        if (mysql_query(connection, query))
                return -1;
        
        return 0;
}



MYSQL *connect_db(char *host, char *user, char *password, char *db_name)
{
        if (!host || !user || !password)
                return -1;

        MYSQL *connection = mysql_init(NULL);

        if (!connection)
                return NULL;
        
        
        if (!mysql_real_connect(connection, host, user, password, db_name, 0, NULL, 0))
                return NULL;
        

        return connection;
}

void disconnect(MYSQL *connection)
{
        if (!connection)
                return;

        mysql_close(connection);
}