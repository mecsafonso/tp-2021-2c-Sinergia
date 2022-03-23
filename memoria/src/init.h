#ifndef INIT_MEMORIA_H_
#define INIT_MEMORIA_H_

#include "config.h"
#include "sharedlib.h"
#include "log.h"
#include "protocol.h"
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>



#define DIR_CONFIG "/home/utnso/tp-2021-2c-Sinergia/memoria/memoria.config"
#define DUMP_PATH_TLB "dumpFile.txt"
#define ERROR_MEMORIA -1
#define ERROR_SWAMP_U_OTRO -2


extern pthread_mutex_t mutex_TLB;



/*	-------------------------------------------------------------------
						ESTRUCTURAS
	-------------------------------------------------------------------*/
typedef struct{
	int idProcess;
	t_list* pages;
	pthread_mutex_t mutex_allocs;
}t_pageTable;

typedef struct {
	int idProcess;
	int idPage;
	int memory_frame_number;
	int swap_frame_number;
	int frame_asig_FIJA;
	bool modified;
}t_page;

typedef struct {
	int32_t prevAlloc;
	int32_t nextAlloc;
	int8_t isFree;
}heapMetadata;

typedef struct {
	int base;
	int size;
}t_alloc;

typedef struct {
	int idProcess;
	int page;
	int frame_number;
}t_tlb;

typedef struct {
	int id_process;
	int hit;
	int miss;
}t_metrics_tlb;

typedef struct{
	int id_process;
	int page;
	int frame;
}t_frame;


/*	-------------------------------------------------------------------
						VARIABLES GLOBALES
	-------------------------------------------------------------------*/



int FD_SWAMP;
int VGID;
void* MEMORY;
int MEMORY_PARTITIONS;
int SIZE_HEAPMETADATA;
t_bitarray* MEMORY_BITMAP;


t_config_memoria* MEMORIA_CFG;
t_log* LOGGER;


/*	-------------------------------------------------------------------
						LISTAS
	-------------------------------------------------------------------*/
t_list* TGP; //TABLA GENERAL DE PAGINAS
t_list* MEMORY_PAGES;
t_list* TLB;
t_list* BITMAPS_PROCESS_FIJA;
t_list* TLB_METRICS;



/*	-------------------------------------------------------------------
						CLOCK MODIFICADO
	-------------------------------------------------------------------*/
typedef struct {
	int use;
	t_page* page;
} t_element;

typedef struct {
	int id_process;
	int clock;
	t_element* pages;
} t_pages_CLOCK_fija;

t_element* POINTER_DINAMICA;
t_list* POINTER_FIJA;
int clock_index;


uint8_t cargar_configuracion(t_config_memoria* config);
uint8_t generar_conexiones(int* fd_swamp, t_config_memoria* cfg);
uint8_t cargar_memoria(int* fd_swamp, t_config_memoria* cfg, t_log* logger);


void cerrar_programa(t_log* logger, t_config_memoria* cfg);
void dump_TLB(int sig);
void clean_TLB(int sig);
bool reset_TLB(t_config_memoria* cfg);
void print_metrics(int sig);

void print_TLB();
void print_memoria();



#endif
