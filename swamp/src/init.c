#include "config.h"
#include "init.h"
#include "log.h"


uint8_t cargar_configuracion(t_config_swamp* config) {
    t_config* cfg = config_create(DIR_CONFIG);

    if(cfg == NULL) {
        log_error(log_swamp, "No se encontro swamp.config");
        return 0;
    }

    char* properties[] = {
        "IP_ESCUCHA",
        "PUERTO_ESCUCHA",
        "IP_MEMORIA",
        "PUERTO_MEMORIA",
        "TAMANIO_SWAP",
        "TAMANIO_PAGINA",
        "ARCHIVOS_SWAP",
        "MARCOS_MAXIMOS",
        "RETARDO_SWAP",
        NULL
    };

    // Falta alguna propiedad
    if(!config_has_all_properties(cfg, properties)) {
        log_error(log_swamp, "Propiedades faltantes en el archivo de configuracion");
        config_destroy(cfg);
        return 0;
    }

	config->IP_ESCUCHA = strdup(config_get_string_value(cfg, "IP_ESCUCHA"));
	config->PUERTO_ESCUCHA = config_get_int_value(cfg, "PUERTO_ESCUCHA");
	config->IP_MEMORIA = strdup(config_get_string_value(cfg, "IP_MEMORIA"));
	config->PUERTO_MEMORIA = config_get_int_value(cfg, "PUERTO_MEMORIA");
    config->TAMANIO_SWAP = config_get_int_value(cfg, "TAMANIO_SWAP");
    config->TAMANIO_PAGINA = config_get_int_value(cfg, "TAMANIO_PAGINA");
    config->ARCHIVOS_SWAP = config_get_array_value(cfg, "ARCHIVOS_SWAP"); // El array devuelto termina en NULL
    config->MARCOS_MAXIMOS = config_get_int_value(cfg, "MARCOS_MAXIMOS");
    config->RETARDO_SWAP = config_get_int_value(cfg, "RETARDO_SWAP");
    config->TIPO_ASIGNACION = "POR DEFINIR";


    log_info(log_swamp, "Archivo de configuracion cargado correctamente");

    config_destroy(cfg);

    return 1;
}

void cerrar_programa(t_log* logger, t_log* logInv, t_config_swamp* cfg) {
    log_destroy(logger);
    log_destroy(logInv);


    free(cfg->IP_MEMORIA);
    free(cfg->IP_ESCUCHA);
    string_array_destroy(cfg->ARCHIVOS_SWAP);
    free(cfg);



}

void crear_archivos(){
	ARCHIVOS = list_create();

	char** listaDeArchivos = SWAMP_CFG->ARCHIVOS_SWAP;
	for(int indiceLista = 0; listaDeArchivos[indiceLista] != NULL; indiceLista++){
		int fd;
		void* archivo;
		remove(listaDeArchivos[indiceLista]); //Borro archivos de previas ejecuciones

		fd = open(listaDeArchivos[indiceLista], O_CREAT | O_RDWR, 00600);

		if (fd == -1){
			printf("error con shared memory");
			exit(1);
		}
		ftruncate(fd, (SWAMP_CFG->TAMANIO_SWAP));
		archivo = mmap(NULL, (SWAMP_CFG->TAMANIO_SWAP), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		msync(archivo, (SWAMP_CFG->TAMANIO_SWAP), MS_SYNC);

		list_add(ARCHIVOS, archivo);

		close(fd);
		log_info(log_swamp,"Se genero el archivo %s", listaDeArchivos[indiceLista]);
	}
}





