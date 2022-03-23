/*
 * hiloCpu.c
 *
 *  Created on: 22 nov. 2021
 *      Author: utnso
 */


#include "hiloCpu.h"

static dispositivo_io* obtenerDispositivo(char* nombre){

	bool llamado(dispositivo_io* semaforo){
		return !strcmp(semaforo->recurso, nombre);
	}

	dispositivo_io* dispositivo;

	pthread_mutex_lock(&mutexListaIO);
	dispositivo = list_find(LIST_IO, (void*) llamado);
	pthread_mutex_unlock(&mutexListaIO);

	return dispositivo;
}

static t_semaforo* buscarSemaforo(char* nombre){
	bool llamado(t_semaforo* semaforo){
		return !strcmp(semaforo->nombre, nombre);
	}

	t_semaforo* lcdtm;

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	lcdtm = (t_semaforo*)list_find(SEMAFOROS, (void*) llamado);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	return lcdtm;
}


void hiloCpu(){
	// TODO: No se cierra nunca esta conexión >.<
    char* port_memoria = string_itoa(KERNEL_CFG->PUERTO_MEMORIA);

    int FD_MEMORIA_HILO = crear_conexion(
            logger,
            "MEMORIA",
			KERNEL_CFG->IP_MEMORIA,
            port_memoria
        );

    free(port_memoria);

	struct timespec tiempoEntrada;
	struct timespec tiempoSalida;

	// log_info(logger,"hilo creado \n");


	while(1){

		sem_wait(&carpinchoAHiloCpu);
		t_running_thread* carpinchoEjecutando = carpinchoAEjecutar;
		sem_post(&carpinchoEnHiloCpu);

		if( clock_gettime( CLOCK_REALTIME, &(tiempoEntrada)) == -1 ) {
			log_error(logger,"Error en la obtencion del tiempo \n");
		}

		ejecutar(carpinchoEjecutando,FD_MEMORIA_HILO);

		sem_post(&gradoMultiprocesamiento);


		if( clock_gettime( CLOCK_REALTIME, &(tiempoSalida)) == -1 ) {
			log_error(logger,"Error en la obtencion del tiempo \n");
		}

		tiempoEnExec(tiempoEntrada, tiempoSalida, carpinchoEjecutando);
		double entrada = (tiempoEntrada.tv_sec + tiempoEntrada.tv_nsec/ 1000000000.0);
		double salida = (tiempoSalida.tv_sec + tiempoSalida.tv_nsec/ 1000000000.0);

		log_info(logger, "El proceso %d estuvo ejecutando %f",carpinchoEjecutando->c->pid, carpinchoEjecutando->c->rafagaAnterior);

		if(carpinchoEjecutando->c->status == EXIT){
			log_info(logger,"Se eliminó el proceso %d", carpinchoEjecutando->c->pid);
			liberarCarpincho(carpinchoEjecutando);
			sem_post(&gradoMultiprogramacion);
		}

	}
}

