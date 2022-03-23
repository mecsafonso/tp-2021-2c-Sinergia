/*
 * algortimo_reemplazo.c
 *
 *  Created on: 20 oct. 2021
 *      Author: utnso
 */

#include "algoritmo_reemplazo.h"

void actualizar_algoritmo_reemplazo_FIJA(t_page* page){
	bool buscar_proceso(t_bitmap_fija* entry){
		return (page->idProcess == entry->id_process);
	}

	t_bitmap_fija* nodo = (t_bitmap_fija*)list_find(BITMAPS_PROCESS_FIJA, (void*)buscar_proceso);

	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		eliminar_pagina_LRU(page);

		pthread_mutex_lock(&nodo->mutex_bitmap);
		for(int i = 0; i <  MEMORIA_CFG->MARCOS_MAXIMOS; i++){
			if(bitarray_test_bit(nodo->BITMAP_PAGES, i) == 1){
				bitarray_clean_bit(nodo->BITMAP_PAGES, i);
				pthread_mutex_unlock(&nodo->mutex_bitmap);
				break;
			}
		}
		pthread_mutex_unlock(&nodo->mutex_bitmap);
	}
	else if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
		eliminarPaginaClock(page);
		bitarray_clean_bit(nodo->BITMAP_PAGES, page->frame_asig_FIJA);
	}
}

void liberar_estructuras_admin_FIJA(int id_process){
	//se elimina la estrucura del bitmap del proceso
	bool esta_proceso_en_MEMORY_PAGES(t_bitmap_fija* entry){
		return id_process == entry->id_process;
	}

	t_bitmap_fija* bitmap = (t_bitmap_fija*)list_remove_by_condition(BITMAPS_PROCESS_FIJA, (void*)esta_proceso_en_MEMORY_PAGES);

	free(bitmap->BITMAP_PAGES);
	free(bitmap);

	//se elimina la estrucutra para el algoritmo de reemplazo
	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		bool esta_proceso_en_MEMORY_PAGES(t_pages_LRU_fija* entry){
			return id_process == entry->id_process;
		}

		pthread_mutex_lock(&mutex_paginasEnMemoria);
		t_pages_LRU_fija* nodo = (t_pages_LRU_fija*)list_remove_by_condition(MEMORY_PAGES, (void*)esta_proceso_en_MEMORY_PAGES);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);

		list_destroy(nodo->pages);
		free(nodo);
	}
	else{
		bool esta_proceso_en_POINTER_FIJA(t_pages_CLOCK_fija* entry){
			return id_process == entry->id_process;
		}

		pthread_mutex_lock(&mutex_paginasEnMemoria);
		t_pages_CLOCK_fija* nodo = (t_pages_CLOCK_fija*)list_remove_by_condition(POINTER_FIJA, (void*)esta_proceso_en_POINTER_FIJA);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);

		free(nodo->pages);
		free(nodo);
	}
}

t_bitmap_fija* get_nodo_bitmap_asignacion_fija(int id_process){

	bool esta_proceso_en_list_FIJA(t_bitmap_fija* entry){
		return id_process == entry->id_process;
	}

	pthread_mutex_lock(&mutex_memory_bitmap);
	t_bitmap_fija* nodo = (t_bitmap_fija*)list_find(BITMAPS_PROCESS_FIJA, (void*)esta_proceso_en_list_FIJA);
	pthread_mutex_unlock(&mutex_memory_bitmap);

	if( nodo == NULL){
		nodo = malloc(sizeof(*nodo));
		nodo->id_process = id_process;
		pthread_mutex_init(&nodo->mutex_bitmap, NULL);

		void* puntero_a_bits = malloc(1);

		pthread_mutex_lock(&nodo->mutex_bitmap);
		nodo->BITMAP_PAGES = bitarray_create(puntero_a_bits, MEMORIA_CFG->MARCOS_MAXIMOS);

		for(int counter = 0; counter <  MEMORIA_CFG->MARCOS_MAXIMOS; counter++)
			bitarray_clean_bit(nodo->BITMAP_PAGES, counter);
		pthread_mutex_unlock(&nodo->mutex_bitmap);

		pthread_mutex_lock(&mutex_memory_bitmap);
		list_add(BITMAPS_PROCESS_FIJA, (void*)nodo);
		pthread_mutex_unlock(&mutex_memory_bitmap);
	}
	return nodo;
}

