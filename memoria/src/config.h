#ifndef MEMORIA_CONFIG_H
#define MEMORIA_CONFIG_H

#include <stdint.h>

typedef struct {
    char* IP_ESCUCHA;
    uint16_t PUERTO_ESCUCHA;
    char* IP_SWAMP;
    uint16_t PUERTO_SWAMP;
    char* IP_KERNEL;
    uint16_t PUERTO_KERNEL;
    uint16_t TAMANIO_MEMORIA;
    char* TIPO_ASIGNACION;
    char* ALGORITMO_REEMPLAZO_MMU;
    uint16_t CANTIDAD_ENTRADAS_TLB;
    char* ALGORITMO_REEMPLAZO_TLB;
    uint16_t RETARDO_ACIERTO_TLB;
    uint16_t RETARDO_FALLO_TLB;
    uint16_t MARCOS_MAXIMOS;
    uint16_t TAMANIO_PAGINA;

} t_config_memoria;

extern t_config_memoria* MEMORIA_CFG;

#endif
