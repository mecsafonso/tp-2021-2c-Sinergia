/*
 * paginacion.c
 *
 *  Created on: 26 sep. 2021
 *      Author: utnso
 */

#include "paginacion.h"

pthread_mutex_t mutex_paginasEnMemoria = PTHREAD_MUTEX_INITIALIZER;  //usado en LRU
pthread_mutex_t mutex_memory_bitmap = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_puertoSWAMP  = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_TGP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_TLB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_METRICS_TLB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_MEMORIA = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_TLB_bitmap = PTHREAD_MUTEX_INITIALIZER;




bool init_memoria(t_config_memoria* config, t_log* logger){
	MEMORY = malloc(config->TAMANIO_MEMORIA);	/*guarda la DATA posta*/
	MEMORY_PARTITIONS = config->TAMANIO_MEMORIA/config->TAMANIO_PAGINA; /*tamaño de pagina = tamano de marco*/

	if(!init_bitmaps(config))
		return false;

	if(strcmp(config->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0)
		init_LRU();

	else if(strcmp(config->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){
		init_ClockModificado(MEMORY_PARTITIONS);
	}

	else{
		log_warning(logger, "Algoritmo de reemplazo NO valido. Ingresar en archivo de config LRU O CLOCK_M");
		return false;
	}

	TGP = list_create(); //tabla general de procesos

	init_TLB();
	return true;
}


bool espacio_disponible(int id_proceso, int cant_pages){
	return encontrar_frame_libre_swamp(id_proceso, cant_pages);
}

bool esta_proceso_en_TGP(int id_process){
	pthread_mutex_lock(&mutex_TGP);
	t_link_element* elemento = TGP->head;
	pthread_mutex_unlock(&mutex_TGP);

	t_pageTable* entry;

	while (elemento != NULL){
		entry = (t_pageTable*)elemento->data;

		if(entry->idProcess == id_process){
			return true;
		}

		pthread_mutex_lock(&mutex_TGP);
		elemento = elemento->next;
		pthread_mutex_unlock(&mutex_TGP);
	}
	return false;
}


t_page* crear_pagina_nueva(t_pageTable* TP, int id_proceso){
	t_page* new_page = malloc(sizeof(*new_page));

	new_page->idProcess = id_proceso;
	new_page->memory_frame_number = encontrar_frame_libre_memoria(new_page, true);
	new_page->swap_frame_number = reservar_frame_libre_swamp(id_proceso, true);
	new_page->modified = true;
	pthread_mutex_lock(&mutex_TGP);
	new_page->idPage = list_is_empty(TP->pages) ?  0 : list_size(TP->pages);
	pthread_mutex_unlock(&mutex_TGP);

	if(new_page->memory_frame_number == -1){
		log_warning(LOGGER, "SEG. FAULT [PID:%d, Pag:%d] - crear new page", new_page->idProcess, new_page->idPage);
		generar_nuevo_free_frame(new_page);
	}
	else
		agregar_page_Memoria(new_page);

	//agregar la nueva pagina a la TP del proceso
	pthread_mutex_lock(&mutex_TGP);
	list_add(TP->pages, (void*)new_page);
	pthread_mutex_unlock(&mutex_TGP);

	return new_page;
}

void free_frame_memoria(int frame_memory){
	pthread_mutex_lock(&mutex_MEMORIA);
	memset(MEMORY + frame_memory * MEMORIA_CFG->TAMANIO_PAGINA, 0, MEMORIA_CFG->TAMANIO_PAGINA);
	pthread_mutex_unlock(&mutex_MEMORIA);
}


void generar_nuevo_free_frame(t_page* new_page){
	log_warning(LOGGER, "No hay espacio en memoria, comienza el proceso de SWAP");
	t_page* victim;

	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		victim = elegir_victima_LRU(new_page->idProcess);
		nueva_pagina_memoria_LRU(new_page);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}
	else{
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		victim = elegir_victima_clockModificado(new_page->idProcess);
		if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0)
			nueva_pagina_memoria_clockModificado(new_page, victim->memory_frame_number);
		else
			nueva_pagina_memoria_clockModificado(new_page, victim->frame_asig_FIJA);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}

	new_page->memory_frame_number = victim->memory_frame_number;
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0)
		new_page->frame_asig_FIJA = victim->frame_asig_FIJA;

	log_info(LOGGER, "Pagina Victima [PID: %d - PAG: %d - FRAME: %d] | Nueva entrada [PID: %d - PAG: %d]",victim->idProcess, victim->idPage, victim->memory_frame_number, new_page->idProcess, new_page->idPage);
	gestion_de_victima(victim);

	eliminar_entrada_en_tlb(victim);
}


