#ifndef COMUNICACION_MEMORIA_H_
#define COMUNICACION_MEMORIA_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>

#include "sharedlib.h"
#include "protocol.h"
#include "init.h"
#include "paginacion.h"
#include "tlb.h"
#include "algoritmo_reemplazo.h"

extern pthread_mutex_t mutex_VGID;
extern pthread_mutex_t mutex_TGP;
extern pthread_mutex_t mutex_puertoSWAMP;
extern pthread_mutex_t mutex_METRICS_TLB;
extern pthread_mutex_t mutex_MEMORIA;


int server_escuchar(t_log* logger, char* server_name, int server_socket);
bool crear_proceso(int cliente_socket, int* id_proceso);
bool mem_alloc(int cliente_socket, int id_proceso, uint32_t size, int* add);
bool mem_free(int cliente_socket, int id_proceso, int32_t addr);
bool mem_read(int cliente_socket, int id_proceso, int32_t origin, int sizeToRead);
bool mem_write(int cliente_socket, int id_proceso, void* data, int32_t dest, int sizeToRead);
bool suspender_proceso(int cliente_socket, int id_proceso);
bool finalizar_proceso(int cliente_socket, int id_proceso);


#endif
