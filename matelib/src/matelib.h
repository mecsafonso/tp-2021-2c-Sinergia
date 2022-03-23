/*
 * matelib.h
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#ifndef MATELIB_H_
#define MATELIB_H_

#include <stdint.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/collections/list.h>

#define DIR_CONFIG "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config"
#define DIR_LOG "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.log"



//-------------------Type Definitions----------------------/
typedef struct mate_instance
{
    void *group_info;
    int pid;
    int fd_backend;
    t_log* logger;
    t_list* semaforos;
} mate_instance;

typedef struct mate_sem
{
	char* nombre;
	int valor;
	sem_t sem;
} mate_sem;



typedef char *mate_io_resource;

typedef char *mate_sem_name;

typedef int32_t mate_pointer;

// TODO: Docstrings

//------------------General Functions---------------------/
int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/
int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);


//-------------- Funciones Auxiliares -------------------/

mate_sem* buscarSem(mate_instance *lib_ref, char* nombre);

#endif /* MATELIB_H_ */
