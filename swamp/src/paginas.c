/*
 * paginas.c
 *
 *  Created on: 9 oct. 2021
 *      Author: utnso
 */
#include "paginas.h"


int devolver_frame_disponible(int fd, int procedencia, void* proceso){

	int frameDisponible = 1;//buscar_frame_disponible(procedencia,proceso);

	if(procedencia == 0){
		if(frameDisponible != -1){
			return 1;
		}
		else return 0;
	}
	else{
		if(frameDisponible != -1){
			return frameDisponible;
		}
		else return 0;
	}

}



void crear_bitmap(){
	int tamanio_archivo = SWAMP_CFG->TAMANIO_SWAP;
	int tamanio_pagina = SWAMP_CFG->TAMANIO_PAGINA;
	int cantidad_de_archivos = string_array_size(SWAMP_CFG->ARCHIVOS_SWAP);
	//int cantidad_de_paginas = tamanio_archivo / tamanio_pagina;

	int bits_bitmap = (tamanio_archivo * cantidad_de_archivos)/tamanio_pagina;

	int bytes_bitmap;
	if(bits_bitmap%8 == 0){
		bytes_bitmap = bits_bitmap/8;
	}
	else{
		bytes_bitmap = (bits_bitmap/8)+1;
	}


	void* bitmap_paginas = malloc(bytes_bitmap);


	/*
	int newFile;

	newFile = open("bitmap.bin", O_CREAT | O_RDWR, 00600);										//todo: revisar bien la creacion del bitmap y la logica a seguir
																								//si hacer un bitmap para todos los archivos o uno para c/u
	if (newFile == -1){
		printf("error con shared memory");
		exit(1);
	}
	ftruncate(newFile, (tamanio_bitmap));
	close(newFile);
	log_info(log_swamp,"Se genero el archivo %s", "bitmap.bin");*/

	BITMAP = bitarray_create_with_mode((char*)bitmap_paginas , bytes_bitmap , LSB_FIRST);

	off_t maximo = bitarray_get_max_bit(BITMAP);
	for(int i = 0; i < maximo; i++){
		bitarray_clean_bit(BITMAP, i);
	}
}

void destruir_bitmap(){
	free(BITMAP->bitarray);
	bitarray_destroy(BITMAP);
}

bool checkearFrame(int pid, int frameId){
	if(!bitarray_test_bit(BITMAP, frameId)){
		log_warning(log_swamp, "El frame solicitado no le pertenece a ningun proceso (checkearFrame()), frameID: %d", frameId);
		return false;
	}
	int archivo = frameId / (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = list_find(LISTA_PROCESOS, (void*)mismoPid);
	if(proceso->indiceArchivo != archivo){
		log_error(log_swamp, "El frame solicitado pertenece a otro archivo (checkearFrame()), frameID: %d, archivo del proceso: %d", frameId, proceso->indiceArchivo);
		return false;
	}
	return true;
}

void escribirFrame(int frameId, void* datosAGuardar, int size_datosAGuardar){
	int indiceArchivo = frameId / (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	void* archivo = list_get(ARCHIVOS, indiceArchivo);
	int subindiceFrame = frameId % (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int offset = subindiceFrame * SWAMP_CFG->TAMANIO_PAGINA;

	memcpy(archivo + offset, datosAGuardar, size_datosAGuardar);
	msync(archivo, (SWAMP_CFG->TAMANIO_SWAP), MS_SYNC);
}

void* devolverFrame(int frameId){
	void* frame = malloc(SWAMP_CFG->TAMANIO_PAGINA);
	int indiceArchivo = frameId / (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	void* archivo = list_get(ARCHIVOS, indiceArchivo);
	int subindiceFrame = frameId % (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int offset = subindiceFrame * SWAMP_CFG->TAMANIO_PAGINA;

	memcpy(frame, archivo + offset, SWAMP_CFG->TAMANIO_PAGINA);

	return frame;
}