void ejecutar(t_running_thread* carpincho, int FD_MEMORIA_HILO){

	log_warning(logger,"El proceso %d se encuentra en ejecución",carpincho->c->pid);
	op_code cop;

	while(carpincho->c->status == EXEC){

		if(carpincho->recienCreado){
			R_send_crear_proceso(carpincho->socketMateLib, carpincho->c->pid);
			carpincho->recienCreado = false;
		}

		if(carpincho->blocked){
			send_R_call_io(carpincho->socketMateLib);
			carpincho->blocked = false;
		}

		if(carpincho->posted){
			send_R_wait_sem(carpincho->socketMateLib);
			carpincho->posted = false;
			log_warning(logger,"carpincho %d posteado",carpincho->c->pid);
		}

		int pid;
		void *origin;   //Validar a que estructura referencia

		if (recv(carpincho->socketMateLib, &cop, sizeof(op_code), 0 ) != sizeof(op_code)) {
			log_info(logger, "DISCONNECT!");
			return;
		}

		switch(cop){


		// SEMAFOROS
		case INICIAR_SEM:;
			log_info(logger,"Proceso %d : INICIAR_SEM recibido", carpincho->c->pid);
			int valorInicial;
			char* nombreSem;
			recv_iniciar_sem(carpincho->socketMateLib, &pid, &valorInicial, &nombreSem);
			log_info(logger,"Proceso %d : solicitud de INICIAR_SEM confirmada, nombreSem: %s  valorInicial: %d",carpincho->c->pid,nombreSem,valorInicial);

			bool successIniciar = crearSemaforo(nombreSem,valorInicial);
			successIniciar? log_info(logger,"El proceso %d creó el semaforo %s con valor inicial de %d", carpincho->c->pid, nombreSem, valorInicial) : log_error(logger,"El proceso %d no creó el semaforo %s ", carpincho->c->pid, nombreSem, valorInicial);

			send_R_iniciar_sem(carpincho->socketMateLib, successIniciar);

			free(nombreSem);

			break;


		case WAIT_SEM:;
			log_info(logger,"Proceso %d : WAIT_SEM recibido", carpincho->c->pid);
			char* nombreSemWait;
			recv_wait_sem(carpincho->socketMateLib, &pid, &nombreSemWait);
			log_info(logger,"Proceso %d : solicitud de WAIT_SEM confirmada, nombreSem: %s ",carpincho->c->pid,nombreSemWait);
			if(waitSem(nombreSemWait, carpincho)){
				send_R_wait_sem(carpincho->socketMateLib);
			}
			else{
				carpincho->c->status = BLOCKED;
				pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
				list_add(LISTA_BLOQUEADOS, carpincho);
				pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
				//TODO: Agregar carpincho a la lista de bloqueados (del sistema)
			}

			free(nombreSemWait);

			break;


		case POST_SEM:;
			log_info(logger,"Proceso %d : POST_SEM recibido", carpincho->c->pid);
			char* nombreSemPost;
			recv_post_sem(carpincho->socketMateLib, &pid, &nombreSemPost);
			log_info(logger,"Proceso %d : solicitud de POST_SEM confirmada, nombreSem: %s ",carpincho->c->pid,nombreSemPost);
			postSem(nombreSemPost, carpincho);

			free(nombreSemPost);

			break;


		case DESTROY_SEM:;
			log_info(logger,"Proceso %d : DESTROY_SEM recibido", carpincho->c->pid);
			char* nombreSemDestroy;
			recv_post_sem(carpincho->socketMateLib, &pid, &nombreSemDestroy);
			log_info(logger,"Proceso %d : solicitud de DESTROY_SEM confirmada, nombreSem: %s  ",carpincho->c->pid,nombreSemDestroy);
			if(!eliminarSemaforo(nombreSemDestroy)){
				log_error(logger,"Proceso %d : Error al eliminar el semaforo %s", carpincho->c->pid,nombreSemDestroy);
				break;
			}
			log_info(logger,"Proceso %d ELIMINÓ el semaforo %s", carpincho->c->pid, nombreSemDestroy);
			//send_R_destroy_sem(carpincho->socketMateLib, successIniciar);

			free(nombreSemDestroy);

			break;


		// IOS
		case CALL_IO:;

			char* recurso;
			char* mensaje;
			recv_call_io(carpincho->socketMateLib, &pid, &recurso, &mensaje);
			log_info(logger,"El carpincho %d solicita una IO al recurso: %s, con el mensaje: %s",carpincho->c->pid, recurso, mensaje);
			dispositivo_io* dispositivo = obtenerDispositivo(recurso);
			carpincho->c->status = BLOCKED;

			pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
			list_add(LISTA_BLOQUEADOS, carpincho);
			pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);

			pthread_mutex_lock(&(dispositivo->mutexLista));
			list_add(dispositivo->listaEspera, carpincho);

			sem_post(dispositivo->semaforo);
			pthread_mutex_unlock(&(dispositivo->mutexLista));

			free(recurso);
			free(mensaje);
			break;


		// MEMORIA
		case MEM_ALLOC:;
			log_info(logger,"Proceso %d : MEM_ALLOC recibido", carpincho->c->pid);
			uint32_t size;
			int direccionLogica;

			recv_mem_alloc(carpincho->socketMateLib,&size,&pid);
			log_info(logger,"Proceso %d : solicitud de MEM_ALLOC confirmada, size: %d",carpincho->c->pid,size);
			// Reenvio el mensaje a memoria

			if(send_mem_alloc(FD_MEMORIA_HILO,size,pid)){
				log_info(logger,"Proceso %d : solicitud de MEM_ALLOC enviada a memoria, size: %d", carpincho->c->pid,size);
			}

			// Recibo la rta de memoria
			recv(FD_MEMORIA_HILO, &cop, sizeof(op_code), 0 );
			recv_R_mem_alloc(FD_MEMORIA_HILO,&direccionLogica);
			log_info(logger,"Proceso %d : respuesta por parte de memoria al MEM_ALLOC, direccionLogica: %d" , carpincho->c->pid ,direccionLogica);
			// Envio lo recibido de memoria al carpincho
			send_R_mem_alloc(carpincho->socketMateLib, direccionLogica);
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_ALLOC enviada a matelib, direccionLogica: %d", carpincho->c->pid ,direccionLogica);
			break;


		case MEM_WRITE:;
			log_info(logger,"Proceso %d : MEM_WRITE recibido", carpincho->c->pid);
			void* data;
			int size_To_Read;
			int32_t dest;
			int rta;

			recv_mem_write(carpincho->socketMateLib,&data, &dest, &size_To_Read, &pid);
			log_info(logger,"Proceso %d : solicitud de MEM_WRITE confirmada, dest: %d  sizeToRead: %d",carpincho->c->pid,size, dest,size_To_Read);
			// Reenvio el mensaje a memoria
			send_mem_write(FD_MEMORIA_HILO,data, dest, size_To_Read, pid);
			log_info(logger,"Proceso %d : solicitud de MEM_WRITE enviada a memoria, dest: %d  sizeToRead: %d",carpincho->c->pid,size, dest,size_To_Read);

			// Recibo la rta de memoria
			recv(FD_MEMORIA_HILO, &cop, sizeof(op_code), 0 ); //opcode
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_WRITE recibido, opcode: %d", carpincho->c->pid, cop);
			recv_R_mem_write(FD_MEMORIA_HILO,&rta);
			log_info(logger,"Proceso %d : respuesta por parte de memoria al MEM_WRITE confirmada, rta: %d" , carpincho->c->pid ,rta);
			// Envio lo recibido de memoria al carpincho
			send_R_mem_write(carpincho->socketMateLib,rta);
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_WRITE enviada a matelib, rta: %d", carpincho->c->pid ,rta);

			// TODO free(data);

			break;


		case MEM_READ:;
			log_info(logger,"Proceso %d : MEM_READ recibido", carpincho->c->pid);
			int32_t origin;
			int sizeToRead;
			void* destRead;

			recv_mem_read(carpincho->socketMateLib,&origin,&sizeToRead,&pid);
			log_info(logger,"Proceso %d : solicitud de MEM_READ confirmada, origin: %d  sizeToRead: %d",carpincho->c->pid,size, origin,sizeToRead);
			// Reenvio el mensaje a memoria
			send_mem_read(FD_MEMORIA_HILO,origin,sizeToRead,pid);
			log_info(logger,"Proceso %d : solicitud de MEM_READ enviada a memoria, origin: %d  sizeToRead: %d",carpincho->c->pid,size, origin,size_To_Read);
			destRead =malloc(sizeToRead);
			// Recibo la rta de memoria
			recv(FD_MEMORIA_HILO, &cop, sizeof(op_code), 0 );
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_READ recibido, opcode: %d", carpincho->c->pid, cop);
			recv_R_mem_read(FD_MEMORIA_HILO,destRead,&sizeToRead);
			log_info(logger,"Proceso %d : respuesta por parte de memoria al MEM_READ confirmada, destRead: %d sizeToRead: %d" , carpincho->c->pid ,destRead,sizeToRead);
			// Envio lo recibido de memoria al carpincho
			send_R_mem_read(carpincho->socketMateLib,destRead,sizeToRead);
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_READ enviada a matelib, destRead: %d sizeToRead: %d" , carpincho->c->pid ,destRead,sizeToRead);
			free(destRead);
			break;


		case MEM_FREE:;
			log_info(logger,"Proceso %d : MEM_FREE recibido", carpincho->c->pid);
			int32_t addr;
			int direcLogi;

			recv_mem_free(carpincho->socketMateLib,&addr,&pid);
			log_info(logger,"Proceso %d : solicitud de MEM_FREE confirmada, addr: %d ",carpincho->c->pid,size, addr);
			// Reenvio el mensaje a memoria
			send_mem_free(FD_MEMORIA_HILO,addr,pid);
			log_info(logger,"Proceso %d : solicitud de MEM_FREE enviada a memoria, addr: %d",carpincho->c->pid,size, addr);
			// Recibo la rta de memoria
			recv(FD_MEMORIA_HILO, &cop, sizeof(op_code), 0 );
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_FREE recibido, opcode: %d", carpincho->c->pid, cop);
			recv_R_mem_free(FD_MEMORIA_HILO,&direcLogi);
			log_info(logger,"Proceso %d : respuesta por parte de memoria al MEM_FREE confirmada, direcLogi: %d " , carpincho->c->pid, direcLogi );
			// Envio lo recibido de memoria al carpincho
			send_R_mem_free(carpincho->socketMateLib,direcLogi);
			log_info(logger,"Proceso %d : respuesta de memoria al MEM_FREE enviada a matelib, direcLogi: %d " , carpincho->c->pid , direcLogi);
			break;


		case FINALIZAR_PROCESO:;
			log_info(logger,"Proceso %d : FINALIZAR_PROCESO recibido", carpincho->c->pid);

			recv_finalizar_proceso(carpincho->socketMateLib, &pid);
			log_info(logger,"Proceso %d : FINALIZAR_PROCESO confirmado", carpincho->c->pid);

			carpincho->c->status = EXIT;

			send_finalizar_proceso(FD_MEMORIA_HILO, pid);
			log_info(logger,"Proceso %d : FINALIZAR_PROCESO enviado a memoria", carpincho->c->pid);

			return;


        default:
			log_error(logger, "Algo anduvo mal en el server del carpincho %d - llego el Op_Code: %d", carpincho->c->pid, cop);
			return;
		}

	}
}


