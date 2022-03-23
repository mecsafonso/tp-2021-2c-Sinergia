/*
 * matelib.c
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#include "matelib.h"
#include "sharedlib.h"
#include "protocol.h"

int mate_init(mate_instance *lib_ref, char *config){

	lib_ref->logger = log_create(DIR_LOG, "MATE_PROCESS", true, LOG_LEVEL_INFO);
	lib_ref->semaforos = list_create();

	t_config* cfg = config_create(config); // @suppress("Type cannot be resolved")

	char* ip_backend = strdup(config_get_string_value(cfg,"IP_BACKEND"));
	char* puerto_backend = strdup(config_get_string_value(cfg,"PUERTO_BACKEND"));


	lib_ref->fd_backend = crear_conexion(lib_ref->logger,"BACKEND", ip_backend, puerto_backend);

	free(ip_backend);
	free(puerto_backend);
	config_destroy(cfg);

	if(lib_ref->fd_backend != 0){
		// Mensaje a Kernel
		if(!send_crear_proceso(lib_ref->fd_backend,0)){
			log_error(lib_ref->logger, "Fallo en el envio para la creacion del proceso");
			return 0;
		}

		log_info(lib_ref->logger, "Conexión realizada con éxito");

		op_code cop;
		if (recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
			log_info(lib_ref->logger, "DISCONNECT!");
			return 0;
		}
		if(cop == R_CREAR_PROCESO){
			if(!R_recv_crear_proceso(lib_ref->fd_backend, &lib_ref->pid)){
				return 0;
			}
			if(lib_ref->pid == -1){
				log_error(lib_ref->logger, "Kernel negó la creacion del preoceso");
				return 0;
			}
			else if(lib_ref->pid == -2){
				log_error(lib_ref->logger, "Memoria negó la creacion del preoceso");
				return 0;
			}

			log_info(lib_ref->logger, "Proceso %d creado correctamente", lib_ref->pid);
			return 1;
		}


		return 0;

	}else{
		log_error(lib_ref->logger, "Fallo en la creacion de la conexion");
		return 0;
	}

}


int mate_close(mate_instance *lib_ref){

	if(!send_finalizar_proceso(lib_ref->fd_backend, lib_ref->pid)){
		log_error(lib_ref->logger, "Fallo en el envio para la finalizacion del proceso");
		return 0;
	}

	int pid = lib_ref->pid;

	liberar_conexion(&lib_ref->fd_backend);

	void eliminarSem(mate_sem* sem){
		free(sem->nombre);
		sem_destroy(&sem->sem);
		free(sem);
	}

	list_clean_and_destroy_elements(lib_ref->semaforos, (void*) eliminarSem);

	log_info(lib_ref->logger, "proceso %d eliminada con exito", pid);

	log_destroy(lib_ref->logger);

	return 1;
}


//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value){


/*
	mate_sem* new_sem = malloc(sizeof(mate_sem));
	new_sem->nombre = sem;
	new_sem->valor = value;
    sem_init(&new_sem->sem,0,value);
    list_add(lib_ref->semaforos, new_sem); */

	send_iniciar_sem(lib_ref->fd_backend,lib_ref->pid, sem, value);

	int success;

	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);

	if(cop == R_INICIAR_SEM){
		recv_R_iniciar_sem(lib_ref->fd_backend, &success);
		log_warning(lib_ref->logger, "Se creó el semaforo %s",sem);
	}
	else{
		log_warning(lib_ref->logger, "Código de mensaje incorrecto (mate_sem_init()) %d",cop);
	}

	return success;
}


int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem){

	/*
	mate_sem* aux = buscarSem(lib_ref, sem);
	if(aux == NULL){return 0;}

	if(--aux->valor > 0){
		sem_wait(&aux->sem);
		return 1;
	}

	// TODO Mensaje a kernel: pasar carpincho a Block
	//send_sem_bloquear_proceso(lib_ref->fd_backend, lib_ref->pid);
	sem_wait(&aux->sem);
	// TODO Mensaje a kernel: pasar carpincho a Ready
	//send_sem_desbloquear_proceso(lib_ref->fd_backend, lib_ref->pid);
	*/
	log_warning(lib_ref->logger, "Se quiere hacer un wait al semaforo %s",sem);
	send_wait_sem(lib_ref->fd_backend,lib_ref->pid, sem);
	// int success;
	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);

	if(cop == R_WAIT_SEM){
		// recv_R_wait_sem(lib_ref->fd_backend);
		log_warning(lib_ref->logger, "El proceso %d agarra el semaforo %s", lib_ref->pid, sem);
		return 1;
	}
	else if(cop == R_WAIT_SEM_ELIMINADO){

		sem_t sem;
		sem_init(&sem,1,0);
		sem_wait(&sem);
		return 1;
	}
	else{
		log_warning(lib_ref->logger, "Código de mensaje incorrecto (mate_sem_wait()) %d, pid: %d",cop, lib_ref->pid);
		return -1;
	}

}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem){
	/*
	mate_sem* aux = buscarSem(lib_ref, sem);
	if(aux == NULL){return 0;}

	aux->valor += 1;
	sem_post(&aux->sem);

	// TODO Mensaje a kernel
	*/
	return send_post_sem(lib_ref->fd_backend,lib_ref->pid, sem);
}


