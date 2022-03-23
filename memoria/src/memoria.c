#include "config.h"
#include "memoria.h"

#define MODULENAME "MEMORIA"





static t_config_memoria* initialize_cfg() {
    t_config_memoria* cfg = malloc(sizeof(t_config_memoria));
    cfg->IP_ESCUCHA = NULL;
    cfg->IP_KERNEL = NULL;
    cfg->TIPO_ASIGNACION = NULL;
    cfg->ALGORITMO_REEMPLAZO_MMU = NULL;
    cfg->ALGORITMO_REEMPLAZO_TLB = NULL;
    cfg->IP_SWAMP = NULL;
    return cfg;
}


int main() {
	// CAMBIAR LAS FUNCIONES DE LOS SIGNAL
 	signal(SIGUSR1, &dump_TLB);
	signal(SIGUSR2, &clean_TLB);
	signal(SIGINT, &print_metrics);


	/*	-------------------------------------------------------------------
	    				INICIALIZACION
	    	-------------------------------------------------------------------*/
	LOGGER = log_create(DIR_LOG, "MEMORIA", true, LOG_LEVEL_INFO);
	MEMORIA_CFG = initialize_cfg();


    if(!cargar_configuracion(MEMORIA_CFG)) {
        cerrar_programa(LOGGER, MEMORIA_CFG);
        return !EXIT_FAILURE;
    }
    log_info(LOGGER, "heapMetadata %d - uint32 %d - uint8 %d", sizeof(heapMetadata), sizeof(uint32_t), sizeof(uint8_t));


    if(!cargar_memoria(&FD_SWAMP, MEMORIA_CFG, LOGGER)){
    	cerrar_programa(LOGGER, MEMORIA_CFG);
		return !EXIT_FAILURE;
    }

    /*	-------------------------------------------------------------------
    					CREACION DE SERVIDOR
    	-------------------------------------------------------------------*/
    char* puerto_escucha = string_itoa(MEMORIA_CFG->PUERTO_ESCUCHA);
    int memoria_server = iniciar_servidor(LOGGER, "MEMORIA", MEMORIA_CFG->IP_ESCUCHA, puerto_escucha);

    while (server_escuchar(LOGGER, "MEMORIA", memoria_server));


    /*	-------------------------------------------------------------------
    					FINALIZAR MODULO
    	-------------------------------------------------------------------*/
   // liberar_conexion(&memoria_server);
  //  cerrar_programa(LOGGER, MEMORIA_CFG);

    return 1;
}