// Actualiza la rafaga anterior del carpincho
void tiempoEnExec(struct timespec tiempoEntrada, struct timespec tiempoSalida, t_running_thread* carpincho){

	double tiempoTotalEjecutando;

	tiempoTotalEjecutando = ( tiempoSalida.tv_sec - tiempoEntrada.tv_sec )
	          + ( tiempoSalida.tv_nsec - tiempoEntrada.tv_nsec )
	            / 1000000000.0;

	carpincho->c->rafagaAnterior = tiempoTotalEjecutando;
}


// SEMAFOROS


bool crearSemaforo(char* nombre, int valorInicial){

	// Al momento de ejecutar un mate_sem_init(), si el semáforo ya se encuentra inicializado,
	// el valor del mismo no debe modificarse.
	if(existeSemaforoLlamado(nombre)){
		return 0;
	}

	t_semaforo* nuevoSemaforo = malloc(sizeof(t_semaforo));

	nuevoSemaforo->nombre = string_duplicate(nombre);
	nuevoSemaforo->valor = valorInicial;
	nuevoSemaforo->usadoPor = list_create();
	nuevoSemaforo->bloqueandoA = list_create();
	pthread_mutex_init(&(nuevoSemaforo->mutexSemaforo ), NULL);// TODO check


	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	list_add(SEMAFOROS,nuevoSemaforo);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	return 1;
}


