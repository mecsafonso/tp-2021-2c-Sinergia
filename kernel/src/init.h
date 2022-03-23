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
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define DIR_CONFIG "/home/utnso/tp-2021-2c-Sinergia/kernel/kernel.config"

uint8_t cargar_configuracion(t_config_kernel* config);
uint8_t generar_conexiones(int* fd_memoria, t_config_kernel* cfg);
void cerrar_programa(t_log* logger, t_log* logInv, t_config_kernel* cfg);

int FD_MEMORIA;

#endif /* INIT_H_ */
