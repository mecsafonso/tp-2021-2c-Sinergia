/*
 * protocol.h
 *
 *  Created on: 24 sep. 2021
 *      Author: utnso
 */

#ifndef SRC_PROTOCOL_H_
#define SRC_PROTOCOL_H_

#include <sys/socket.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MATE_FREE_FAULT -5
#define MATE_READ_FAULT -6
#define MATE_WRITE_FAULT -7

typedef enum {

	CREAR_PROCESO,
	R_CREAR_PROCESO,
	FINALIZAR_PROCESO,
	R_FINALIZAR_PROCESO,
	SUSPENDER_PROCESO,
	R_SUSPENDER_PROCESO,


	CALL_IO,

	MEM_ALLOC,
	R_MEM_ALLOC,

	MEM_FREE,
	R_MEM_FREE,

	MEM_READ,
	R_MEM_READ,

	MEM_WRITE,
	R_MEM_WRITE,


	//SEMAFOROS
	INICIAR_SEM,
	R_INICIAR_SEM,
	WAIT_SEM,
	R_WAIT_SEM,
	R_WAIT_SEM_ELIMINADO,
	POST_SEM,
	R_POST_SEM,
	DESTROY_SEM,
	R_DESTROY_SEM,

	//Swamp
	PEDIDO_DE_LECTURA,
	PEDIDO_DE_ESCRITURA,
	BORRAR_PARTICION_PROCESO,
	TIPO_DE_ASIGNACION,

	//MEMORIA
	SWAMP_READY,
	R_SWAMP_READY,
	CONSULTAR_FRAME_LIBRE_SWAMP,
	R_CONSULTAR_FRAME_LIBRE_SWAMP,
	RESERVAR_FRAME_LIBRE_SWAMP,
	R_RESERVAR_FRAME_LIBRE_SWAMP,
	FREE_FRAME_SWAMP,
	R_FREE_FRAME_SWAMP,
	GUARDAR_EN_SWAMP,
	R_GUARDAR_EN_SWAMP,
	PEDIR_PAGE_A_SWAMP,
	R_PEDIR_PAGE_A_SWAMP,

}op_code;

// FUNCIONES PARA LA SERIALIZACION Y DESERIALIZACION
void serializarVariable(void* AEnviar, void* ASerializar, int tamanio, int *offset);
void deserializarVariable(void* variable, void* stream, int tamanio, int* offset);
void serializarChar(void* mensajeSerializado, char* mensaje, uint32_t tamanio, int* offset);
void deserializarChar(char** receptor, void* stream, int* offset);

// CREAR_PROCESO
bool send_crear_proceso(int fd, int pid);
bool recv_crear_proceso(int fd, int* pid);

// Respuesta CREAR_PROCESO
bool R_send_crear_proceso(int fd, int pid);
bool R_recv_crear_proceso(int fd, int* pid);

// FINALIZAR_PROCESO
bool send_finalizar_proceso(int fd, int pid);
bool recv_finalizar_proceso(int fd, int* numeroDeProceso);

//R SUSPENDER_PROCESO
bool send_R_finalizar_proceso(int fd, int success);
bool recv_R_finalizar_proceso(int fd, int* success);

// SUSPENDER_PROCESO
bool send_suspender_proceso(int fd, int pid);
bool recv_suspender_proceso(int fd, int* pid);

//R SUSPENDER_PROCESO
bool send_R_suspender_proceso(int fd, int success);
bool recv_R_suspender_proceso(int fd, int* success);

// MEM_ALLOC
bool send_mem_alloc(int fd, uint32_t tamanio, int pid);
bool recv_mem_alloc(int fd, uint32_t* tamanio, int* pid);

// R_MEM_ALLOC
bool send_R_mem_alloc(int fd, int direccionLogica);
bool recv_R_mem_alloc(int fd, int* direccionLogica);

// MEM_FREE,
bool send_mem_free(int fd, int32_t addr, int pid);
bool recv_mem_free(int fd, int32_t* addr, int* pid);

// R_MEM_FREE,
bool send_R_mem_free(int fd, int direccionLogica);
bool recv_R_mem_free(int fd, int* direccionLogica);

// MEM_READ
bool send_mem_read(int fd, int32_t origin, int sizeToRead, int pid);
bool recv_mem_read(int fd, int32_t* origin, int* sizeToRead, int* pid);

// R_MEM_READ
bool send_R_mem_read(int fd, void* dest, int sizeToRead);
bool recv_R_mem_read(int fd, void* dest, int* sizeToRead);

// MEM_WRITE
bool send_mem_write(int fd, void *origin, int32_t dest, int sizeOrigin, int pid);
bool recv_mem_write(int fd, void **origin, int32_t* dest, int* sizeOrigin, int* pid);

// R_MEM_WRITE
bool send_R_mem_write(int fd, int rta);
bool recv_R_mem_write(int fd, int* rta);

