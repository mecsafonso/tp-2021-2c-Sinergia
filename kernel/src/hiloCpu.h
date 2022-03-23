/*
 * hiloCpu.h
 *
 *  Created on: 22 nov. 2021
 *      Author: utnso
 */

#ifndef HILOCPU_H_
#define HILOCPU_H_

#include "planificadores.h"

void hiloCpu();
void ejecutar(t_running_thread* carpincho, int FD_MEMORIA_HILO);

extern sem_t carpinchoAHiloCpu;
extern sem_t carpinchoEnHiloCpu;
extern sem_t gradoMultiprocesamiento;
extern sem_t gradoMultiprogramacion;
extern pthread_mutex_t MUTEX_COLA_BLOQUEADOS;
extern pthread_mutex_t MUTEX_SEMAFOROS;
extern pthread_mutex_t mutexListaIO;
extern pthread_mutex_t MUTEX_LISTA_BLOQUEADOS;

extern t_list* LIST_IO;
extern t_list* LISTA_BLOQUEADOS;
void tiempoEnExec(struct timespec tiempoEntrada, struct timespec tiempoSAlida, t_running_thread* carpincho);

bool crearSemaforo(char* nombre, int valorInicial);
bool eliminarSemaforo(char* nombre);
bool waitSem(char* nombre, t_running_thread* carpincho);
bool postSem(char* nombre, t_running_thread* carpincho);
bool existeSemaforoLlamado(char* nombre);
void liberarCarpincho(t_running_thread* carpinchoEjecutando);
void liberarSemaforo(t_semaforo* semaforoAEliminar);
void liberarSemaforoDeCarpinchos(t_semaforo* semaforoAEliminar);

t_list* buscarDeadlockSiguiente(t_running_thread* carpincho, t_list* listaSemaforosBloqueados);

void rutinaDeadlock();
bool deteccionDeadlock(t_list** carpinchosBloqueadosFinal);
void recuperacionDeadlock(t_list* carpinchosBloqueadosFinal);
char* idsEnDeadlock(t_list* cEnDeadlock);
void liberarCarpinchoDeadlock(t_running_thread* carpinchoEjecutando);
bool postSemDeadlock(char* nombre, t_running_thread* carpincho);

#endif /* HILOCPU_H_ */
