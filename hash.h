#ifndef __HASH_H__
#define __HASH_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct hash hash_t;

/*
 * Crea el hash reservando la memoria necesaria para el.
 *
 * Capacidad indica la capacidad inicial con la que se crea el hash. La
 * capacidad inicial no puede ser menor a 3. Si se solicita una capacidad menor,
 * el hash se creará con una capacidad de 3.
 *
 * Devuelve un puntero al hash creado o NULL en caso de no poder crearlo.
 */
hash_t *hash_crear(size_t capacidad);

/*
 * Inserta o actualiza un elemento en el hash asociado a la clave dada.
 *
 * Si la clave ya existía y se reemplaza el elemento, se almacena un puntero al
 * elemento reemplazado en *anterior, si anterior no es NULL.
 *
 * Si la clave no existía y anterior no es NULL, se almacena NULL en *anterior.
 *
 * La función almacena una copia de la clave provista por el usuario,
 *
 * Nota para los alumnos: Recordar que si insertar un elemento provoca
 * que el factor de carga exceda cierto umbral, SE DEBE AJUSTAR EL
 * TAMAÑO DE LA TABLA PARA EVITAR FUTURAS COLISIONES.
 *
 * Devuelve el hash si pudo guardar el elemento o NULL si no pudo.
 */
hash_t *hash_insertar(hash_t *hash, const char *clave, void *elemento, void** anterior);

/*
 * Quita un elemento del hash y lo devuelve.
 *
 * Si no encuentra el elemento o en caso de error devuelve NULL
 */
void *hash_quitar(hash_t *hash, const char *clave);

/*
 * Devuelve un elemento del hash con la clave dada o NULL si dicho
 * elemento no existe (o en caso de error).
 */
void *hash_obtener(hash_t *hash, const char *clave);

/*
 * Devuelve true si el hash contiene un elemento almacenado con la
 * clave dada o false en caso contrario (o en caso de error).
 */
bool hash_contiene(hash_t *hash, const char *clave);

/*
 * Devuelve la cantidad de elementos almacenados en el hash o 0 en
 * caso de error.
 */
size_t hash_cantidad(hash_t *hash);

/*
 * Destruye el hash liberando la memoria reservada.
 */
void hash_destruir(hash_t *hash);

/*
 * Destruye el hash liberando la memoria reservada y asegurandose de
 * invocar la funcion destructora con cada elemento almacenado en el
 * hash.
 */
void hash_destruir_todo(hash_t *hash, void (*destructor)(void *));

/*
 * Recorre cada una de las claves almacenadas en la tabla de hash e invoca a la
 * función f, pasandole como parámetros la clave, el valor asociado a la clave y
 * el puntero auxiliar.
 *
 * Mientras que queden mas claves o la funcion retorne true, la
 * iteración continúa. Cuando no quedan mas claves o la función
 * devuelve false, la iteración se corta y la función principal
 * retorna.
 *
 * Devuelve la cantidad de claves totales iteradas (la cantidad de
 * veces que fue invocada la función) o 0 en caso de error.
 *
 */
size_t hash_con_cada_clave(hash_t *hash,
			   bool (*f)(const char *clave, void *valor, void *aux),
			   void *aux);

#endif /* __HASH_H__ */