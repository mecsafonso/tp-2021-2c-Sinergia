/*
 * config.h
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <stdint.h>
typedef struct {
    char* IP_MEMORIA;
    uint16_t PUERTO_MEMORIA;
    char* ALGORITMO_PLANIFICACION;
    double ESTIMACION_INICIAL;
    double ALFA;
    char** DISPOSITIVOS_IO;
    char** DURACIONES_IO;
    uint16_t RETARDO_CPU;
    uint16_t GRADO_MULTIPROGRAMACION;
    uint16_t GRADO_MULTIPROCESAMIENTO;
    uint16_t TIEMPO_DEADLOCK;
    char* IP_ESCUCHA;
    uint16_t PUERTO_ESCUCHA;
} t_config_kernel;

extern t_config_kernel* KERNEL_CFG;

int PROCESOS_TOTALES;

#endif