bool existeSemaforoLlamado(char* nombre){

	return buscarSemaforo(nombre) != NULL;
}


bool eliminarSemaforo(char* nombre){

	bool llamado(t_semaforo* semaforo){
		return !strcmp(semaforo->nombre, nombre);
	}

	if(!existeSemaforoLlamado(nombre)){
		return 0;
	}

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	list_remove_and_destroy_by_condition(SEMAFOROS,(void*) llamado, (void*)liberarSemaforo );
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	return 1;
}


bool waitSem(char* nombre, t_running_thread* carpincho){

	bool llamado(t_semaforo* semaforo){
		return !strcmp(semaforo->nombre, nombre);
	}


	if(!existeSemaforoLlamado(nombre)){
		return 1;
	}

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	t_semaforo* semaforo = list_find(SEMAFOROS, (void*) llamado);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	pthread_mutex_lock(&(semaforo->mutexSemaforo));// TODO check
	if(semaforo->valor > 0){
		log_info(logger,"El proceso %d toma el recurso wait", carpincho->c->pid);
		semaforo->valor-=1;
		list_add(carpincho->semaforosTomados, semaforo);
		list_add(semaforo->usadoPor, carpincho);
		pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
		return true;
	}
	else{
		log_info(logger,"El proceso %d entra a la lista de espera para los waits", carpincho->c->pid);
		carpincho->bloqueadoPor = semaforo;
		list_add(semaforo->bloqueandoA, carpincho);
		pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
		return false;
	}



	return 1;
}


