/*
 * paginacion.h
 *
 *  Created on: 26 sep. 2021
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "init.h"
#include "memoria.h"
#include "algoritmo_reemplazo.h"

extern pthread_mutex_t mutex_RESERVAR_FRAME_SWAMP;


typedef enum{
	PREV_ALLOC,
	NEXT_ALLOC,
	IS_FREE,
} code_heap;

bool init_memoria(t_config_memoria* config, t_log* logger);
bool espacio_disponible(int id_proceso, int cant_pages);
bool esta_proceso_en_TGP(int id_process);
t_page* crear_pagina_nueva(t_pageTable* TP, int id_proceso);
void free_frame_memoria(int frame_memory);
void generar_nuevo_free_frame(t_page* new_page);
void gestion_de_victima(t_page* victim);
t_pageTable* get_tabla_de_paginas(int id_process);
int consultar_indice_TP(int id_process);
bool alloc_disponible(t_pageTable* TP, uint32_t size_to_reserve, heapMetadata* heapmetadata, int* dir_logica);
int consultar_frame(t_pageTable* TP, t_page* page);

void traer_data_de_memoria(t_pageTable* TP, int size_copy, void* data, int dir_logica);
void guardar_data_en_memoria(t_pageTable* TP, int size_copy, void* data, int dir_logica);
void borrar_data_en_memoria(t_pageTable* TP, int size_copy, int dir_logica);
void traer_page_a_memoria(t_page* page);
void actualizar_page(t_page* page);

void agregar_page_Memoria(t_page* new_page);
void ajustar_size_de_alloc(t_pageTable* TP, int size_reserve, heapMetadata* hpmtd_avalible, int dir_logica);
bool validar_dir_logica(t_pageTable* TP, int dir_logica, int* dirLog_heap, heapMetadata* heapmetadata);
void free_alloc(t_pageTable* TP, int dir_logica, heapMetadata* heapmetadata);


void terminar_paginacion(t_config_memoria* config);



/*	-------------------------------------------------------------------
						FUNCIONES para SWAMP
	-------------------------------------------------------------------*/
int encontrar_frame_libre_swamp(int id_proceso, int cant_pages);
int reservar_frame_libre_swamp(int id_proceso, bool reserve);
bool free_frame_swamp(int id_proceso, int frame_swap);
bool guardar_en_swamp(int id_proceso, int frame_swap, void* frame);
bool modificar_frame_swamp(int frame_memory, int frame_swap, t_page* victim);
bool pedir_page_a_swamp(int id_process, int page, void** page_swamp);


/*	-------------------------------------------------------------------
						FUNCIONES para HEAPMETADATA
	-------------------------------------------------------------------*/
void traer_heapmetadata_de_memoria(t_pageTable* TP, int dir_logica, heapMetadata* heapmetadata);
void actualizar_heapmetadata_en_memoria(t_pageTable* TP, heapMetadata* heapmetadata, int dir_logica, code_heap code_heap);


/*	-------------------------------------------------------------------
						FUNCIONES para BITMAP
	-------------------------------------------------------------------*/
bool init_bitmaps(t_config_memoria* config);
void actualizar_bitmap_FIJA(t_page* page, int frame);
int reservar_frame_libre_memoria(int id_process, bool reserve);
int encontrar_frame_libre_memoria(t_page* page, bool reserve);


#endif /* PAGINACION_H_ */
