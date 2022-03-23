/*
 * comunicacion.h
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include <semaphore.h>

#include "config.h"
#include "sharedlib.h"
#include "protocol.h"
#include "init.h"
#include "planificador.h"


extern pthread_mutex_t mutexProcesosTotales;
extern sem_t carpinchosEnNew;
extern sem_t gradoMultiprogramacion;
extern sem_t posibleSuspencion;

int server_escuchar(t_log* logger, char* server_name, int server_socket);

bool aprobadoPorKernel();
int cantidadProcesosEnSistema();

bool aprobadoPorMemoria(int pid);

#endif /* COMUNICACION_H_ */