// PROCESO BLOQUEADO POR SEMAFORO
bool send_block_sem(int fd, char* sem_name);
bool recv_block_sem(int fd, int* numeroDeProceso, char** sem_name);

// PROCESO DESBLOQUEADO POR SEMAFORO
/*
 * TODO: ver la parte de semaforos. Actualemente lo tenemos planteado para que cada carpincho tenga sus propios
 * semaforos, pero la consigna dice que por cada post se libera un carpincho en orden que fueron bloqueados.
 * Esto me da a entender que a lo mejor los carpinchos comparten semáforos
 */
bool send_unblock_sem(int fd, char* sem_name);
bool recv_unblock_sem(int fd, int* numeroDeProceso, char** sem_name);


// INICIAR_SEM
bool send_iniciar_sem(int fd, int pid, char* nombreSem, int valorInicial);
bool recv_iniciar_sem(int fd, int* pid, int* valorInicial, char** nombreSem);

// R_INICIAR_SEM
bool send_R_iniciar_sem(int fd, int success);
bool recv_R_iniciar_sem(int fd, int* success);

// WAIT_SEM
bool send_wait_sem(int fd, int pid, char* nombreSem);
bool recv_wait_sem(int fd, int* pid,  char** nombreSem);

// R_WAIT_SEM
bool send_R_wait_sem(int fd);
bool recv_R_wait_sem(int fd);

// POST_SEM
bool send_post_sem(int fd, int pid, char* nombreSem);
bool recv_post_sem(int fd, int* pid,  char** nombreSem);
// R_POST_SEM
bool send_R_post_sem(int fd);
bool recv_R_post_sem(int fd);

// DESTROY_SEM
bool send_destroy_sem(int fd, int pid, char* nombreSem);
bool recv_destroy_sem(int fd, int* pid,  char** nombreSem);

// R_DESTROY_SEM
bool send_R_destoy_sem(int fd);
bool recv_R_destoy_sem(int fd);




// CALL_IO
bool send_call_io(int fd, int pid, char* recurso, char* mensaje);
bool recv_call_io(int fd, int* pid, char** recurso, char** mensaje);

// R_CALL_IO
bool send_R_call_io(int fd);
bool recv_R_call_io(int fd);


/*	-------------------------------------------------------------------
    				MEMORIA
    	-------------------------------------------------------------------*/

//SWAMP READY
bool send_swamp_ready(int fd, char* tipo_asignacion);
bool recv_swamp_ready(int fd, char** tipoAsignacion);
//R_SWAMP_READY
bool send_R_swamp_ready(int fd, int sucess);
bool recv_R_swamp_ready(int fd, int* sucess);

//CONSULTAR FRAME LIBRE SWAMP
bool send_consultar_frame_libre_swamp(int fd, int id_proceso, int cant_frames);
bool recv_consultar_frame_libre_swamp(int fd, int* id_proceso, int* cant_frames) ;

//R CONSULTAR FRAME LIBRE SWAMP
bool send_R_consultar_frame_libre_swamp(int fd, int cant_frames);
bool recv_R_consultar_frame_libre_swamp(int fd, int* cant_frames);

//RESERVAR FRAME LIBRE SWAMP
bool send_reservar_frame_libre_swamp(int fd, int id_proceso, int success);
bool recv_reservar_frame_libre_swamp(int fd, int* id_proceso, int* success) ;

//R RESERVAR FRAME LIBRE SWAMP
bool send_R_reservar_frame_libre_swamp(int fd, int sucess);
bool recv_R_reservar_frame_libre_swamp(int fd, int* sucess);

//FREE FRAME SWAMP
bool send_free_frame_swamp(int fd, int id_proceso, int frame);
bool recv_free_frame_swamp(int fd, int* id_proceso, int* frame);

//R FREE FRAME SWAMP
bool send_R_free_frame_swamp(int fd, int frame);
bool recv_R_free_frame_swamp(int fd, int* frame);

//GUARDAR EN SWAMP
bool send_guardar_en_swamp(int fd, int id_proceso, int frame, void* data, int sizeToRead);
bool recv_guardar_en_swamp(int fd, int* id_proceso, int* frame, void** data, int* sizeToRead);

//R GUARDAR EN SWAMP
bool send_R_guardar_en_swamp(int fd, int success);
bool recv_R_guardar_en_swamp(int fd, int* success);

//PEDIR PAGE A SWAMP
bool send_pedir_page_a_swamp(int fd, int id_process, int page);
bool recv_pedir_page_a_swamp(int fd, int* id_process, int* page);

//R PEDIR PAGE A SWAMP
bool send_R_pedir_page_a_swamp(int fd, void* dest, int sizeToRead);
bool recv_R_pedir_page_a_swamp(int fd, void** dest);



#endif /* SRC_PROTOCOL_H_ */