void gestion_de_victima(t_page* victim){
	if(victim->modified){
		//if(!free_frame_swamp(victim->idProcess, victim->swap_frame_number)) //TODO SACAR ESA LINEA porque basla con embiar la sgte
			//log_info(LOGGER, "error en SWAMP");
		if(!modificar_frame_swamp(victim->memory_frame_number, victim->swap_frame_number, victim))
			log_info(LOGGER, "error en SWAMP");
	}

	free_frame_memoria(victim->memory_frame_number);
	victim->memory_frame_number = -1;
	victim->modified = false;
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0)
		victim->frame_asig_FIJA = -1;
}


void agregar_page_Memoria(t_page* new_page){
	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		nueva_pagina_memoria_LRU(new_page);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}
	else{
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0)
			nueva_pagina_memoria_clockModificado(new_page, new_page->memory_frame_number);
		else
			nueva_pagina_memoria_clockModificado(new_page, new_page->frame_asig_FIJA);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}
}



/*	-------------------------------------------------------------------
	-------------------------------------------------------------------*/
t_pageTable* get_tabla_de_paginas(int id_process){
	/*
	pthread_mutex_lock(&mutex_TGP);
	t_link_element* elemento = TGP->head;
	pthread_mutex_unlock(&mutex_TGP);
	t_pageTable* entry;

	while (elemento != NULL){
		 entry = (t_pageTable*)elemento->data;

		if(entry->idProcess == id_process){
			return entry;
		}

		pthread_mutex_lock(&mutex_TGP);
		elemento = elemento->next;
		pthread_mutex_unlock(&mutex_TGP);
	}

	return NULL;*/
	bool validar_proceso(t_pageTable* entry){
		return id_process == entry->idProcess;
	}
	pthread_mutex_lock(&mutex_TGP);
	t_pageTable* TP = (t_pageTable*)list_find(TGP, (void*)validar_proceso);
	pthread_mutex_unlock(&mutex_TGP);
	return TP->idProcess > 0? TP : NULL;
}


int consultar_indice_TP(int id_process){
	int indice = 0;

	pthread_mutex_lock(&mutex_TGP);
	t_link_element* elemento = TGP->head;
	pthread_mutex_unlock(&mutex_TGP);
	t_pageTable* entry;

	while (elemento != NULL){
		 entry = (t_pageTable*)elemento->data;

		if(entry->idProcess == id_process){
			return indice;
		}

		pthread_mutex_lock(&mutex_TGP);
		elemento = elemento->next;
		pthread_mutex_unlock(&mutex_TGP);

		indice++;
	}
	return -1;
}

bool alloc_disponible(t_pageTable* TP, uint32_t size_to_reserve, heapMetadata* heapmetadata,  int* dir_logica){

	pthread_mutex_lock(&TP->mutex_allocs);
	t_page* page = list_size(TP->pages) == 0 ? NULL: list_get(TP->pages, 0);
	pthread_mutex_unlock(&TP->mutex_allocs);

	*dir_logica = 0;
	int size_actual = 0;

	while (page != NULL){
		//t_page* page = (t_page*)elemento_page->data;

		//limpiar el heapmetada
		heapmetadata->prevAlloc = -2;
		heapmetadata->nextAlloc = -2;
		heapmetadata->isFree = -1;

		//traer el heapmetada de memoria
		traer_heapmetadata_de_memoria(TP, *dir_logica, heapmetadata);

		//consultar si es el ultimo alloc
		if(heapmetadata->nextAlloc == -1){
			pthread_mutex_lock(&TP->mutex_allocs);
			size_actual = (list_size(TP->pages) * MEMORIA_CFG->TAMANIO_PAGINA) - *dir_logica - SIZE_HEAPMETADATA;
			pthread_mutex_unlock(&TP->mutex_allocs);
		}
		else{
			size_actual = heapmetadata->nextAlloc - *dir_logica - SIZE_HEAPMETADATA;
		}

		//consultar si el heapmetadata cumple size
		if((size_actual == size_to_reserve && heapmetadata->nextAlloc != -1) || size_actual > size_to_reserve + SIZE_HEAPMETADATA){
			//NOTA: (size_to_reserve + SIZE_HEAPMETADATA) => para poder crear otro alloc en caso de dividir el actual

			//consultar - el alloc esta libre
			if(heapmetadata->isFree){
				//se reserva y se copia en memoria el flag de free del heap
				heapmetadata->isFree = false;
				actualizar_heapmetadata_en_memoria(TP, heapmetadata, *dir_logica, IS_FREE);
				page->modified = true;
				return true;
			}
		}

		if(heapmetadata->nextAlloc != -1){
			int nro_page = heapmetadata->nextAlloc / MEMORIA_CFG->TAMANIO_PAGINA;

			pthread_mutex_lock(&TP->mutex_allocs);
			page = (t_page*) list_get(TP->pages, nro_page);
			pthread_mutex_unlock(&TP->mutex_allocs);

			*dir_logica += SIZE_HEAPMETADATA + size_actual;
		}
		else
			page = NULL;
	}
	return false;
}