int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem){

	/*
	mate_sem* aux = buscarSem(lib_ref, sem);
	if(aux == NULL){return 0;}

	bool buscar_sem(mate_sem* semaforo){
	  return (strcmp(semaforo->nombre,sem) == 0);
	}

	void eliminarSem(mate_sem* sem){
		free(sem->nombre);
		sem_destroy(&sem->sem);
		free(sem);
	}

	sem_destroy(&aux->sem);
	list_remove_and_destroy_by_condition(lib_ref->semaforos,(void*)buscar_sem,(void*)eliminarSem);
	 */


	return send_destroy_sem(lib_ref->fd_backend,lib_ref->pid, sem);
}




//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){


	// Mensaje a kernel, pasar proceso a Block


	// sleep(duracion) la duracion me la manda kernel como respuesta, le pregunto con el io

	// Mensaje a kernel, pasar proceso a Ready

	send_call_io(lib_ref->fd_backend,lib_ref->pid, io, msg);


	log_info(lib_ref->logger, "Pidiendo operacion IO, recurso: %s",io);
	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);

	if(cop == CALL_IO){
		log_info(lib_ref->logger, "IO realizada con éxito");
		return 1;
	}
	else{
		log_error(lib_ref->logger, "Llegó otro OpCode",cop);
	}
	return -1;
}




//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
	int* direccionLogica = malloc(sizeof(int));;
	send_mem_alloc(lib_ref->fd_backend, size, lib_ref->pid);

	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);

	if(cop == R_MEM_ALLOC){
		recv_R_mem_alloc(lib_ref->fd_backend, direccionLogica);
	}
	else{
		log_warning(lib_ref->logger, "Código de mensaje incorrecto (mate_memalloc()) - code: %d",cop);
	}


	if(direccionLogica < 0){//TODO: cambiar el 0 por el valor que devuelva memoria en caso de no tener espacio
		return (int) NULL;
	}
	return *direccionLogica;
}


int mate_memfree(mate_instance *lib_ref, mate_pointer addr){
	if(send_mem_free(lib_ref->fd_backend, addr, lib_ref->pid)){

		op_code cop;
		recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);
		int success;
		if(cop == R_MEM_FREE){
			recv_R_mem_free(lib_ref->fd_backend, &success);
		}
		else{
			log_warning(lib_ref->logger, "Código de mensaje incorrecto - mate_memfree()");
			return 0;
		}

		return success;
	}

	return 0;
}


int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int sizeToRead){
	send_mem_read(lib_ref->fd_backend, origin, sizeToRead, lib_ref->pid);

	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);
	int tamanioLeido;
	if(cop == R_MEM_READ){
		recv_R_mem_read(lib_ref->fd_backend, dest, &tamanioLeido);
	}
	else{
		log_warning(lib_ref->logger, "Código de mensaje incorrecto - mate_meread() - code: %d", cop);
		return 0;
	}

	return tamanioLeido;
}


int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){
	send_mem_write(lib_ref->fd_backend, origin, dest, size, lib_ref->pid);

	op_code cop;
	recv(lib_ref->fd_backend, &cop, sizeof(op_code), 0);
	int success;
	if(cop == R_MEM_WRITE){
		recv_R_mem_write(lib_ref->fd_backend, &success);
	}
	else{
		log_warning(lib_ref->logger, "Código de mensaje incorrecto - mate_memwrite()");
		return 0;
	}

	return success;
}


// ------------ Funciones Auxiliares --------------/

mate_sem* buscarSem(mate_instance *lib_ref, char* nombre){

	bool buscar_sem(mate_sem* semaforo){
	  return (strcmp(semaforo->nombre,nombre) == 0);
	};

	return list_find(lib_ref->semaforos, (void*)buscar_sem);
}




