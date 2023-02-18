#include "hash.h"
#include <string.h>
#include <stdlib.h>

#define FACTOR_CARGA_MAXIMO 0.7
#define MINIMA_CAPACIDAD 3
#define ERROR 0
#define FACTOR_INCREMENTO_CAPACIDAD 2

typedef struct nodo_hash {
	char *clave;
	void *valor;
	struct nodo_hash *siguiente;	
}nodo_hash_t;

struct hash {
	size_t cantidad_elementos;
	size_t capacidad;
	nodo_hash_t **nodos;
};

/*
 * Compara si 2 cadenas de texto son iguales, devolviendo true en caso afirmativo, false en caso negativo
 *
 * string1 y string2 no deben ser NULL
 */
bool cadenas_iguales(char *string1, const char *string2)
{
	return (strcmp(string1, string2) == 0);
}

/*
 * Duplica el string recibido por parametro, devolviendo el puntero al duplicado.
 *
 * Se devuelve NULL si la funcion falla o no se recibe un string
 */
char *duplica_string(const char *string)
{
	if (!string)
		return NULL;

	char *nuevo_string = malloc(strlen(string) + 1);

	if (!nuevo_string)
		return NULL;

	strcpy(nuevo_string, string);
	
	return nuevo_string;
}

/*
 * Devuelve el valor del factor de carga del hash pasado por parametro.
 *
 * El hash no debe ser nulo. 
 */
float factor_carga(hash_t *hash)
{
	return (float)hash->cantidad_elementos/(float)hash->capacidad;
}

/*
 * Funcion de hash djb2, el string no debe ser nulo
 */
size_t funcion_hash(const char* str)
{       
        int hash = 5381;
        int c;
        while((c = *(str++)))
                hash = ((hash << 5) + hash) + c;

        return (size_t)hash;
}

hash_t *hash_crear(size_t capacidad)
{
	hash_t *hash = calloc(1, sizeof(hash_t));
	
	if (!hash)
		return NULL;

	if (capacidad < MINIMA_CAPACIDAD)
		capacidad = MINIMA_CAPACIDAD;
	
	hash->capacidad = capacidad;
	hash->nodos = calloc(1, sizeof(nodo_hash_t*) * capacidad);

	if (!hash->nodos)
		return NULL;
	
	return hash;
}

/*
 * Devuelve la posicion que corresponderia en el vector de nodos a la clave enviada.
 * 
 * La clave y el hash no pueden ser nulos.
 */
size_t obtener_posicion(hash_t *hash, const char *clave)
{
	return (funcion_hash(clave) % hash->capacidad);
}

/*
 * Crea un nuevo nodo de tipo par clave-valor para guardar en el hash, duplicando la clave enviada, se devuelve NULL en caso
 * de no poder reservar la memoria suficiente para el nodo o la clave
 * 
 * La clave no puede ser NULL
 */
nodo_hash_t *crear_nuevo_nodo(const char *clave, void *elemento)
{
	nodo_hash_t *nuevo_nodo = calloc(1, sizeof(nodo_hash_t));

	if (!nuevo_nodo)
		return NULL;
	
	nuevo_nodo->clave = duplica_string(clave);

	if (!nuevo_nodo->clave) {
		free(nuevo_nodo);
		return NULL;
	}
		
	nuevo_nodo->valor = elemento;

	return nuevo_nodo;
}

/*
 * Realiza un rehash de la tabla de hash, incrementando su capacidad por el factor_incremento_capacidad.
 * Devuelve true si fue realizado exitosamente, false si ocurrio un error.
 * 
 * El hash no puede ser NULL.
 */
bool hash_rehash(hash_t *hash)
{
	hash_t *hash_intermedio = hash_crear(hash->capacidad*FACTOR_INCREMENTO_CAPACIDAD);

	if (!hash_intermedio)
		return false;

	nodo_hash_t *nodo_aux = NULL;
	bool ocurrio_error = false;

	for (int i = 0; i < hash->capacidad; i++) {
		nodo_aux = hash->nodos[i];
		while (nodo_aux && !ocurrio_error) {
			if(hash_insertar(hash_intermedio, nodo_aux->clave, nodo_aux->valor, NULL) == NULL)
				ocurrio_error = true;

			nodo_aux = nodo_aux->siguiente;
		}
	}

	if (ocurrio_error) {
		hash_destruir(hash_intermedio);
		return false;
	}
		
	hash_t auxiliar = *hash;
	*hash = *hash_intermedio;
	*hash_intermedio = auxiliar;
	hash_destruir(hash_intermedio);
	
	return true;
}