int calcular_offset(int frame, int* aux_offset, int dir_logica){
	*aux_offset = frame * MEMORIA_CFG->TAMANIO_PAGINA;
	return dir_logica % MEMORIA_CFG->TAMANIO_PAGINA;
}

int consultar_frame(t_pageTable* TP, t_page* page){
	int frame = ERROR_MEMORIA;

	if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0)
		frame = consultar_TLB(TP, page);
	else{
		if(page->memory_frame_number == -1){	// sino esta en memoria, se carga la pag
			log_warning(LOGGER, "SEG. FAULT - [PID: %d, Page: %d] - consultar frame", page->idProcess, page->idPage);
			traer_page_a_memoria(page);
		}
		else
			actualizar_page(page);

		frame = page->memory_frame_number;
	}

	return frame;
}

void traer_data_de_memoria(t_pageTable* TP, int size_copy, void* data, int dir_logica){
	int nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

	pthread_mutex_lock(&TP->mutex_allocs);
	t_page* page = (t_page*)list_get(TP->pages , nro_page);
	pthread_mutex_unlock(&TP->mutex_allocs);

	//cosultar page en TLB
	int frame = consultar_frame(TP, page);

	int aux_offset;
	int offset = calcular_offset(frame, &aux_offset, dir_logica);

	if(MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA) >= size_copy){
		//se copia la data completa
		pthread_mutex_lock(&mutex_MEMORIA);
		memcpy(data, MEMORY + aux_offset + offset, size_copy);
		pthread_mutex_unlock(&mutex_MEMORIA);
	}
	else{
		//se trae la primera parte de la data
		int aux_size = MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA);
		pthread_mutex_lock(&mutex_MEMORIA);
		memcpy(data, MEMORY + aux_offset + offset,  aux_size);
		pthread_mutex_unlock(&mutex_MEMORIA);

		int cont_size = aux_size;
		dir_logica += aux_size;

		//se trae el resto de la data
		while(cont_size < size_copy){
			//actualizacion de variables auxiliares
			aux_size = size_copy - cont_size > MEMORIA_CFG->TAMANIO_PAGINA ? MEMORIA_CFG->TAMANIO_PAGINA : size_copy - cont_size;
			nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

			pthread_mutex_lock(&TP->mutex_allocs);
			page = (t_page*)list_get(TP->pages, nro_page);
			pthread_mutex_unlock(&TP->mutex_allocs);

			frame = consultar_frame(TP, page);
			offset = calcular_offset(frame, &aux_offset, dir_logica);

			pthread_mutex_lock(&mutex_MEMORIA);
			memcpy(data + cont_size, MEMORY + aux_offset + offset, aux_size);
			pthread_mutex_unlock(&mutex_MEMORIA);

			cont_size += aux_size;
			dir_logica += aux_size;
		}
	}
}

void guardar_data_en_memoria(t_pageTable* TP, int size_copy, void* data, int dir_logica){
	int nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

	pthread_mutex_lock(&TP->mutex_allocs);
	t_page* page = (t_page*)list_get(TP->pages, nro_page);
	pthread_mutex_unlock(&TP->mutex_allocs);

	//cosultar page en TLB
	int frame = consultar_frame(TP, page);
	page->modified = true;

	int aux_offset;
	int offset = calcular_offset(frame, &aux_offset, dir_logica);
	int free_space = MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA);

	if(free_space >= size_copy){
		//se guarda la data completa
		pthread_mutex_lock(&mutex_MEMORIA);
		memcpy(MEMORY + aux_offset + offset, data, size_copy);
		pthread_mutex_unlock(&mutex_MEMORIA);
	}
	else{
		//se guarda la primera parte de la data
		int aux_size = MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA);
		pthread_mutex_lock(&mutex_MEMORIA);
		memcpy(MEMORY + aux_offset + offset, data, aux_size);
		pthread_mutex_unlock(&mutex_MEMORIA);

		int cont_size = aux_size;
		dir_logica += aux_size;

		//se guarda el resto de la data
		while(cont_size < size_copy){
			//actualizacion de variables auxiliares
			aux_size = size_copy - cont_size > MEMORIA_CFG->TAMANIO_PAGINA ? MEMORIA_CFG->TAMANIO_PAGINA : size_copy - cont_size;
			nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

			pthread_mutex_lock(&TP->mutex_allocs);
			t_page* new_page = (t_page*)list_get(TP->pages, nro_page);
			pthread_mutex_unlock(&TP->mutex_allocs);

			frame = consultar_frame(TP, new_page);
			new_page->modified = true;
			offset = calcular_offset(frame, &aux_offset, dir_logica);

			pthread_mutex_lock(&mutex_MEMORIA);
			memcpy(MEMORY + aux_offset + offset, data + cont_size, aux_size);
			pthread_mutex_unlock(&mutex_MEMORIA);

			cont_size += aux_size;
			dir_logica += aux_size;
		}
	}
}

