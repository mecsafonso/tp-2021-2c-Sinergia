/*
 * init.h
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#ifndef INIT_H_
#define INIT_H_

#include "sharedlib.h"
#include "log.h"
#include "config.h"
#include "paginas.h"
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#define DIR_CONFIG "/home/utnso/tp-2021-2c-Sinergia/swamp/swamp.config"

t_bitarray* BITMAP;

t_list* LISTA_PROCESOS;

t_list* ARCHIVOS;

typedef struct{
	int pid;
	int indiceArchivo;
	char* archivo;
	int primerFrameOcupado;
	t_bitarray* bitmapProceso;
}t_proceso;

uint8_t cargar_configuracion(t_config_swamp* config);
uint8_t generar_conexiones(int* fd_memoria, t_config_swamp* cfg);
void cerrar_programa(t_log* logger, t_log* logInv, t_config_swamp* cfg);
void crear_archivos();
#endif /* INIT_H_ */