/*	-------------------------------------------------------------------
						FUNCIONES para LRU
	-------------------------------------------------------------------*/
void init_LRU(void){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0 || strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0)
		MEMORY_PAGES = list_create();
	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");

}

t_pages_LRU_fija* get_nodo_memoria_asignacion_fija(int id_process){

	bool esta_proceso_en_MEMORY_PAGES(t_pages_LRU_fija* entry){
		return id_process == entry->id_process;
	}

	t_pages_LRU_fija* nodo = (t_pages_LRU_fija*)list_find(MEMORY_PAGES, (void*)esta_proceso_en_MEMORY_PAGES);

	if( nodo == NULL){
		nodo = malloc(sizeof(*nodo));
		nodo->id_process = id_process;
		nodo->pages = list_create();

		list_add(MEMORY_PAGES, (void*)nodo);
	}
	return nodo;
}

void nueva_pagina_memoria_LRU(t_page* new_page){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		list_add(MEMORY_PAGES, new_page);
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_LRU_fija* nodo = get_nodo_memoria_asignacion_fija(new_page->idProcess);
		list_add(nodo->pages, (void*)new_page);
	}
	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

t_page* elegir_victima_LRU(int id_process){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0)
		return (t_page*)list_remove(MEMORY_PAGES, 0);

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_LRU_fija* nodo = get_nodo_memoria_asignacion_fija(id_process);
		return (t_page*)list_remove(nodo->pages, 0);
	}
	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");

	return NULL;
}

void eliminar_pagina_LRU(t_page* page_used){ //TODO: REVISAR LA PARTE DEL WHILE

	bool verificar_page(t_page* page){
		return page_used->idProcess == page->idProcess && page_used->idPage == page->idPage;
	}

	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		list_remove_by_condition(MEMORY_PAGES, (void*)verificar_page);
	}
	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_LRU_fija* nodo = get_nodo_memoria_asignacion_fija(page_used->idProcess);

		list_remove_by_condition(nodo->pages, (void*)verificar_page);
	}
	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

void actualizar_pagina_LRU(t_page* page_used){
	eliminar_pagina_LRU(page_used);
	nueva_pagina_memoria_LRU(page_used);
}


/*	-------------------------------------------------------------------
						FUNCIONES para CLOCK
	-------------------------------------------------------------------*/