void borrar_data_en_memoria(t_pageTable* TP, int size_copy, int dir_logica){
	int nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

	pthread_mutex_lock(&TP->mutex_allocs);
	t_page* page = (t_page*)list_get(TP->pages, nro_page);
	pthread_mutex_unlock(&TP->mutex_allocs);

	//cosultar page en TLB
	int frame = consultar_frame(TP, page);
	page->modified = true;

	int aux_offset;
	int offset = calcular_offset(frame, &aux_offset, dir_logica);

	if(MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA) >= size_copy){
		//se borra la data completa
		pthread_mutex_lock(&mutex_MEMORIA);
		memset(MEMORY + aux_offset + offset, 0, size_copy);
		pthread_mutex_unlock(&mutex_MEMORIA);
	}
	else{
		//se borra la primera parte del heap
		int aux_size = MEMORIA_CFG->TAMANIO_PAGINA - (dir_logica % MEMORIA_CFG->TAMANIO_PAGINA);
		pthread_mutex_lock(&mutex_MEMORIA);
		memset(MEMORY + aux_offset + offset, 0, aux_size);
		pthread_mutex_unlock(&mutex_MEMORIA);

		int cont_size = aux_size;
		dir_logica += aux_size;

		//se borra el resto del heap
		while(cont_size < size_copy){
			//actualizacion de variables auxiliares
			aux_size = size_copy - cont_size > MEMORIA_CFG->TAMANIO_PAGINA ? MEMORIA_CFG->TAMANIO_PAGINA : size_copy - cont_size;
			nro_page = dir_logica / MEMORIA_CFG->TAMANIO_PAGINA;

			pthread_mutex_lock(&TP->mutex_allocs);
			page = (t_page*)list_get(TP->pages, nro_page);
			pthread_mutex_unlock(&TP->mutex_allocs);

			frame = consultar_frame(TP, page);
			page->modified = true;
			offset = calcular_offset(frame, &aux_offset, dir_logica);

			pthread_mutex_lock(&mutex_MEMORIA);
			memset(MEMORY + aux_offset + offset, 0, aux_size);
			pthread_mutex_unlock(&mutex_MEMORIA);

			cont_size += aux_size;
			dir_logica += aux_size;
		}
	}
}

void ajustar_size_de_alloc(t_pageTable* TP, int size_reserve, heapMetadata* hpmtd_avalible, int dir_logica){
	heapMetadata* new_hpmtd = malloc(sizeof(*new_hpmtd));
	new_hpmtd->prevAlloc = dir_logica;
	new_hpmtd->nextAlloc = hpmtd_avalible->nextAlloc;
	new_hpmtd->isFree = true;

	//Actualizar heap actual
	hpmtd_avalible->nextAlloc = dir_logica + SIZE_HEAPMETADATA + size_reserve;
	actualizar_heapmetadata_en_memoria(TP, hpmtd_avalible, dir_logica, NEXT_ALLOC);

	//guardar nuevo heapmetadata
	guardar_data_en_memoria(TP, SIZE_HEAPMETADATA, (void*)new_hpmtd, hpmtd_avalible->nextAlloc);

	free(new_hpmtd);
}


void traer_page_a_memoria(t_page* page){
	page->memory_frame_number = encontrar_frame_libre_memoria(page, 1);

	//consultar si la pagina esta en memoria
	if(page->memory_frame_number == -1){
		log_warning(LOGGER, "SEG. FAULT [PID: %d, PAG: %d] - traer page a mem", page->idProcess, page->idPage);
		generar_nuevo_free_frame(page);
	}

	void* frame_swamp = malloc(MEMORIA_CFG->TAMANIO_PAGINA);
	pedir_page_a_swamp(page->idProcess, page->swap_frame_number, &frame_swamp);

	pthread_mutex_lock(&mutex_MEMORIA);
	memcpy(MEMORY + page->memory_frame_number * MEMORIA_CFG->TAMANIO_PAGINA, frame_swamp, MEMORIA_CFG->TAMANIO_PAGINA);
	pthread_mutex_unlock(&mutex_MEMORIA);

	log_info(LOGGER, "Se paso la informacion del frame %d de swap al %d de memoria principal", page->swap_frame_number, page->memory_frame_number);
	free(frame_swamp);

}

