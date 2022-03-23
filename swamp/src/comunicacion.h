#ifndef COMUNICACION_SWAMP_H_
#define COMUNICACION_SWAMP_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/config.h>
#include<stdbool.h>
#include "sharedlib.h"
#include "protocol.h"

#include "AsignacionDinamica/lista_procesos.h"
#include "AsignacionFija/administrar_procesos_fija.h"
#include "config.h"
#include "log.h"


int server_escuchar(t_log* logger, char* server_name, int server_socket);
bool recv_pagina(int cliente_socket, uint32_t* pagina);
//static void desearilizar_pagina_recibida(void* stream, uint32_t* pagina);


#endif