hash_t *hash_insertar(hash_t *hash, const char *clave, void *elemento,
		      void **anterior)
{
	if (!hash || !clave)
		return NULL;

	if (anterior)
		*anterior = NULL;

	if (factor_carga(hash) > FACTOR_CARGA_MAXIMO) 
		if(!hash_rehash(hash))
			return NULL;

	size_t posicion = obtener_posicion(hash, clave);
	nodo_hash_t *nodo_anterior = NULL;	
	nodo_hash_t *nodo_adelante = hash->nodos[posicion];
	bool es_misma_clave = false;

	while (nodo_adelante && !es_misma_clave) {
		es_misma_clave = cadenas_iguales(nodo_adelante->clave, clave);
		nodo_anterior = nodo_adelante;
		nodo_adelante = nodo_adelante->siguiente;
	}

	if (es_misma_clave) {
		if (anterior)
			*anterior = nodo_anterior->valor;

		nodo_anterior->valor = elemento;
		return hash;
	}

	nodo_hash_t *nuevo_nodo = crear_nuevo_nodo(clave, elemento);

	if (!nuevo_nodo)
		return NULL;

	if (nodo_anterior) {
		nodo_anterior->siguiente = nuevo_nodo;
		hash->cantidad_elementos++;

		return hash;
	}

	hash->nodos[posicion] = nuevo_nodo;
	hash->cantidad_elementos++;

	return hash;
}

void *hash_quitar(hash_t *hash, const char *clave)
{
	if (!hash || !clave)
		return NULL;

	size_t posicion = obtener_posicion(hash, clave);
	nodo_hash_t *nodo_anterior = NULL;
	nodo_hash_t *nodo_adelante = hash->nodos[posicion];
	bool es_misma_clave = false;
	void *elemento_buscado = NULL;

	while (nodo_adelante && !es_misma_clave) {
		if (cadenas_iguales(nodo_adelante->clave, clave)) {
			es_misma_clave = true;
			elemento_buscado = nodo_adelante->valor;
		}
			
		if (!es_misma_clave) {
			nodo_anterior = nodo_adelante;
			nodo_adelante = nodo_adelante->siguiente;
		}
	}

	if (es_misma_clave) {
		if (nodo_anterior) {
			nodo_anterior->siguiente = nodo_adelante->siguiente;
		} else {
			hash->nodos[posicion] = nodo_adelante->siguiente;
		}
		
		free(nodo_adelante->clave);
		free(nodo_adelante);
		hash->cantidad_elementos--;
		
		return elemento_buscado;
	}

	return NULL;
}

void *hash_obtener(hash_t *hash, const char *clave)
{
	if (!hash || !clave)
		return NULL;

	size_t posicion = obtener_posicion(hash, clave);
	nodo_hash_t *nodo_buscado = hash->nodos[posicion];
	bool clave_encontrada = false;
	void *clave_buscada = NULL;

	if (!nodo_buscado)
		return NULL;

	while (nodo_buscado && !clave_encontrada) {
		if (cadenas_iguales(nodo_buscado->clave, clave)) {
			clave_encontrada = true;
			clave_buscada = nodo_buscado->valor;
		}

		nodo_buscado = nodo_buscado->siguiente;
	}

	return clave_buscada;
}

bool hash_contiene(hash_t *hash, const char *clave)
{
	if (!hash || !clave)
		return false;

	size_t posicion = obtener_posicion(hash, clave);
	nodo_hash_t *nodo_buscado = hash->nodos[posicion];
	bool clave_encontrada = false;

	if (!nodo_buscado)
		return false;

	while (nodo_buscado && !clave_encontrada) {
		clave_encontrada = cadenas_iguales(nodo_buscado->clave, clave);
		nodo_buscado = nodo_buscado->siguiente;
	}

	return clave_encontrada;
}

size_t hash_cantidad(hash_t *hash)
{
	if (!hash)
		return ERROR;

	return hash->cantidad_elementos;
}

void hash_destruir(hash_t *hash)
{
	if (!hash)
		return;
	
	hash_destruir_todo(hash, NULL);
}

void hash_destruir_todo(hash_t *hash, void (*destructor)(void *))
{
	if (!hash)
		return;

	nodo_hash_t *nodo_a_destruir = NULL;
	nodo_hash_t *auxiliar = NULL;
	
	for (int i = 0; i < hash->capacidad; i++) {
		nodo_a_destruir = hash->nodos[i];

		while (nodo_a_destruir) {
			auxiliar = nodo_a_destruir;
			if (destructor)
				destructor(auxiliar->valor);

			nodo_a_destruir = nodo_a_destruir->siguiente;
			free(auxiliar->clave);
			free(auxiliar);
		}
	}

	free(hash->nodos);
	free(hash);
}

size_t hash_con_cada_clave(hash_t *hash,
			   bool (*f)(const char *clave, void *valor, void *aux),
			   void *aux)
{
	if (!hash || !f)
		return ERROR;

	nodo_hash_t *nodo_actual = NULL;
	bool debo_iterar = true;
	size_t contador = 0;
	
	for (int i = 0; i < hash->capacidad; i++) {
		nodo_actual = hash->nodos[i];

		while (nodo_actual && debo_iterar) {
			debo_iterar = f(nodo_actual->clave, nodo_actual->valor, aux);
			nodo_actual = nodo_actual->siguiente;
			contador++;
		}
	}
	
	return contador;
}