void actualizar_page(t_page* page){
	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		actualizar_pagina_LRU(page);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}
	else{
		pthread_mutex_lock(&mutex_paginasEnMemoria);
		actualizarPaginaClock(page);
		pthread_mutex_unlock(&mutex_paginasEnMemoria);
	}
}


bool validar_dir_logica(t_pageTable* TP, int dir_logica, int* dirLog_heap, heapMetadata* heapmetadata){
	pthread_mutex_lock(&TP->mutex_allocs);
	t_link_element* elemento_page = TP->pages->head;
	pthread_mutex_unlock(&TP->mutex_allocs);

	while (elemento_page != NULL){
		//limpiar el heapmetada
		heapmetadata->prevAlloc = -2;
		heapmetadata->nextAlloc = -2;
		heapmetadata->isFree = -1;

		//traer el heapmetada de memoria
		traer_heapmetadata_de_memoria(TP, *dirLog_heap, heapmetadata);

		//VALIDACION - consultar si es el ultimo alloc
		int range_max = 0;
		if(heapmetadata->nextAlloc == -1){
			pthread_mutex_lock(&TP->mutex_allocs);
			range_max = dir_logica < list_size(TP->pages) * MEMORIA_CFG->TAMANIO_PAGINA;
			pthread_mutex_unlock(&TP->mutex_allocs);
		}
		else
			range_max = dir_logica < heapmetadata->nextAlloc;

		if(*dirLog_heap + SIZE_HEAPMETADATA <= dir_logica  && range_max){
			return true;
		}

		if(heapmetadata->nextAlloc != -1){
			int nro_page = heapmetadata->nextAlloc / MEMORIA_CFG->TAMANIO_PAGINA;

			pthread_mutex_lock(&TP->mutex_allocs);
			elemento_page = (t_link_element*) list_get(TP->pages, nro_page);
			pthread_mutex_unlock(&TP->mutex_allocs);

			*dirLog_heap = heapmetadata->nextAlloc;
		}
		else
			elemento_page = NULL;
	}
	return false;
}


void free_data_memoria(int frame_memory, int offset, int sizeToFree){
	pthread_mutex_lock(&mutex_MEMORIA);
	memset(MEMORY + frame_memory * MEMORIA_CFG->TAMANIO_PAGINA + offset, 0, sizeToFree);
	pthread_mutex_unlock(&mutex_MEMORIA);
}


