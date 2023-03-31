#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "../../imports/common_imports.h"

//gcc lua_parser.c -o lua_parser -llua -L../lua -I../lua -lm -ldl 

lua_State *create_table_from_json(lua_State *L, json_t *json, char *table_name)
{

        if (!L || !json || !table_name)
                return NULL;

        lua_newtable(L);
        const char* key;
        json_t *value;
        json_object_foreach(json, key, value) {
                if (json_is_integer(value)) {
                        lua_pushstring(L, key);
                        lua_pushnumber(L, json_integer_value(value));
                        lua_settable(L, -3);
                }
                if (json_is_string(value)) {
                        lua_pushstring(L, key);
                        lua_pushstring(L, json_string_value(value));
                        lua_settable(L, -3);
                }
                if (json_is_boolean(value)) {
                        lua_pushstring(L, key);
                        lua_pushboolean(L, json_boolean_value(value));
                        lua_settable(L, -3);
                }
        }

        lua_setglobal(L, table_name);
        return L;
}       


lua_State *execute_lua_code(char *lua_code)
{
        if (!lua_code)
                return NULL;

        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        luaL_dostring(L ,lua_code);

        return L;
}