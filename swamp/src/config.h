#ifndef SWAMP_CONFIG_H
#define SWAMP_CONFIG_H

#include <stdint.h>
typedef struct {
    char* IP_ESCUCHA;
    uint16_t PUERTO_ESCUCHA;
    char* IP_MEMORIA;
    uint16_t PUERTO_MEMORIA;
    uint32_t TAMANIO_SWAP;
    uint32_t TAMANIO_PAGINA;
    char** ARCHIVOS_SWAP;
    uint32_t MARCOS_MAXIMOS;
    uint32_t RETARDO_SWAP;
    char* TIPO_ASIGNACION;
} t_config_swamp;

extern t_config_swamp* SWAMP_CFG;

#endif
