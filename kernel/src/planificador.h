#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include <time.h>



#include "config.h"
#include "sharedlib.h"
#include "protocol.h"
#include "init.h"
#include "planificador.h"
// Estructuras Carpinchos

typedef enum {
    NEW                 = 'N',
    READY               = 'R',
    EXEC                = 'X',
    BLOCKED             = 'B',
    SUSPENDED           = 'S',
	READYSUSPENDED      = 'D',
    EXIT                = 'E',
} t_status;

typedef struct {
    uint32_t pid;
    t_status status;
    double rafagaAnterior; //Tenemos que saber cuanto tardo para poder estimar
    pthread_mutex_t mutexCarpincho;
    double estimadoAnterior;
} t_carpincho;

typedef struct kernel_sem
{
	char* nombre;
	int valor;
	t_list* usadoPor;
	t_list* bloqueandoA;
	pthread_mutex_t mutexSemaforo;

} t_semaforo;


t_list* SEMAFOROS;


typedef struct {
	int socketMateLib;
    t_carpincho* c;
    sem_t sem_pause;
    bool blocked;
    bool posted;
    bool recienCreado;
    struct timespec arriveTime;
    t_list* semaforosTomados;
    t_semaforo* bloqueadoPor;
} t_running_thread;


extern sem_t carpinchosEnReady ;
extern sem_t gradoMultiprocesamiento;
extern sem_t carpinchoAHiloCpu;
extern sem_t carpinchoEnHiloCpu;

// Semaforos colas

/*
extern pthread_mutex_t MUTEX_COLA_NEW;
extern pthread_mutex_t MUTEX_COLA_EXIT;
extern pthread_mutex_t MUTEX_COLA_BLOQUEADOS;
extern pthread_mutex_t MUTEX_LISTA_READY;
extern pthread_mutex_t MUTEX_LISTA_SUSPENDIDOS;
extern pthread_mutex_t MUTEX_SEMAFOROS;*/

sem_t CARPINCHOS_EN_COLA;

void iniciarSemaforos();
sem_t carpinchosEnNew;


void iniciarColas();

// Cola NEW

t_queue* COLA_NEW;

void push_cola_new(t_running_thread*);
t_running_thread* pop_cola_new();
void iterar_cola_new(void (*f)(void*));
uint32_t largo_cola_new();
t_running_thread* buscar_cola_new(uint32_t tid);
void* remover_cola_new(uint32_t tid);

// Bloqueados E/S

t_list* LISTA_BLOQUEADOS;

void agregar_lista_bloqueados(t_running_thread* thread);
t_running_thread* quitar_lista_bloqueados();
void iterar_lista_bloqueados(void (*f)(void*));
t_running_thread* buscar_lista_bloqueados(uint32_t tid);
uint32_t largo_lista_bloqueados();

// Cola exit

t_list* LISTA_EXIT;

void agregar_lista_exit(void*);
void iterar_lista_exit(void (*f)(void*));
void* remover_lista_exit(uint32_t tid);
t_carpincho* obtener_lista_exit(uint32_t tid);

// Cola Suspendido

t_list* LISTA_SUSPENDIDOS_READY;
//TODO HAY QUE PENSARLA UN POCO

// Lista hilos //Esta lista de hilos siento que va pero no termino de entender su funcion al 100%, revisar en otro momento (preferiblemente antes de las 2am)

t_list* LISTA_READY;

void iniciar_mutex();

void add_lista_ready(void*);
void add_lista_ready_sin_mutex(void* t);
void* remove_by_condition_lista_ready(bool (*f)(void*));
uint32_t largo_lista_ready();
t_running_thread* buscar_lista_ready(uint32_t tid);
void* remover_lista_ready(uint32_t);
void iterar_lista_ready(void (*f)(void*));

// Funciones Carpinchos

void free_t_carpincho(void* carpincho);
void cambiar_estado(t_carpincho* c, t_status nuevo);

bool filter_by_pid(void* t_c);
bool filter_t_running_thread_by_pid(void* item);


#endif /* PLANIFICADOR_H_ */
