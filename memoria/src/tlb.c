/*
 * tlb.c
 *
 *  Created on: 16 dic. 2021
 *      Author: utnso
 */


#include "tlb.h"
#include "paginacion.h"

void init_TLB() {
	if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0){
		TLB = list_create();
		TLB_METRICS = list_create(); //tabla para las metricas de la TLB
	}
}


int cargar_pagina_TLB(t_page* new_page){

	if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0){
		int posc = posicion_en_TLB(new_page);

		t_tlb* new_entry = malloc(sizeof(*new_entry));
		new_entry->idProcess = new_page->idProcess;
		new_entry->page = new_page->idPage;
		new_entry->frame_number = new_page->memory_frame_number;

		//if (strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_TLB, "LRU") == 0){
			//CASO 1: la pagina esta en la TLB
		if(posc > -1){
			//se elimina la entrada y se agrega al final para LRU
			if (strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_TLB, "LRU") == 0){
				pthread_mutex_lock(&mutex_TLB);
				list_remove(TLB, posc);
				list_add(TLB, (void*)new_entry);
				pthread_mutex_unlock(&mutex_TLB);
				return true;
			}
			else{
				free(new_entry);
			}
		}
		//CASO 2: la pagina NO esta en la TLB
		else{
			pthread_mutex_lock(&mutex_TLB);
			//si hay espacio libre en la TLB
			if(list_is_empty(TLB) || list_size(TLB) < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB){
				list_add(TLB, (void*)new_entry);
				pthread_mutex_unlock(&mutex_TLB);
				return true;
			}
			//si no hay espacio en la TLB
			else{
				list_remove(TLB, 0);
				list_add(TLB, (void*)new_entry);
				pthread_mutex_unlock(&mutex_TLB);
				return true;
			}
		}

	}
	return false;
}

int buscar_en_TLB(int id_process, int nro_page){

	bool buscar_nodo(t_tlb* entry){
		return entry->idProcess == id_process && entry->page == nro_page;
	}

	t_tlb* nodo = (t_tlb*)list_find(TLB, (void*) buscar_nodo);

	return nodo != NULL ? nodo->frame_number : ERROR_MEMORIA;
}

int consultar_TLB(t_pageTable* TP, t_page* page){

	bool buscar_metric_nodo(t_metrics_tlb* entry){
			return entry->id_process == page->idProcess;
		}

	bool buscar_nodo(t_tlb* entry){
		return entry->idProcess == page->idProcess && entry->page == page->idPage;
	}


	t_metrics_tlb* metric_nodo = (t_metrics_tlb*)list_find(TLB_METRICS, (void*)buscar_metric_nodo);

	t_tlb* nodo = (t_tlb*)list_find(TLB, (void*) buscar_nodo);
	int frame =  nodo != NULL ? nodo->frame_number : ERROR_MEMORIA;

	if(frame != -1 && frame == page->memory_frame_number){
/*HIT TLB*/		usleep(MEMORIA_CFG->RETARDO_ACIERTO_TLB*1000);

		metric_nodo->hit += 1;
		log_warning(LOGGER, "HIT TLB - [PID: %d, pag: %d, Marco: %d]", page->idProcess, page->idPage, frame);
		actualizar_page(page);
		return frame;
	}
	else{
/*MISS TLB*/	usleep(MEMORIA_CFG->RETARDO_FALLO_TLB*1000);
		metric_nodo->miss += 1;
		log_warning(LOGGER, "MISS TLB - [PID: %d, pag: %d]", page->idProcess, page->idPage);

		if(page->memory_frame_number == -1){	// sino esta en memoria, se carga la pag
			log_warning(LOGGER, "SEG. FAULT [PID: %d, PAG: %d] - consultar TLB" , page->idProcess, page->idPage);
			traer_page_a_memoria(page);
		}
		else
			actualizar_page(page);

		cargar_pagina_TLB(page);
		return page->memory_frame_number;
	}
	return ERROR_MEMORIA;
}

int posicion_en_TLB(t_page* page){
	int position = 0;

	pthread_mutex_lock(&mutex_TLB);
	t_link_element* elemento = TLB->head;
	pthread_mutex_unlock(&mutex_TLB);
	t_tlb* entry;

	while (elemento != NULL){
		 entry = (t_tlb*)elemento->data;

		if(entry->idProcess == page->idProcess && entry->page == page->idPage){
			return position;
		}
		position++;
		pthread_mutex_lock(&mutex_TLB);
		elemento = elemento->next;
		pthread_mutex_unlock(&mutex_TLB);
	}
	return ERROR_MEMORIA;
}


void eliminar_entrada_en_tlb(t_page* victima){
	if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0){
		int indice = posicion_en_TLB(victima);

		if (indice != -1){
			pthread_mutex_lock(&mutex_TLB);
			t_tlb* nodo = list_remove(TLB, indice);
			free(nodo);
			pthread_mutex_unlock(&mutex_TLB);
		}

		log_warning(LOGGER, "entrada elimada de TLB - [PID: %d, pag: %d, Marco: %d]", victima->idProcess, victima->idPage, victima->memory_frame_number);
	}
}