void free_alloc(t_pageTable* TP, int dir_logica, heapMetadata* heapmetadata){
	int dir_log_actual = dir_logica - SIZE_HEAPMETADATA;
	int nro_page = -1;
	//marcar flag de is_free
	heapmetadata->isFree = 1;
	actualizar_heapmetadata_en_memoria(TP, heapmetadata, dir_log_actual, IS_FREE);

	pthread_mutex_lock(&TP->mutex_allocs);
	int total_dir = list_size(TP->pages);
	pthread_mutex_unlock(&TP->mutex_allocs);

	int size_alloc = heapmetadata->nextAlloc != -1 ? heapmetadata->nextAlloc - dir_logica : (total_dir * MEMORIA_CFG->TAMANIO_PAGINA) - dir_logica;

	borrar_data_en_memoria(TP, size_alloc, dir_logica);


	/* ----------------------------------------------------------------------------------
	 * 								REVISION DE ALLOCS FREE CONTIGUOS
	 *----------------------------------------------------------------------------------*/
	void verificar_paginas_libres(t_pageTable* TP, int dir_logica_init, int dir_logica_fin){

		int init = dir_logica_init / MEMORIA_CFG->TAMANIO_PAGINA;

		for(int i = init; i <= (dir_logica_fin - 1) / MEMORIA_CFG->TAMANIO_PAGINA; i++){
			//consultar si es una page completa
			if( dir_logica_fin - dir_logica_init >= MEMORIA_CFG->TAMANIO_PAGINA){
				nro_page = dir_logica_init / MEMORIA_CFG->TAMANIO_PAGINA;

				pthread_mutex_lock(&TP->mutex_allocs);
				t_page* page = (t_page*)list_get(TP->pages, nro_page);
				pthread_mutex_unlock(&TP->mutex_allocs);

				//actualizar flag del frame en bitmap de memoria
				bitarray_clean_bit(MEMORY_BITMAP, page->memory_frame_number);
				log_info(LOGGER, "free frame en MEMORIA [PID: %d, PAG: %d, FRAME: %d]", page->idProcess, page->idPage, page->memory_frame_number);

				page->memory_frame_number = -1;

				//borrar entrada en la TLB
				eliminar_entrada_en_tlb(page);
			}
			dir_logica_init += MEMORIA_CFG->TAMANIO_PAGINA;
		}
	}


	//CONSOLIDAR HACIA ARRIBA
	if(heapmetadata->prevAlloc != -1){
		heapMetadata pre_alloc;
		traer_heapmetadata_de_memoria(TP, heapmetadata->prevAlloc, &pre_alloc);

		if(pre_alloc.isFree){
			//borrar heapmetadata a liberar de memoria
			borrar_data_en_memoria(TP, SIZE_HEAPMETADATA, dir_log_actual);

			dir_log_actual = heapmetadata->prevAlloc;
			pre_alloc.nextAlloc =  heapmetadata->nextAlloc;
			heapmetadata = &pre_alloc;

			actualizar_heapmetadata_en_memoria(TP, heapmetadata, dir_log_actual, NEXT_ALLOC);
		}
	}

	//CONSOLIDAR HACIA ABAJO
	if(heapmetadata->nextAlloc != -1){
		heapMetadata next_alloc;
		traer_heapmetadata_de_memoria(TP, heapmetadata->nextAlloc, &next_alloc);

		if(next_alloc.isFree){
			//borrar heapmetadata a liberar de memoria
			borrar_data_en_memoria(TP, SIZE_HEAPMETADATA, heapmetadata->nextAlloc);

			heapmetadata->nextAlloc =  next_alloc.nextAlloc;
			actualizar_heapmetadata_en_memoria(TP, heapmetadata, dir_log_actual, NEXT_ALLOC);
		}
	}

	//liberar paginas que queden libres de memoria
	pthread_mutex_lock(&TP->mutex_allocs);
	total_dir = list_size(TP->pages);
	pthread_mutex_unlock(&TP->mutex_allocs);

	int size_alloc_actual = heapmetadata->nextAlloc != -1? heapmetadata->nextAlloc - (dir_log_actual + SIZE_HEAPMETADATA): (total_dir * MEMORIA_CFG->TAMANIO_PAGINA) - (dir_log_actual + SIZE_HEAPMETADATA);
	int resto_page = MEMORIA_CFG->TAMANIO_PAGINA - ((dir_log_actual + SIZE_HEAPMETADATA) % MEMORIA_CFG->TAMANIO_PAGINA);

	if(size_alloc_actual > resto_page)
		verificar_paginas_libres(TP, dir_log_actual + SIZE_HEAPMETADATA + resto_page, heapmetadata->nextAlloc != -1? heapmetadata->nextAlloc : total_dir * MEMORIA_CFG->TAMANIO_PAGINA);
}


