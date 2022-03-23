/*
 * administrar_procesos_fija.c
 *
 *  Created on: 9 nov. 2021
 *      Author: utnso
 */
#include "administrar_procesos_fija.h"

void ocuparBitmapGlobal(t_proceso* proceso){
	int inicioArchivo = (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA) * proceso->indiceArchivo;
	int finArchivo = inicioArchivo + (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int framesDisponibles = 0;
	for(; inicioArchivo < finArchivo && framesDisponibles < SWAMP_CFG->MARCOS_MAXIMOS; inicioArchivo++){
		if(!bitarray_test_bit(BITMAP, inicioArchivo)){
			if(framesDisponibles == 0){proceso->primerFrameOcupado = inicioArchivo;}
			bitarray_set_bit(BITMAP, inicioArchivo);
			framesDisponibles++;
		}
	}
}

void vaciarBitmapGlobal(t_proceso* proceso){

	int finArchivo = proceso->primerFrameOcupado + SWAMP_CFG->MARCOS_MAXIMOS;

	for(int inicioProceso = proceso->primerFrameOcupado; inicioProceso < finArchivo; inicioProceso++){
		if(bitarray_test_bit(BITMAP, inicioProceso)){
			bitarray_clean_bit(BITMAP, inicioProceso);
		}
	}
	log_info(log_swamp, "Se liberaron los frames desde el %d - %d para nuevos procesos", proceso->primerFrameOcupado, finArchivo - 1);
}


void crearBitmap(t_proceso* proceso){
	int bits_bitmap = SWAMP_CFG->MARCOS_MAXIMOS;

	int bytes_bitmap;
	if(bits_bitmap%8 == 0){
		bytes_bitmap = bits_bitmap/8;
	}
	else{
		bytes_bitmap = (bits_bitmap/8)+1;
	}


	void* bitmap_paginas = malloc(bytes_bitmap);
	proceso->bitmapProceso = bitarray_create_with_mode((char*)bitmap_paginas , bytes_bitmap , LSB_FIRST);

	size_t maximo = bitarray_get_max_bit(proceso->bitmapProceso);
	for(int i = 0; i < maximo; i++){
		bitarray_clean_bit(proceso->bitmapProceso, i);
	}
}

bool crearProceso_F(int pid){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pid = pid;
	proceso->archivo = buscarArchivoDisponible_F();
	if(proceso->archivo == NULL){
		free(proceso);
		return false;
	}


	char** listaDeArchivos = SWAMP_CFG->ARCHIVOS_SWAP;
	int indiceLista = 0;
	for(; strcmp(listaDeArchivos[indiceLista], proceso->archivo) != 0; indiceLista++){}
	proceso->indiceArchivo = indiceLista;

	ocuparBitmapGlobal(proceso);
	crearBitmap(proceso);


	list_add(LISTA_PROCESOS, (void*)proceso);


	return true;
}


void borrarEstructurasProcesos_F(int pid){
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	void liberarProceso(t_proceso* proceso){
		vaciarBitmapGlobal(proceso);
		free(proceso->archivo);
		bitarray_destroy(proceso->bitmapProceso);
		free(proceso);
	}

	list_remove_and_destroy_by_condition(LISTA_PROCESOS, (void*)mismoPid, (void*)liberarProceso);
}

char* buscarArchivoDisponible_F(){
	char** listaDeArchivos = SWAMP_CFG->ARCHIVOS_SWAP;
	int indiceLista = 0;
	int tamanioMaximo = 0;
	int indiceArchivoDisponible;
	while(listaDeArchivos[indiceLista] != NULL ){
		int tamanioDisponible = tamanioArchivo(indiceLista);
		if(tamanioDisponible > tamanioMaximo){
			tamanioMaximo = tamanioDisponible;
			indiceArchivoDisponible = indiceLista;
		}
		indiceLista++;
	}
	if(tamanioMaximo < SWAMP_CFG->MARCOS_MAXIMOS)
		return NULL;

	char* archivoDisponible = strdup(listaDeArchivos[indiceArchivoDisponible]);
	return archivoDisponible;
}



int buscarFramesDisponibles_F(int pid){

	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);


	int framesDisponibles = 0;

	for(uint16_t indice = 0; indice < SWAMP_CFG->MARCOS_MAXIMOS; indice++){
		if(!bitarray_test_bit(proceso->bitmapProceso, indice)){
			framesDisponibles++;
		}
	}
	return framesDisponibles;
}


int asignarEspacio_F(int pid){
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);


	for(uint16_t indice = 0; indice < SWAMP_CFG->MARCOS_MAXIMOS; indice++){
		if(!bitarray_test_bit(proceso->bitmapProceso, indice)){
			bitarray_set_bit(proceso->bitmapProceso, indice);
			return indice + proceso->primerFrameOcupado;
		}
	}
	return -1;
}


int liberarPagina_F(int pid, int frameId){
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);

	if(frameId >= proceso->primerFrameOcupado && frameId < proceso->primerFrameOcupado + SWAMP_CFG->MARCOS_MAXIMOS){

		if(!bitarray_test_bit(proceso->bitmapProceso, frameId - proceso->primerFrameOcupado)){
			log_warning(log_swamp, "El frame %d ya se encontraba liberado", frameId);
			return frameId;
		}

		bitarray_clean_bit(proceso->bitmapProceso, frameId - proceso->primerFrameOcupado);

		//CODIGO REPETIDO FUNCION(?
		void* archivo = list_get(ARCHIVOS, proceso->indiceArchivo);

		int subindiceFrame = frameId % (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
		int offset = subindiceFrame * SWAMP_CFG->TAMANIO_PAGINA;
		memset(archivo + offset, 0, SWAMP_CFG->TAMANIO_PAGINA);
		msync(archivo, (SWAMP_CFG->TAMANIO_SWAP), MS_SYNC);

		log_info(log_swamp, "Frame %d liberado con exito (frame del proceso: %d)", frameId, pid);
		return frameId;

	}else{
		log_warning(log_swamp, "El frame solicitado pertenece a otro proceso (liberarPagina()), frameID: %d, proceso: %d", frameId, proceso->pid);
		return -2;
	}

}

bool checkearFrame_F(int pid, int frame){
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);

	return (frame >= proceso->primerFrameOcupado && frame < proceso->primerFrameOcupado + SWAMP_CFG->MARCOS_MAXIMOS);
}

