/*
 * algoritmo_reemplazo.h
 *
 *  Created on: 20 oct. 2021
 *      Author: utnso
 */

#ifndef ALGORITMO_REEMPLAZO_H_
#define ALGORITMO_REEMPLAZO_H_

#include "init.h"
#include "paginacion.h"

extern pthread_mutex_t mutex_paginasEnMemoria;
extern pthread_mutex_t mutex_memory_bitmap;

extern t_config_memoria* MEMORIA_CFG;
extern t_log* LOGGER;

typedef struct {
	int id_process;
	t_bitarray* BITMAP_PAGES;
	pthread_mutex_t mutex_bitmap;
} t_bitmap_fija;

typedef struct {
	int id_process;
	t_list* pages;
} t_pages_LRU_fija;

void actualizar_algoritmo_reemplazo_FIJA(t_page* page);
void liberar_estructuras_admin_FIJA(int id_process);
t_bitmap_fija* get_nodo_bitmap_asignacion_fija(int id_process);


/*	-------------------------------------------------------------------
						LRU
	-------------------------------------------------------------------*/
void init_LRU(void);
void nueva_pagina_memoria_LRU(t_page* nuevaPagina);
t_page* elegir_victima_LRU(int id_process);
void eliminar_pagina_LRU(t_page* paginaUsada);
void actualizar_pagina_LRU(t_page*);


/*	-------------------------------------------------------------------
						CLOCK MODIFICADO
	-------------------------------------------------------------------*/
void init_ClockModificado(int);
void proxima_posicion_clockModificado(int id_process);
void nueva_pagina_memoria_clockModificado(t_page*, int );
t_page* elegir_victima_clockModificado(int id_process);
int buscar_pagina_clockModificado(t_page*);
void eliminarPaginaClock(t_page*);
void actualizarPaginaClock(t_page*);

#endif /* ALGORITMO_REEMPLAZO_H_ */