void init_ClockModificado(int memoriaParticiones){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		clock_index = 0;
		POINTER_DINAMICA = malloc(sizeof(t_element) * memoriaParticiones);
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		POINTER_FIJA = list_create();
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

t_pages_CLOCK_fija* esta_proceso_en_POINTER_FIJA(int id_process){
	bool verificar_proceso(t_pages_CLOCK_fija* entry){
		return id_process == entry->id_process;
	}

	return (t_pages_CLOCK_fija*)list_find(POINTER_FIJA, (void*)verificar_proceso);
}

t_pages_CLOCK_fija* get_nodo_memoria_asignacion_fija_clock(int id_process){
	t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(id_process);

	if( nodo == NULL){
		nodo = malloc(sizeof(*nodo));
		nodo->id_process = id_process;
		nodo->clock = 0;
		nodo->pages = malloc(sizeof(t_element)* MEMORIA_CFG->MARCOS_MAXIMOS);

		list_add(POINTER_FIJA, (void*)nodo);
	}
	return nodo;
}


void proxima_posicion_clockModificado(int id_process){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		if(clock_index < MEMORY_PARTITIONS - 1)
			clock_index++;
		else
			clock_index = 0;
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(id_process);

		if(nodo->clock < MEMORIA_CFG->MARCOS_MAXIMOS - 1)
			nodo->clock++;
		else
			nodo->clock = 0;
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

void nueva_pagina_memoria_clockModificado(t_page* new_page, int espacioLibre){
	t_element element;
	element.page = new_page;
	element.use = 1;

	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0)
		POINTER_DINAMICA[espacioLibre] = element;

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = get_nodo_memoria_asignacion_fija_clock(new_page->idProcess);

		nodo->pages[espacioLibre] = element;
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

t_page* elegir_victima_clockModificado(int id_process){ //TODO: ACTUALIZAR PARA ASIGNACION FIJA
	t_page* victim = NULL;
	int counter;

	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		for(int i = 1; i <= 4; i++){
			counter = 0;

			while(counter < MEMORY_PARTITIONS){
				switch(i){
					case 1:
					case 3:
						if(POINTER_DINAMICA[clock_index].page->modified == 0 && POINTER_DINAMICA[clock_index].use == 0)
							victim = POINTER_DINAMICA[clock_index].page;
						break;

					case 2:
					case 4:
						if(POINTER_DINAMICA[clock_index].page->modified == 1 && POINTER_DINAMICA[clock_index].use == 0){
							victim = POINTER_DINAMICA[clock_index].page;
						}
						else
							POINTER_DINAMICA[clock_index].use = 0;
						break;
				}

				proxima_posicion_clockModificado(id_process);
				if(victim != NULL)
					return victim;

				counter++;
			}
		}
		return victim;
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(id_process);

		for(int i = 1; i <= 4; i++){
			counter = 0;

			while(counter < MEMORIA_CFG->MARCOS_MAXIMOS){
				switch(i){
					case 1:
					case 3:
						if(nodo->pages[nodo->clock].page->modified == 0 && nodo->pages[nodo->clock].use == 0)
							victim = nodo->pages[nodo->clock].page;
						break;

					case 2:
					case 4:
						if(nodo->pages[nodo->clock].page->modified == 1 && nodo->pages[nodo->clock].use == 0){
							victim = nodo->pages[nodo->clock].page;
						}
						else
							nodo->pages[nodo->clock].use = 0;
						break;
				}

				proxima_posicion_clockModificado(id_process);
				if(victim != NULL)
					return victim;

				counter++;
			}
		}
		return victim;
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
	return NULL;
}

int buscar_pagina_clockModificado(t_page* page_used){
	int position = 0;

	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		while (position < MEMORY_PARTITIONS){
			if(POINTER_DINAMICA[position].page == page_used){
				return position;
			}
			position ++;
		}
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(page_used->idProcess);

		while (position < MEMORIA_CFG->MARCOS_MAXIMOS){
			if(nodo->pages[position].page == page_used){
				return position;
			}
			position ++;
		}
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");

	return ERROR_MEMORIA;
}

void eliminarPaginaClock(t_page* page_used){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		int position = buscar_pagina_clockModificado(page_used);
		POINTER_DINAMICA[position].page = NULL;
		POINTER_DINAMICA[position].use = 0;
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(page_used->idProcess);

		int position = buscar_pagina_clockModificado(page_used);
		nodo->pages[position].page = NULL;
		nodo->pages[position].use = 0;
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

void actualizarPaginaClock(t_page* page_used){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		int position = buscar_pagina_clockModificado(page_used);
		POINTER_DINAMICA[position].use = 1;
	}

	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_pages_CLOCK_fija* nodo = esta_proceso_en_POINTER_FIJA(page_used->idProcess);

		int position = buscar_pagina_clockModificado(page_used);
		nodo->pages[position].use = 1;
	}

	else
		log_warning(LOGGER, "Tipo de asignacion NO valido. Ingresar en archivo de config DINAMICA o FIJA");
}

