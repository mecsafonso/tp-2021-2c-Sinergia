#ifndef KERNEL_H_
#define KERNEL_H_

#include "sharedlib.h"

#include <stdio.h>
#include "comunicacion.h"
#include "init.h"

#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>


#include "log.h"
#include "planificadores.h"
#include "hiloCpu.h"


// Inicializacion

void inicializacion();
void realizarConfiguracion();


void iniciarPlanificadores();
void iniciarHilosCPU();

// Configuración y Logger


#define DIR_LOG "/home/utnso/tp-2021-2c-Sinergia/kernel/kernel.log"


// Variables Globales del archivo config

char* IP_MEMORIA;
int PUERTO_MEMORIA;
char* ALGORITMO_PLANIFICACION;
int ESTIMACION_INICIAL;
float ALFA;
char** DISPOSITIVOS_IO;
char** DURACIONES_IO;
int RETARDO_CPU;
int GRADO_MULTIPROGRAMACION;
int GRADO_MULTIPROCESAMIENTO;
int IP_ESCUCHA;
int PUERTO_ESCUCHA;

extern t_list* LIST_IO;


bool SERVIDOR_MEMORIA_CONECTADO;

bool crearEntradasSalidas();
void dispositivoIO(void* args);


#endif /* KERNEL_H_ */