bool postSem(char* nombre, t_running_thread* carpincho){

	bool llamado(t_semaforo* semaforo){
		return !strcmp(nombre, semaforo->nombre);
	}


	if(!existeSemaforoLlamado(nombre)){
			return 0;
	}

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	t_semaforo* semaforo = list_find(SEMAFOROS, (void*) llamado);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	pthread_mutex_lock(&(semaforo->mutexSemaforo));

	bool sonElMismoCarpincho(t_running_thread* thread){
		return (carpincho->c->pid == thread->c->pid);
	}

	// log_error(logger,"(postSem()), %s, size: %d, valor: %d", semaforo->nombre, list_size(semaforo->usadoPor), semaforo->valor);
	list_remove_by_condition(semaforo->usadoPor, (void*) sonElMismoCarpincho);
	list_remove_by_condition(carpincho->semaforosTomados, (void*) llamado);

	/*
	if(list_size(semaforo->usadoPor)){
		t_running_thread* aaa = (t_running_thread*)list_remove(semaforo->usadoPor, 0);
		log_error(logger,"PID: %d", aaa->c->pid);
	}
	*/


	if(list_is_empty(semaforo->bloqueandoA)){
		semaforo->valor+=1;
		pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
		// log_error(logger,"Lista de carpinchos blockeados vacía - Sem:  %s - valor: %d", semaforo->nombre, semaforo->valor);
		return 1;
	}

	t_running_thread* carpinchoALiberar = list_remove(semaforo->bloqueandoA, 0);

	list_add(semaforo->usadoPor, carpinchoALiberar);
	pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
	carpinchoALiberar->bloqueadoPor = NULL;
	list_add(carpinchoALiberar->semaforosTomados, semaforo);


	if(carpinchoALiberar->c->status == BLOCKED){
		bool mismoID(t_running_thread* carp){
			return (carpinchoALiberar->c->pid == carp->c->pid);
		}

		pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
		list_remove_by_condition(LISTA_BLOQUEADOS, (void*) mismoID);
		pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);


		carpinchoALiberar->c->status = READY;
		add_lista_ready(carpinchoALiberar);

		sem_post(&carpinchosEnReady);
	}
	else if(carpinchoALiberar->c->status == SUSPENDED){

		list_add(LISTA_SUSPENDIDOS_READY, carpinchoALiberar);
		sem_post(&carpinchosEnNew);
	}


	log_info(logger,"Proceso %d : toma el recurso wait del semaforo %s", carpinchoALiberar->c->pid, semaforo->nombre);
	//send_R_wait_sem(carpinchoALiberar->socketMateLib);
	carpinchoALiberar->posted = true;

	return 1;
}


void liberarCarpincho(t_running_thread* carpinchoEjecutando){

	bool sonElMismoCarpincho(t_running_thread* thread){
				return (carpinchoEjecutando->c->pid == thread->c->pid);
			}

	pthread_mutex_destroy(&(carpinchoEjecutando->c->mutexCarpincho));



	if(carpinchoEjecutando->c->status == BLOCKED){

		pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
		list_remove_by_condition(LISTA_BLOQUEADOS, (void*) sonElMismoCarpincho);
		pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);

		pthread_mutex_lock(&(carpinchoEjecutando->bloqueadoPor->mutexSemaforo));
		list_remove_by_condition(carpinchoEjecutando->bloqueadoPor->bloqueandoA, (void*) sonElMismoCarpincho);
		pthread_mutex_unlock(&(carpinchoEjecutando->bloqueadoPor->mutexSemaforo));

	}



	int tamanioLista = list_size(carpinchoEjecutando->semaforosTomados);
	for(int i=0;i<tamanioLista;i++){
		t_semaforo* sem = list_get(carpinchoEjecutando->semaforosTomados,0);
		postSem(sem->nombre,carpinchoEjecutando);
	}

