/*
 * init.c
 *
 *  Created on: 20 sep. 2021
 *      Author: utnso
 */

#include "config.h"
#include "init.h"
#include "log.h"

uint8_t cargar_configuracion(t_config_kernel* config) {
    t_config* cfg = config_create(DIR_CONFIG);

    if(cfg == NULL) {
        log_error(logger, "No se encontro kernel.config");
        return 0;
    }

    char* properties[] = {
        "IP_MEMORIA",
        "PUERTO_MEMORIA",
        "ALGORITMO_PLANIFICACION",
        "ESTIMACION_INICIAL",
        "ALFA",
        "DISPOSITIVOS_IO",
        "DURACIONES_IO",
        "RETARDO_CPU",
        "GRADO_MULTIPROGRAMACION",
        "GRADO_MULTIPROCESAMIENTO",
        "IP_ESCUCHA",
        "PUERTO_ESCUCHA",
        NULL
    };

    // Falta alguna propiedad
    if(!config_has_all_properties(cfg, properties)) {
        log_error(logger, "Propiedades faltantes en el archivo de configuracion");
        config_destroy(cfg);
        return 0;
    }

	config->IP_MEMORIA = strdup(config_get_string_value(cfg, "IP_MEMORIA"));
	config->PUERTO_MEMORIA = config_get_int_value(cfg, "PUERTO_MEMORIA");
	config->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(cfg, "ALGORITMO_PLANIFICACION"));
	config->ESTIMACION_INICIAL = config_get_double_value(cfg, "ESTIMACION_INICIAL");
	config->ALFA = config_get_double_value(cfg, "ALFA");
	config->DISPOSITIVOS_IO = config_get_array_value(cfg, "DISPOSITIVOS_IO"); // El array devuelto termina en NULL
	config->DURACIONES_IO = config_get_array_value(cfg, "DURACIONES_IO"); // El array devuelto termina en NULL
	config->RETARDO_CPU = config_get_int_value(cfg, "RETARDO_CPU");
	config->GRADO_MULTIPROGRAMACION = config_get_int_value(cfg, "GRADO_MULTIPROGRAMACION");
	config->GRADO_MULTIPROCESAMIENTO = config_get_int_value(cfg, "GRADO_MULTIPROCESAMIENTO");
	config->IP_ESCUCHA = strdup(config_get_string_value(cfg, "IP_ESCUCHA"));
	config->PUERTO_ESCUCHA = config_get_int_value(cfg, "PUERTO_ESCUCHA");

	// Tipo de planificación

/*
    if(strncmp(config->ALGORITMO, "RR", 2) == 0)
        correr_tripulante = correr_tripulante_RR;
    else
        correr_tripulante = correr_tripulante_FIFO;
*/

    log_info(logger, "Archivo de configuración cargado correctamente.");
    log_info(logger, "Algoritmo de planificación utilizado: %s.", config->ALGORITMO_PLANIFICACION);
    config_destroy(cfg);

    return 1;
}


uint8_t generar_conexiones(int* fd_memoria, t_config_kernel* cfg) {

    char* port_memoria = string_itoa(cfg->PUERTO_MEMORIA);


   *fd_memoria = crear_conexion(
        logger,
        "MEMORIA",
        cfg->IP_MEMORIA,
        port_memoria
    );

    free(port_memoria);

    FD_MEMORIA = *fd_memoria;

    return *fd_memoria != 0;
}

void cerrar_programa(t_log* logger, t_log* logInv, t_config_kernel* cfg) {
    log_destroy(logger);
    log_destroy(logInv);

    free(cfg->ALGORITMO_PLANIFICACION);
    free(cfg->IP_MEMORIA);
    free(cfg->IP_ESCUCHA);
    string_array_destroy(cfg->DISPOSITIVOS_IO);
    string_array_destroy(cfg->DURACIONES_IO);
    free(cfg);

}

