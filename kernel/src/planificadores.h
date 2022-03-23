/*
 * planificadores.h
 *
 *  Created on: 21 nov. 2021
 *      Author: utnso
 */

#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

#include <semaphore.h>


#include "config.h"
#include "sharedlib.h"
#include "protocol.h"
#include "init.h"
#include "planificador.h"

t_list* LISTA_INDICES_HRNN;
t_list* LISTA_INDICES_SJF;
extern t_list* LISTA_READY;

typedef struct {
	char* recurso;
	sem_t* semaforo;
	int duracion;
	int index;
	t_list* listaEspera;
	pthread_mutex_t mutexLista;
} dispositivo_io;

t_list* LIST_IO;
extern t_list* LISTA_SUSPENDIDOS_READY;

extern pthread_mutex_t MUTEX_LISTA_READY;
extern pthread_mutex_t MUTEX_COLA_NEW;
extern pthread_mutex_t MUTEX_LISTA_BLOQUEADOS;
extern pthread_mutex_t MUTEX_LISTA_SUSPENDIDOS_READY;

extern sem_t carpinchosEnReady ;
extern sem_t gradoMultiprocesamiento;
extern sem_t carpinchoAHiloCpu;
extern sem_t carpinchoEnHiloCpu;

sem_t carpinchoAHiloCpu;
sem_t carpinchoEnHiloCpu;

sem_t carpinchosEnReady;
sem_t carpinchosEnNew;

sem_t gradoMultiprogramacion;
sem_t gradoMultiprocesamiento;

sem_t posibleSuspencion;

t_running_thread* carpinchoAEjecutar;

void planificadorCortoPlazo();
void planificadorMedianoPlazo();
void planificadorLargoPlazo();
t_running_thread* obtenerSiguiente();

bool cumpleCondiciones();
void suspenderProceso();

double jobTime(t_running_thread* carpincho);
double responseRatio(t_running_thread* carpincho, struct timespec stop);


#endif /* PLANIFICADORES_H_ */