//	pthread_mutex_lock(&MUTEX_SEMAFOROS);
//	t_semaforo* semaforo = list_find(SEMAFOROS, (void*) llamado);
//	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	list_destroy(carpinchoEjecutando->semaforosTomados);
	free(carpinchoEjecutando->c);
	sem_destroy(&(carpinchoEjecutando->sem_pause));
	liberar_conexion(&(carpinchoEjecutando->socketMateLib));
	free(carpinchoEjecutando);

}


void liberarSemaforo(t_semaforo* semaforoAEliminar){ //mate_sem_destroy

	bool llamado(t_semaforo* semaforo){
			return !strcmp(semaforoAEliminar->nombre, semaforo->nombre);
		}

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	list_remove_by_condition(SEMAFOROS,(void*) llamado);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);



	liberarSemaforoDeCarpinchos(semaforoAEliminar);

	list_destroy(semaforoAEliminar->usadoPor);
	list_destroy(semaforoAEliminar->bloqueandoA);

	pthread_mutex_destroy(&(semaforoAEliminar->mutexSemaforo));

	free(semaforoAEliminar->nombre);
	free(semaforoAEliminar);

}

void liberarSemaforoDeCarpinchos(t_semaforo* semaforoAEliminar){

	bool llamado(t_semaforo* semaforo){
				return !strcmp(semaforoAEliminar->nombre, semaforo->nombre);
			}

	while(!list_is_empty(semaforoAEliminar->usadoPor)){
		t_running_thread* unCarpincho = list_remove(semaforoAEliminar->usadoPor,0);
		list_remove_by_condition(unCarpincho->semaforosTomados, (void*) llamado);
	}
	while(!list_is_empty(semaforoAEliminar->bloqueandoA)){
		t_running_thread* unCarpincho = list_remove(semaforoAEliminar->bloqueandoA,0);
		unCarpincho->bloqueadoPor = NULL;
	}
}








// DEADLOCK


void rutinaDeadlock(){
	// Prayge

	while(1){

		usleep( KERNEL_CFG->TIEMPO_DEADLOCK * 1000);
		t_list* listaCarpinchosEnDeadlock = NULL;

		if(deteccionDeadlock(&listaCarpinchosEnDeadlock)){
			log_error(logger,"Se detectó Deadlock en el sistema");
			char* aux = idsEnDeadlock(listaCarpinchosEnDeadlock);
			log_error(logger,"Se detectó deadlock entre los carpinchos con id:  %s ", aux);
			free(aux);

			log_warning(logger,"Corriendo recuperación de deadlocks");
			recuperacionDeadlock(listaCarpinchosEnDeadlock);
		}else{
			log_warning(logger,"No se detectó deadlock");
		}

		if(listaCarpinchosEnDeadlock != NULL){
			list_destroy(listaCarpinchosEnDeadlock);
		}
	}

}



bool deteccionDeadlock(t_list** carpinchosBloqueadosFinal){
	log_warning(logger,"Corriendo deteccion de deadlocks");
	pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
	bool bloqueadoPorSemaforo(t_running_thread* c){
		return c->bloqueadoPor != NULL;
	}

    t_list* listaBloqueadosPorSemaforos;
    listaBloqueadosPorSemaforos = list_filter(LISTA_BLOQUEADOS, (void*) bloqueadoPorSemaforo);


    //log_warning(logger,"El size carpinchos bloqueados por semaforo: %d", list_size(listaBloqueadosPorSemaforos));
    //t_running_thread* aux32 = list_get(listaBloqueadosPorSemaforos, 0);
   // log_error(logger, "pid :%d", aux32->c->pid);
    while(!list_is_empty(listaBloqueadosPorSemaforos)){
    	t_list* listaIterable = list_create();
    	t_running_thread* carpincho;
    	carpincho = list_remove(listaBloqueadosPorSemaforos,0);
    	t_list* carpinchosBloqueados = buscarDeadlockSiguiente(carpincho,listaIterable);
    	if(carpinchosBloqueados!= NULL){

    		(*carpinchosBloqueadosFinal) = carpinchosBloqueados;
    		list_destroy(listaIterable);
    		list_destroy(listaBloqueadosPorSemaforos);
    	    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
    		return 1;
    	}
    	list_destroy(listaIterable);
    }
    list_destroy(listaBloqueadosPorSemaforos);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);

	return 0;
}

