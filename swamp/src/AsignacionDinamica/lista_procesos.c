/*
 * lista_procesos.c
 *
 *  Created on: 29 oct. 2021
 *      Author: utnso
 */
#include "lista_procesos.h"



bool existeProceso(int pid){

	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	if(list_find(LISTA_PROCESOS, (void*)mismoPid) == NULL)
		return false;
	return true;
}


bool crearProceso_D(int pid){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->pid = pid;
	proceso->archivo = buscarArchivoDisponible_D();
	if(proceso->archivo == NULL){
		free(proceso);
		return false;
	}


	char** listaDeArchivos = SWAMP_CFG->ARCHIVOS_SWAP;

	int indiceLista = 0;

	for(; strcmp(listaDeArchivos[indiceLista], proceso->archivo) != 0; indiceLista++){}

	proceso->indiceArchivo = indiceLista;

	list_add(LISTA_PROCESOS, (void*)proceso);


	return true;
}

void borrarEstructurasProcesos_D(int pid){
	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	void liberarProceso(t_proceso* proceso){
		free(proceso->archivo);
		free(proceso);
	}

	list_remove_and_destroy_by_condition(LISTA_PROCESOS, (void*)mismoPid, (void*)liberarProceso);
}

char* buscarArchivoDisponible_D(){
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
	if(tamanioMaximo == 0)
		return NULL;

	char* archivoDisponible = strdup(listaDeArchivos[indiceArchivoDisponible]);
	return archivoDisponible;
}


int tamanioArchivo(int indiceLista){
	int inicioArchivo = (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA) * indiceLista;
	int finArchivo = inicioArchivo + (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int framesDisponibles = 0;
	for(; inicioArchivo < finArchivo; inicioArchivo++){
		if(!bitarray_test_bit(BITMAP, inicioArchivo)){
			framesDisponibles++;
		}
	}
	return framesDisponibles;
}

int asignarEspacio(int pid){

	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);

	int inicioArchivo = (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA) * proceso->indiceArchivo;
	int finArchivo = inicioArchivo + (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	while(inicioArchivo < finArchivo && bitarray_test_bit(BITMAP, inicioArchivo)){
		inicioArchivo++;
	}
	if(inicioArchivo < finArchivo){
		bitarray_set_bit(BITMAP, inicioArchivo); //Reserva el espacio del frame disponible
		log_info(log_swamp, "Se reservó el frame %d, PID: %d", inicioArchivo, pid);
		return inicioArchivo; //Frame disponible
	}

	return -1;
}


int buscarFramesDisponibles_D(int pid){

	bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = (t_proceso*)list_find(LISTA_PROCESOS, (void*)mismoPid);

	int inicioArchivo = (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA) * proceso->indiceArchivo;
	int finArchivo = inicioArchivo + (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int cantidadDisponible = 0;
	while(inicioArchivo < finArchivo){
		if(!bitarray_test_bit(BITMAP, inicioArchivo)){
			cantidadDisponible++;
		}
		inicioArchivo++;
	}
	return cantidadDisponible;
}


int liberarPagina_D(int pid, int frameId){
	int indiceArchivo = frameId / (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	/*bool mismoPid(t_proceso* proceso){
		return proceso->pid == pid;
	}

	t_proceso* proceso = list_find(LISTA_PROCESOS, (void*)mismoPid);
	if(proceso->indiceArchivo != archivo){
		log_warning(log_swamp, "El frame solicitado pertenece a otro archivo (liberarPagina_D())");
		return -1;
	}*/
	if(!checkearFrame(pid, frameId)){
		return -2;
	}
	bitarray_clean_bit(BITMAP, frameId);
	void* archivo = list_get(ARCHIVOS, indiceArchivo);

	int subindiceFrame = frameId % (SWAMP_CFG->TAMANIO_SWAP/SWAMP_CFG->TAMANIO_PAGINA);
	int offset = subindiceFrame * SWAMP_CFG->TAMANIO_PAGINA;
	memset(archivo + offset, 0, SWAMP_CFG->TAMANIO_PAGINA);
	msync(archivo, (SWAMP_CFG->TAMANIO_SWAP), MS_SYNC);
	//list_add(ARCHIVOS, archivo);

	return frameId;
}










