#include "config.h"
#include "swamp.h"


t_config_swamp* SWAMP_CFG;

t_log* log_swamp;
t_log* logInv_swamp;

int memoria_server;

static t_config_swamp* initialize_cfg() {
    t_config_swamp* cfg = malloc(sizeof(t_config_swamp));
    cfg->IP_ESCUCHA = NULL;
    cfg->IP_MEMORIA = NULL;
	cfg->ARCHIVOS_SWAP = NULL;
	return cfg;
}

int main(){

	init();
/*	//PRUEBAS
	crearProceso_F(1);
	crearProceso_F(2);

	int espacioLibre = 0;
	for(int inicioProceso = 0; inicioProceso < 30; inicioProceso++){
		if(bitarray_test_bit(BITMAP, inicioProceso)){
			espacioLibre++;
		}
	}
	log_info(log_swamp, "Frames del bitmap ocupados: %d", espacioLibre);

	crearProceso_F(3);

	log_info(log_swamp, "El proceso 1 tiene %d, frames disponibles", buscarFramesDisponibles_F(1));

	log_info(log_swamp, "Le asigno el frame %d al proceso 2", asignarEspacio_F(2));
	log_info(log_swamp, "Le asigno el frame %d al proceso 2", asignarEspacio_F(2));
	log_info(log_swamp, "Le libero el frame %d al proceso 2", liberarPagina_F(2, 10));
	log_info(log_swamp, "El proceso 2 tiene %d, frames disponibles", buscarFramesDisponibles_F(2));
	log_info(log_swamp, "El proceso 3 tiene %d, frames disponibles", buscarFramesDisponibles_F(3));
*/

    // ****** CREACION DEL SERVIDOR ******
    char* puerto = string_itoa(SWAMP_CFG->PUERTO_ESCUCHA);
    memoria_server = iniciar_servidor(log_swamp, "SWAMP_SERVER", SWAMP_CFG->IP_ESCUCHA, puerto);
    free(puerto);


    while (server_escuchar(log_swamp,"SWAMP_SERVER", memoria_server));

    liberar_conexion(&memoria_server);
	cerrar_programa(log_swamp,logInv_swamp,SWAMP_CFG);

	return EXIT_SUCCESS;
}


void init(){
    log_swamp = log_create(DIR_LOG, "SWAMP", true, LOG_LEVEL_INFO);
    logInv_swamp = log_create(DIR_LOG, "SWAMP", false, LOG_LEVEL_TRACE);

    SWAMP_CFG = initialize_cfg();
	cargar_configuracion(SWAMP_CFG);
	crear_bitmap();
	crear_archivos();
	LISTA_PROCESOS = list_create();
}