t_list* buscarDeadlockSiguiente(t_running_thread* carpincho, t_list* listaCarpinchosBloqueados){
	bool estaCarpincho(t_running_thread* c){
		return c->c->pid == carpincho->c->pid;
	}
	if(list_any_satisfy(listaCarpinchosBloqueados, (void*) estaCarpincho)){
		return list_duplicate(listaCarpinchosBloqueados);
	}
	if(carpincho->bloqueadoPor == NULL){
		return NULL;
	}
	list_add(listaCarpinchosBloqueados, carpincho);
	t_semaforo* semaforoBloqueando;
	semaforoBloqueando = carpincho->bloqueadoPor;
	pthread_mutex_lock(&(semaforoBloqueando->mutexSemaforo));
	t_list* duplicadoListaSemaforosUsadosPorSemaforo = list_duplicate(semaforoBloqueando->usadoPor);
	pthread_mutex_unlock(&(semaforoBloqueando->mutexSemaforo));

	while(!list_is_empty(duplicadoListaSemaforosUsadosPorSemaforo)){
    	t_running_thread* carpincho2;
    	carpincho2 = list_remove(duplicadoListaSemaforosUsadosPorSemaforo,0);

    	t_list* aux = list_duplicate(listaCarpinchosBloqueados);

    	t_list* aux2 = buscarDeadlockSiguiente(carpincho2,aux);

    	list_destroy(aux);

    	if(aux2 != NULL){
    		list_destroy(duplicadoListaSemaforosUsadosPorSemaforo);
    		return aux2;
    	}
    }
	list_destroy(duplicadoListaSemaforosUsadosPorSemaforo);
	return NULL;

}



void recuperacionDeadlock(t_list* carpinchosBloqueadosFinal){

	// Obtener carpinchos en Deadlock? Ponerlos en una lista?

	int i = list_size(carpinchosBloqueadosFinal);
	int index;
	int pid = 0;
	t_running_thread* carpinchoAux;


	for(int a = 0; a < i; a++){
		carpinchoAux = list_get(carpinchosBloqueadosFinal,a);
		if(carpinchoAux->c->pid > pid){
			pid = carpinchoAux->c->pid;
			index = a;
		}
	}

	t_running_thread* carpinchoAux2 = list_remove(carpinchosBloqueadosFinal,index);
	log_error(logger,"Se eliminó el carpincho con id : %d para intentar recuperar el deadlock", carpinchoAux2->c->pid);
	liberarCarpinchoDeadlock(carpinchoAux2);

	// Se van eliminando los carpinchos de a 1, de ID mayor a menor, hasta que se arregle el deadlock
	// elimino el de mayor ID

	t_list* aux = NULL;

	while(deteccionDeadlock(&aux)){

		int i = list_size(carpinchosBloqueadosFinal);
		int index;
		int pid = -1;
		t_running_thread* carpinchoAux;

		for(int a = 0; a < i; a++){
			carpinchoAux = list_get(carpinchosBloqueadosFinal,a);
			if(carpinchoAux->c->pid > pid){

				pid = carpinchoAux->c->pid;
				index = a;
			}
		}


		liberarCarpinchoDeadlock(list_remove(carpinchosBloqueadosFinal,index));

		list_destroy(aux);
		aux = NULL;
	}

	log_warning(logger,"Se recuperó del deadlock");
}



char* idsEnDeadlock(t_list* cEnDeadlock){

    int funMap(t_running_thread* carpincho){
		return carpincho->c->pid;
	}
    t_list* aux2 = NULL;
    aux2 = cEnDeadlock = list_map(cEnDeadlock, (void*) funMap);
	char* aux = string_new();
	string_append(&aux, "");
	void iterador(int a){
		//log_warning(logger,"id: %d",a);
		string_append_with_format(&aux,"%d ",a);
	}
	list_iterate(cEnDeadlock, (void*) iterador);

	list_destroy(aux2);
	return aux;
}