void terminar_paginacion(t_config_memoria* config){ //TODO: TERMINAR
	list_destroy(TGP);
	//list_destroy(TLB); no es una lista

	//se eliminan las estructuras para el algortimo de reemplazo MMU
	if(strcmp(config->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
		list_destroy(MEMORY_PAGES);
	}
	else if(strcmp(config->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0){

	}
}





/*	-------------------------------------------------------------------
						FUNCIONES para SWAMP
	-------------------------------------------------------------------*/

int encontrar_frame_libre_swamp(int id_proceso, int cant_pages){
	int frame = ERROR_SWAMP_U_OTRO;

	// Mensaje a SWAMP
	pthread_mutex_lock(&mutex_puertoSWAMP);
	if(FD_SWAMP != 0){
		if(!send_consultar_frame_libre_swamp(FD_SWAMP, id_proceso, cant_pages)){
			log_error(LOGGER, "Fallo en pedido de frame a swamp");
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return frame;
		}
	}

	//RTA de SWAMP
	op_code cop;
	if (recv(FD_SWAMP, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(LOGGER, "SWAMP DISCONNECT!");
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return frame;
	}
	if(cop == R_CONSULTAR_FRAME_LIBRE_SWAMP){
		if(!recv_R_consultar_frame_libre_swamp(FD_SWAMP, &frame)){
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return ERROR_SWAMP_U_OTRO;
		}
		if(frame == -1) //ACTUALIZAR CON UN BOOL, ESTO SERIA FALSE
			log_error(LOGGER, "SWAMP - no hay frames disponibles");

	}
	pthread_mutex_unlock(&mutex_puertoSWAMP);
	return frame;
}

int reservar_frame_libre_swamp(int id_proceso, bool reserve){
	int frame = ERROR_SWAMP_U_OTRO;

	// Mensaje a SWAMP
	pthread_mutex_lock(&mutex_puertoSWAMP);
	if(FD_SWAMP != 0){
		if(!send_reservar_frame_libre_swamp(FD_SWAMP, id_proceso, reserve)){
			log_error(LOGGER, "Fallo en reserva de frame a swamp");
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return frame;
		}
	}

	//RTA de SWAMP
	op_code cop;
	if (recv(FD_SWAMP, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(LOGGER, "SWAMP DISCONNECT!");
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return frame;
	}
	if(cop == R_RESERVAR_FRAME_LIBRE_SWAMP){
		if(!recv_R_reservar_frame_libre_swamp(FD_SWAMP, &frame)){
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return ERROR_SWAMP_U_OTRO;
		}
		if(frame == -1)
			log_error(LOGGER, "SWAMP - no hay frames disponibles");

	}
	pthread_mutex_unlock(&mutex_puertoSWAMP);
	return frame;
}

bool free_frame_swamp(int id_proceso, int frame_swap){
	// Mensaje a SWAMP
	pthread_mutex_lock(&mutex_puertoSWAMP);
	if(FD_SWAMP != 0){
		if(!send_free_frame_swamp(FD_SWAMP, id_proceso, frame_swap)){
			log_error(LOGGER, "Fallo en pedido free frame a swamp");
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return false;
		}
	}

	//RTA de SWAMP
	op_code cop;
	if (recv(FD_SWAMP, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(LOGGER, "SWAMP DISCONNECT!");
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}
	if(cop != R_FREE_FRAME_SWAMP){
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}

	int sucess;
	if(!recv_R_free_frame_swamp(FD_SWAMP, &sucess)){
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}
	else if(sucess < 0){
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}
	pthread_mutex_unlock(&mutex_puertoSWAMP);
	return true;
}

bool guardar_en_swamp(int id_proceso, int frame_swap, void* data){
	// Mensaje a SWAMP
	pthread_mutex_lock(&mutex_puertoSWAMP);
	if(FD_SWAMP != 0){
		if(!send_guardar_en_swamp(FD_SWAMP, id_proceso, frame_swap, data, MEMORIA_CFG->TAMANIO_PAGINA)){
			log_error(LOGGER, "Fallo en pedido guardar page en swamp");
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return false;
		}
	}

	//RTA de SWAMP
	op_code cop;
	if (recv(FD_SWAMP, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(LOGGER, "SWAMP DISCONNECT!");
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}
	if(cop == R_GUARDAR_EN_SWAMP){ //TODO: HACER SERIALIZACION Y AGREGAR CODIGO AL .h
		int sucess;
		if(!recv_R_guardar_en_swamp(FD_SWAMP, &sucess)){
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return false;
		}
	}
	pthread_mutex_unlock(&mutex_puertoSWAMP);
	return true;
}

bool modificar_frame_swamp(int frame_memory, int frame_swap, t_page* victim){
	void* frame = malloc(MEMORIA_CFG->TAMANIO_PAGINA);
	memcpy(frame, MEMORY + frame_memory * MEMORIA_CFG->TAMANIO_PAGINA, MEMORIA_CFG->TAMANIO_PAGINA);

	bool status = guardar_en_swamp(victim->idProcess, frame_swap, frame);

	victim->modified = false;

	free(frame);
	return status;
}


bool pedir_page_a_swamp(int id_process, int frame_swamp, void** page_swamp){
	//MENSAJE A SWAMP
	pthread_mutex_lock(&mutex_puertoSWAMP);
	if(FD_SWAMP != 0){
		if(!send_pedir_page_a_swamp(FD_SWAMP, id_process, frame_swamp)){
			log_error(LOGGER, "Fallo en pedido de page a swamp");
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return false;
		}
	}

	//RTA de SWAMP
	op_code cop;
	if (recv(FD_SWAMP, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(LOGGER, "SWAMP DISCONNECT!");
		pthread_mutex_unlock(&mutex_puertoSWAMP);
		return false;
	}
	if(cop == R_PEDIR_PAGE_A_SWAMP){ //TODO: HACER SERIALIZACION Y AGREGAR CODIGO AL .h
		if(!recv_R_pedir_page_a_swamp(FD_SWAMP, page_swamp)){
			pthread_mutex_unlock(&mutex_puertoSWAMP);
			return false;
		}
	}
	pthread_mutex_unlock(&mutex_puertoSWAMP);
	return true;
}




/*	-------------------------------------------------------------------
						FUNCIONES para HEAPMETADATA
	-------------------------------------------------------------------*/
void traer_heapmetadata_de_memoria(t_pageTable* TP, int dir_logica, heapMetadata* heapmetadata){
	void* data = malloc(SIZE_HEAPMETADATA);
	traer_data_de_memoria(TP, SIZE_HEAPMETADATA, data, dir_logica);

	memcpy(&heapmetadata->prevAlloc, data, sizeof(uint32_t));
	memcpy(&heapmetadata->nextAlloc, data + sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&heapmetadata->isFree, data + (sizeof(uint32_t) * 2), sizeof(uint8_t));

	free(data);
}


void actualizar_heapmetadata_en_memoria(t_pageTable* TP, heapMetadata* heapmetadata, int dir_logica, code_heap code_heap){

	int size_copy = sizeof(int32_t);

	switch (code_heap) {
		case PREV_ALLOC:
			guardar_data_en_memoria(TP, size_copy, &heapmetadata->prevAlloc, dir_logica );
			break;
		case NEXT_ALLOC:
			dir_logica += sizeof(int32_t);
			guardar_data_en_memoria(TP, size_copy, &heapmetadata->nextAlloc, dir_logica );
			break;
		case IS_FREE:
			dir_logica += (sizeof(int32_t) * 2);
			size_copy = sizeof(int8_t);
			guardar_data_en_memoria(TP, size_copy, &heapmetadata->isFree, dir_logica );
			break;
	}
}




/*	-------------------------------------------------------------------
						FUNCIONES para BITMAP
	-------------------------------------------------------------------*/
bool init_bitmaps(t_config_memoria* config){
	void* puntero_a_bits = malloc(1);
	void* puntero_a_bits2 = malloc(1);


	MEMORY_BITMAP = bitarray_create(puntero_a_bits, MEMORY_PARTITIONS);

	for(int counter = 0; counter < MEMORY_PARTITIONS; counter++)
		bitarray_clean_bit(MEMORY_BITMAP, counter);
		//MEMORY_BITMAP[counter] = 0;

	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0)
		BITMAPS_PROCESS_FIJA = list_create();

	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_TLB, "FIFO") == 0){
		TLB_BITMAP = bitarray_create(puntero_a_bits2, MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB);

		for(int counter = 0; counter < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB; counter++)
			bitarray_clean_bit(TLB_BITMAP, counter);
	}

	return true;
}



void actualizar_bitmap_FIJA(t_page* page, int frame){

	bool buscar_proceso(t_bitmap_fija* entry){
		return (page->idProcess == entry->id_process);
	}

	t_bitmap_fija* nodo = (t_bitmap_fija*)list_find(BITMAPS_PROCESS_FIJA, (void*)buscar_proceso);

	if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "CLOCK-M") == 0)
		bitarray_clean_bit(nodo->BITMAP_PAGES, page->frame_asig_FIJA);
	else{
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

}



int reservar_frame_libre_memoria(int id_process, bool reserve){
	pthread_mutex_lock(&mutex_memory_bitmap);
	for(int i = 0; i < MEMORY_PARTITIONS; i++){
		if(bitarray_test_bit(MEMORY_BITMAP, i) == 0){
			if(reserve)
				bitarray_set_bit(MEMORY_BITMAP, i);
			pthread_mutex_unlock(&mutex_memory_bitmap);
			return i;
		}
	}
	pthread_mutex_unlock(&mutex_memory_bitmap);

	return ERROR_MEMORIA;
}

int encontrar_frame_libre_memoria(t_page* page, bool reserve){
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		return reservar_frame_libre_memoria(page->idProcess, reserve);
	}
	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		t_bitmap_fija* nodo = get_nodo_bitmap_asignacion_fija(page->idProcess);

		pthread_mutex_lock(&nodo->mutex_bitmap);
		for(int i = 0; i < MEMORIA_CFG->MARCOS_MAXIMOS; i++){
			if(bitarray_test_bit(nodo->BITMAP_PAGES, i) == 0){
				if(reserve){
					bitarray_set_bit(nodo->BITMAP_PAGES, i);
					pthread_mutex_unlock(&nodo->mutex_bitmap);
					page->frame_asig_FIJA = i;
					return reservar_frame_libre_memoria(page->idProcess, reserve);
				}
			}
		}
		page->frame_asig_FIJA = ERROR_MEMORIA;
		pthread_mutex_unlock(&nodo->mutex_bitmap);
	}
	return ERROR_MEMORIA;
}


