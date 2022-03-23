/*
 * planificadores.c
 *
 *  Created on: 21 nov. 2021
 *      Author: utnso
 */

#include "planificadores.h"


// CORTO PLAZO
void planificadorCortoPlazo(){


	while(1){
		//Hay hilos disponibles
		sem_wait(&gradoMultiprocesamiento);
		//Hay carpinchos en ready
		sem_wait(&carpinchosEnReady);
		log_info(logger, "Un proceso ingresó a READY");
		// Es una variablo global
		carpinchoAEjecutar = obtenerSiguiente();
		sem_post(&carpinchoAHiloCpu);
		sem_wait(&carpinchoEnHiloCpu);
	}

}

t_running_thread* obtenerSiguiente(){


	t_running_thread* carpinchoSeleccionado;
	double hrr;
	double sjt;
	int indiceLista = 0;
	int indiceCarpinchoSeleccionado;

	struct timespec stop;

	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
		log_error(logger,"Error en la obtencion del tiempo \n");
	}
	pthread_mutex_lock(&MUTEX_LISTA_READY);
	t_list_iterator* iterator = list_iterator_create(LISTA_READY);

	if((strcmp(KERNEL_CFG->ALGORITMO_PLANIFICACION,"HRRN")) == 0){

		// Planificación con HRNN
		hrr = -1;

		while(list_iterator_has_next(iterator)){

			t_running_thread* carpincho = list_iterator_next(iterator);
			double rr = responseRatio(carpincho, stop);
			log_error(logger,"ERRE ERRE: %f - PID: %d", rr, carpincho->c->pid);
			if(rr > hrr){
				hrr = rr;
		        indiceCarpinchoSeleccionado = indiceLista;
		    }

			indiceLista++;
		}
	}
	else{

		// Planificación con SJF
		if(list_iterator_has_next(iterator)){
			sjt = jobTime(list_iterator_next(iterator));
			indiceCarpinchoSeleccionado = indiceLista;
			indiceLista++;

		}

		while(list_iterator_has_next(iterator)){

			double jt = jobTime(list_iterator_next(iterator));
			//log_error(logger,"SJF calculado: %f - SJF anterior: %f", jt, sjt);
			if(jt < sjt){
				sjt = jt;
				indiceCarpinchoSeleccionado = indiceLista;
			}

			indiceLista++;
		}


	}
	list_iterator_destroy(iterator);
	carpinchoSeleccionado = list_remove(LISTA_READY, indiceCarpinchoSeleccionado);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
	carpinchoSeleccionado->c->estimadoAnterior = jobTime(carpinchoSeleccionado);

	//pthread_mutex_lock(&(carpinchoSeleccionado->c->mutexCarpincho));
	carpinchoSeleccionado->c->status=EXEC; // VALGRIND MARCA RACE CONDITION
	//pthread_mutex_unlock(&(carpinchoSeleccionado->c->mutexCarpincho));   //TODO: Deberian ser mutex estos sem? o por concurrencia nunca deberia darse que varios hilos toquen el estado de un c?

	return carpinchoSeleccionado;
}


double responseRatio(t_running_thread* carpincho, struct timespec stop){
	double tiempoEnWait;

	tiempoEnWait = ( stop.tv_sec - carpincho->arriveTime.tv_sec )
	          + ( stop.tv_nsec - carpincho->arriveTime.tv_nsec )
	            / 1000000000.0;

	//log_info(logger, "Tiempo en espera: %f - JobTime: %f", tiempoEnWait, jobTime(carpincho));
	return (tiempoEnWait + jobTime(carpincho))/jobTime(carpincho);
	//return 1 + ( tiempoEnWait / jobTime(carpincho));
}

double jobTime(t_running_thread* carpincho){

	if(carpincho->c->estimadoAnterior == 0){
		return KERNEL_CFG->ESTIMACION_INICIAL/1000;
	}
	return KERNEL_CFG->ALFA * carpincho->c->rafagaAnterior + (1 - KERNEL_CFG->ALFA ) * carpincho->c->estimadoAnterior;
}


// MEDIANO PLAZO
void planificadorMedianoPlazo(){
	sem_init(&posibleSuspencion, 0, 0);


	while(1){

		//TODO: WAIT QUE SE ACTIVA CUANDO ALGUNAS DE LAS CONDICIONES SE CUMPLE

		sem_wait(&posibleSuspencion);



		pthread_mutex_lock(&MUTEX_COLA_NEW);
		pthread_mutex_lock(&MUTEX_LISTA_READY);
		pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);


		if(cumpleCondiciones()){
			suspenderProceso();
			sem_post(&gradoMultiprogramacion);
		}


		pthread_mutex_unlock(&MUTEX_COLA_NEW);
		pthread_mutex_unlock(&MUTEX_LISTA_READY);
		pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);

	}

}

bool cumpleCondiciones(){

	if(queue_is_empty(COLA_NEW)){
		return false;
	}

	if(!list_is_empty(LISTA_READY)){
		return false;
	}

	if(list_is_empty(LISTA_BLOQUEADOS)){
		return false;
	}

	return true;

}


void suspenderProceso(){
	int index = list_size(LISTA_BLOQUEADOS) - 1;
	t_running_thread* carpincho  = list_remove(LISTA_BLOQUEADOS, index);

	carpincho->c->status = SUSPENDED;

	char* port_memoria = string_itoa(KERNEL_CFG->PUERTO_MEMORIA);

	int FD_MEMORIA_SUSPENCION = crear_conexion(
			logger,
			"MEMORIA",
			KERNEL_CFG->IP_MEMORIA,
			port_memoria
		);

	free(port_memoria);

	send_suspender_proceso(FD_MEMORIA_SUSPENCION, carpincho->c->pid);

	int success;
	recv_R_suspender_proceso(FD_MEMORIA_SUSPENCION, &success);

	if(success){
		log_info(logger,"Proceso %d suspendido correctamente", carpincho->c->pid);
	}
	else{
		log_error(logger,"Error al suspender el proceso %d", carpincho->c->pid);
	}

	close(FD_MEMORIA_SUSPENCION);
}



// LARGO PLAZO
void planificadorLargoPlazo(){

	sem_init(&gradoMultiprogramacion,0,KERNEL_CFG->GRADO_MULTIPROGRAMACION);
	//sem_init(&carpinchosEnReady,0,0);

	while(1){

		sem_wait(&carpinchosEnNew);
		sem_wait(&gradoMultiprogramacion);


		pthread_mutex_lock(&MUTEX_COLA_NEW);
		pthread_mutex_lock(&MUTEX_LISTA_READY);
		pthread_mutex_lock(&MUTEX_LISTA_SUSPENDIDOS_READY);

		t_running_thread* carpincho;

		if(list_is_empty(LISTA_SUSPENDIDOS_READY)){
			carpincho = queue_pop(COLA_NEW);
		}else{
			carpincho = list_remove(LISTA_SUSPENDIDOS_READY, 0); // tienen prioridad los que entran a READY luego de ser desmapeados de memoria
		}


		carpincho->c->status=READY;
		log_info(logger, "Envio carpincho a ready");
		add_lista_ready_sin_mutex(carpincho);

		pthread_mutex_unlock(&MUTEX_COLA_NEW);
		pthread_mutex_unlock(&MUTEX_LISTA_READY);
		pthread_mutex_unlock(&MUTEX_LISTA_SUSPENDIDOS_READY);

		sem_post(&carpinchosEnReady);
	}
}
