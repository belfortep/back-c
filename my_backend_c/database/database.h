#ifndef DATABASE_H_
#define DATABASE_H_

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>
MYSQL *connect_db(char *host, char *user, char *password, char *db_name);
int create_db(MYSQL *connection, char *db_name);
MYSQL_RES *get_all(MYSQL *connection, char *table_name);
MYSQL_RES *get_by_id(MYSQL *connection, char *table_name, char *id);
int update_by_id(MYSQL *connection, char *table_name, char *id, json_t *object);
int delete_by_id(MYSQL *connection, char *table_name, char *id);
int insert_into(MYSQL *connection, char *table_name, json_t *object);
void disconnect(MYSQL *connection);

#endif // DATABASE_H_