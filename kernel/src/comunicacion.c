/*
 * comunicacion.c
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */


#include "comunicacion.h"

typedef struct {
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

pthread_mutex_t mutexProcesosTotales = PTHREAD_MUTEX_INITIALIZER;

static void procesar_conexion(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);

    op_code cop;
    	int pid;

        if (recv(cliente_socket, &cop, sizeof(op_code), 0 ) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }

        if(cop == CREAR_PROCESO){

			recv(cliente_socket,&pid,sizeof(int),0); // no tocar
			log_info(logger, "Se recibió CREAR_PROCESO", pid);
			pthread_mutex_lock(&mutexProcesosTotales);

			if(!aprobadoPorMemoria(++PROCESOS_TOTALES)){

				R_send_crear_proceso(cliente_socket, -2);
				--PROCESOS_TOTALES;
				close(cliente_socket);
				log_info(logger, "El proceso %d no se inció ya que memoria negó su creación");
				return;
			}
			// log_info(logger, "Aprobado por memoria");

			// Creo el carpincho

			t_carpincho* nuevoCarpincho = malloc(sizeof(t_carpincho));
			nuevoCarpincho->pid = PROCESOS_TOTALES;

			pthread_mutex_unlock(&mutexProcesosTotales);


			nuevoCarpincho->rafagaAnterior = 0;
			nuevoCarpincho->estimadoAnterior = 0;
			pthread_mutex_init(&(nuevoCarpincho->mutexCarpincho ), NULL);
			nuevoCarpincho->status = NEW;

			t_running_thread* nuevoHCarpincho = malloc(sizeof(t_running_thread));
			nuevoHCarpincho->c = nuevoCarpincho;
			nuevoHCarpincho->blocked = false;
			nuevoHCarpincho->recienCreado = true;
			nuevoHCarpincho->posted= false;
			nuevoHCarpincho->socketMateLib = cliente_socket;
			nuevoHCarpincho->semaforosTomados = list_create();
			nuevoHCarpincho->bloqueadoPor = NULL ;
			//log_info(logger, "Carpincho creado");
			push_cola_new(nuevoHCarpincho); // Añado el carpincho a la cola NEW


			/*TODO
			 * gradoMultiprogramacion*/
			int valorSemMultiprogramacion;
			sem_getvalue(&gradoMultiprogramacion, &valorSemMultiprogramacion);
			if(valorSemMultiprogramacion == 0){
				sem_post(&posibleSuspencion);
			}

			// Respuesta al carpincho
			//R_send_crear_proceso(cliente_socket, nuevoCarpincho->pid);

			log_info(logger, "Se creó el proceso número %d", nuevoCarpincho->pid);

			sem_post(&carpinchosEnNew);
			// pthread_mutex_unlock(&mutexProcesosTotales);

        }
        else{
        	log_error(logger, "Llego una conexion diferente");
        	close(cliente_socket);
        }
    return;
}

int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}


bool aprobadoPorKernel(){

	if (cantidadProcesosEnSistema() < KERNEL_CFG->GRADO_MULTIPROGRAMACION){
		return true;
	}
	return false;
}

int cantidadProcesosEnSistema(){
	//TODO ver cómo obtener el valor
	return 1;
}


bool aprobadoPorMemoria(int pid){
	//Enviar mensaje a memoria con el id
	send_crear_proceso(FD_MEMORIA,pid);
	op_code cop;
	if (recv(FD_MEMORIA, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(logger, "DISCONNECT!");
		return false;
	}

	if(cop == R_CREAR_PROCESO){
		if(!R_recv_crear_proceso(FD_MEMORIA, &pid)) return false; return true;
	}

	return false;
}