void liberarCarpinchoDeadlock(t_running_thread* carpinchoEjecutando){

	bool sonElMismoCarpincho(t_running_thread* thread){
				return (carpinchoEjecutando->c->pid == thread->c->pid);
			}

	pthread_mutex_destroy(&(carpinchoEjecutando->c->mutexCarpincho));


	if(carpinchoEjecutando->c->status == BLOCKED){

		list_remove_by_condition(LISTA_BLOQUEADOS, (void*) sonElMismoCarpincho);

		pthread_mutex_lock(&(carpinchoEjecutando->bloqueadoPor->mutexSemaforo));
		list_remove_by_condition(carpinchoEjecutando->bloqueadoPor->bloqueandoA, (void*) sonElMismoCarpincho);
		pthread_mutex_unlock(&(carpinchoEjecutando->bloqueadoPor->mutexSemaforo));
	}


	int tamanioLista = list_size(carpinchoEjecutando->semaforosTomados);
	for(int i=0;i<tamanioLista;i++){
		t_semaforo* sem = list_get(carpinchoEjecutando->semaforosTomados,i);
		postSemDeadlock(sem->nombre,carpinchoEjecutando);
	}

//	pthread_mutex_lock(&MUTEX_SEMAFOROS);
//	t_semaforo* semaforo = list_find(SEMAFOROS, (void*) llamado);
//	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	list_destroy(carpinchoEjecutando->semaforosTomados);
	free(carpinchoEjecutando->c);
	sem_destroy(&(carpinchoEjecutando->sem_pause));
	send_R_wait_sem_eliminado(carpinchoEjecutando->socketMateLib);
	liberar_conexion(&(carpinchoEjecutando->socketMateLib));
	free(carpinchoEjecutando);

}





bool postSemDeadlock(char* nombre, t_running_thread* carpincho){

	bool llamado(t_semaforo* semaforo){
		return !strcmp(nombre, semaforo->nombre);
	}


	if(!existeSemaforoLlamado(nombre)){
			return 0;
	}

	pthread_mutex_lock(&MUTEX_SEMAFOROS);
	t_semaforo* semaforo = list_find(SEMAFOROS, (void*) llamado);
	pthread_mutex_unlock(&MUTEX_SEMAFOROS);

	// TODO check

	pthread_mutex_lock(&(semaforo->mutexSemaforo));

	bool sonElMismoCarpincho(t_running_thread* thread){
		return (carpincho->c->pid == thread->c->pid);
	}

	list_remove_by_condition(semaforo->usadoPor, (void*) sonElMismoCarpincho);
	list_remove_by_condition(carpincho->semaforosTomados, (void*) llamado);

	if(list_is_empty(semaforo->bloqueandoA)){
		semaforo->valor+=1;
		pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
		return 1;
	}

	t_running_thread* carpinchoALiberar = list_remove(semaforo->bloqueandoA, 0);

	list_add(semaforo->usadoPor, carpinchoALiberar);
	pthread_mutex_unlock(&(semaforo->mutexSemaforo));// TODO check
	carpinchoALiberar->bloqueadoPor = NULL;
	list_add(carpinchoALiberar->semaforosTomados, semaforo);


	if(carpinchoALiberar->c->status == BLOCKED){
		bool mismoID(t_running_thread* carp){
			return (carpinchoALiberar->c->pid == carp->c->pid);
		}


		list_remove_by_condition(LISTA_BLOQUEADOS, (void*) mismoID);


		carpinchoALiberar->c->status = READY;
		add_lista_ready(carpinchoALiberar);

		sem_post(&carpinchosEnReady);
	}
	else if(carpinchoALiberar->c->status == SUSPENDED){

		list_add(LISTA_SUSPENDIDOS_READY, carpinchoALiberar);
		sem_post(&carpinchosEnNew);
	}
	carpinchoALiberar->posted = true;

	log_info(logger,"Proceso %d : toma el recurso wait del semaforo %s", carpinchoALiberar->c->pid, semaforo->nombre);


	return 1;
}











