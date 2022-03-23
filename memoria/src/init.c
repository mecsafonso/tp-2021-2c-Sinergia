
#include "init.h"
#include "paginacion.h"

pthread_mutex_t mutex_VGID  = PTHREAD_MUTEX_INITIALIZER;

uint8_t cargar_configuracion(t_config_memoria* config) {
    t_config* cfg = config_create(DIR_CONFIG);

    if(cfg == NULL) {
        log_error(LOGGER, "No se encontro memoria.config");
        return false;
    }

    char* properties[] = {
        "IP_ESCUCHA",
        "PUERTO_ESCUCHA",
        "IP_SWAMP",
        "PUERTO_SWAMP",
        "IP_KERNEL",
        "PUERTO_KERNEL",
        "TAMANIO",
        "TIPO_ASIGNACION",
        "ALGORITMO_REEMPLAZO_MMU",
        "CANTIDAD_ENTRADAS_TLB",
        "ALGORITMO_REEMPLAZO_TLB",
        "RETARDO_ACIERTO_TLB",
        "RETARDO_FALLO_TLB",
        "MARCOS_MAXIMOS",
        "TAMANIO_PAGINA",
        NULL
    };

    // Falta alguna propiedad
    if(!config_has_all_properties(cfg, properties)) {
        log_error(LOGGER, "Propiedades faltantes en el archivo de configuracion");
        config_destroy(cfg);
        return false;
    }

	config->IP_ESCUCHA = strdup(config_get_string_value(cfg, "IP_ESCUCHA"));
	config->PUERTO_ESCUCHA = config_get_int_value(cfg, "PUERTO_ESCUCHA");
	config->IP_KERNEL = strdup(config_get_string_value(cfg, "IP_KERNEL"));
	config->PUERTO_KERNEL = config_get_int_value(cfg, "PUERTO_KERNEL");
	config->IP_SWAMP = strdup(config_get_string_value(cfg, "IP_SWAMP"));
	config->PUERTO_SWAMP = config_get_int_value(cfg, "PUERTO_SWAMP");

	config->TAMANIO_MEMORIA = config_get_int_value(cfg, "TAMANIO");
	config->TIPO_ASIGNACION = strdup(config_get_string_value(cfg, "TIPO_ASIGNACION"));
	config->ALGORITMO_REEMPLAZO_MMU = strdup(config_get_string_value(cfg, "ALGORITMO_REEMPLAZO_MMU"));
	config->CANTIDAD_ENTRADAS_TLB = config_get_int_value(cfg, "CANTIDAD_ENTRADAS_TLB");
	config->ALGORITMO_REEMPLAZO_TLB = strdup(config_get_string_value(cfg, "ALGORITMO_REEMPLAZO_TLB"));
	config->RETARDO_ACIERTO_TLB = config_get_int_value(cfg, "RETARDO_ACIERTO_TLB");
	config->RETARDO_FALLO_TLB = config_get_int_value(cfg, "RETARDO_FALLO_TLB");
	config->MARCOS_MAXIMOS = config_get_int_value(cfg, "MARCOS_MAXIMOS");

	config->TAMANIO_PAGINA = config_get_int_value(cfg, "TAMANIO_PAGINA");

    log_info(LOGGER, "Archivo de configuracion cargado correctamente");
    //log_info(LOGGER, "%s", config->IP_ESCUCHA);

    config_destroy(cfg);

    return true;
}

uint8_t generar_conexiones(int* fd_swamp, t_config_memoria* cfg) {

    char* port_swamp = string_itoa(cfg->PUERTO_SWAMP);
   *fd_swamp = crear_conexion(LOGGER, "SWAMP", cfg->IP_SWAMP, port_swamp);
    free(port_swamp);

    return  *fd_swamp != 0;
}

void generar_size_heapmetadata(){
	SIZE_HEAPMETADATA = (sizeof(uint32_t) * 2) + sizeof(uint8_t);
}

uint8_t cargar_memoria(int* fd_swamp, t_config_memoria* cfg, t_log* logger){
	pthread_mutex_lock(&mutex_VGID);
	VGID = 1;
	pthread_mutex_unlock(&mutex_VGID);

	generar_size_heapmetadata();

    if(!init_memoria(cfg, logger))
        return false;

    //VERIFICACION QUE ESTA SWAMP ESTA LISTA PARA RECIBIR MENSAJES
    if (!generar_conexiones(fd_swamp, cfg))
    	return false;

    // Mensaje a SWAMP
	if(*fd_swamp != 0){
		if(!send_swamp_ready(*fd_swamp, cfg->TIPO_ASIGNACION)){
			log_error(logger, "Fallo en el envio para swamp ready");
			return false;
		}
	}

	op_code cop;
	int success;

	//RTA de SWAMP
	if (recv(*fd_swamp, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
		log_info(logger, "SWAMP DISCONNECT!");
		return 0;
	}
	if(cop == R_SWAMP_READY){
		if(!recv_R_swamp_ready(*fd_swamp, &success)){
			return false;
		}
		if(!success){
			log_error(logger, "SWAMP con problemas de inicializacion de MEM VIRTUAL");
			return false;
		}
		else
			log_warning(logger, "SWAMP READY");
	}

    //VERIFICACION QUE ESTA SWAMP ESTA LISTA PARA RECIBIR MENSAJES
    if (!generar_conexiones(fd_swamp, cfg))
    	return false;

	return true;
}






void cerrar_programa(t_log* logger, t_config_memoria* cfg) {
	//terminar_paginacion(cfg);

	log_destroy(logger);

    free(cfg->IP_ESCUCHA);
    free(cfg->IP_KERNEL);
    free(cfg->IP_SWAMP);
    free(cfg->TIPO_ASIGNACION);
    free(cfg->ALGORITMO_REEMPLAZO_MMU);
    free(cfg->ALGORITMO_REEMPLAZO_TLB);
    free(cfg);
}



/*	-------------------------------------------------------------------
						DUMP
	-------------------------------------------------------------------*/
void escribirInfoDeTiempoEnDump(FILE* dumpFile){
  time_t hoy = time(NULL);
  struct tm* tiempoInfo;
  setenv("TZ", "America/Buenos_Aires", 1);
  tiempoInfo = localtime(&hoy);
  fprintf(dumpFile, "%d/%d/%d %d:%d:%d\n", tiempoInfo->tm_mday, tiempoInfo->tm_mon + 1, tiempoInfo->tm_year + 1900, tiempoInfo->tm_hour, tiempoInfo->tm_min, tiempoInfo->tm_sec);
}

void escribirInfoDeLasEntradas(FILE* dumpFile){
	char* estado;
	int i;
	t_tlb TLB_ouput[MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB];

	//inicializar TLB_ouput
	for(i = 0; i < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB; i++){
		TLB_ouput[i].idProcess = -1;
		TLB_ouput[i].page = -1;
		TLB_ouput[i].frame_number = -1;
	}

	//agregar data real de la TLB
	i = 0;
	pthread_mutex_lock(&mutex_TLB);
	t_link_element* elemento = TLB->head;
	pthread_mutex_unlock(&mutex_TLB);
	t_tlb* info;


	while (elemento != NULL) {
		info = (t_tlb*) elemento->data;
		TLB_ouput[i] = *info;

		pthread_mutex_lock(&mutex_TLB);
		elemento = elemento->next;
		pthread_mutex_unlock(&mutex_TLB);
		i++;
	}


	//Print de TLB
	for(i = 0; i < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB; i++){
		if(TLB_ouput[i].idProcess != -1)
			estado = "OCUPADO";
		else
			estado = "LIBRE";
		fprintf(dumpFile, "Entrada: %-10d Estado: %-20s Carpincho: %-10d Pagina: %-10d Marco: %-10d\n", i, estado, TLB_ouput[i].idProcess, TLB_ouput[i].page, TLB_ouput[i].frame_number);

	}
}

void dump_TLB(int sig){
	pthread_mutex_lock(&mutex_TLB);
	FILE* dumpFile = fopen(DUMP_PATH_TLB, "w");

	const char* line = "-----------------------------------------------------------------------------------------------------------------------------";

	fprintf(dumpFile,"%s\n", line);
	escribirInfoDeTiempoEnDump(dumpFile);
	escribirInfoDeLasEntradas(dumpFile);
	fprintf(dumpFile,"%s\n", line);

	log_warning(LOGGER,"Dump de TLB ha sido generado");
	fclose(dumpFile);
	pthread_mutex_unlock(&mutex_TLB);
}

void clean_TLB(int sig){
	pthread_mutex_lock(&mutex_TLB);
	list_clean(TLB);
	pthread_mutex_unlock(&mutex_TLB);
}

/*	-------------------------------------------------------------------
						SINGINT => METRICAS
	-------------------------------------------------------------------*/

void print_metrics(int sig){
	int total_MISS = 0;
	int total_HIT = 0;
	const char* line = "-------------------------------------------------------------------------------------------------------------";


	printf("%s\n", line);
	printf("%20s METRICAS TLB\n", " ");
	printf("%s\n", line);

	if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0){
		pthread_mutex_lock(&mutex_METRICS_TLB);
		if(!list_is_empty(TLB_METRICS)){
			t_link_element* elemento = TLB_METRICS->head;
			pthread_mutex_unlock(&mutex_METRICS_TLB);
			t_metrics_tlb* entry;

			while (elemento != NULL){
				entry = (t_metrics_tlb*)elemento->data;

				if(entry->hit != 0){
					total_HIT += entry->hit;
				}

				if(entry->miss != 0){
					total_MISS += entry->miss;
				}

				printf("PID:%3d	Total HIT:%3d, Total MISS:%3d\n", entry->id_process, entry->hit, entry->miss);

				pthread_mutex_lock(&mutex_METRICS_TLB);
				elemento = elemento->next;
				pthread_mutex_unlock(&mutex_METRICS_TLB);

			}
		}
		pthread_mutex_unlock(&mutex_METRICS_TLB);

		printf("\nTLB Hit totales = %d\n", total_HIT);
		printf("TLB MISS totales = %d\n", total_MISS);
		printf("%s\n", line);
	}
	else
		printf("no hay TLB\n %s\n", line);

	exit(1);
}





void print_TLB(){
	char* estado;
	int i;
	t_tlb TLB_ouput[MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB];

	//inicializar TLB_ouput
	for(i = 0; i < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB; i++){
		TLB_ouput[i].idProcess = -1;
		TLB_ouput[i].page = -1;
		TLB_ouput[i].frame_number = -1;
	}

	//agregar data real de la TLB
	i = 0;
	pthread_mutex_lock(&mutex_METRICS_TLB);
	if(!list_is_empty(TLB)){
		t_link_element* elemento = TLB->head;
		pthread_mutex_unlock(&mutex_METRICS_TLB);
		t_tlb* info;

		while (elemento != NULL) {
			info = (t_tlb*) elemento->data;
			TLB_ouput[i] = *info;

			pthread_mutex_lock(&mutex_METRICS_TLB);
			elemento = elemento->next;
			pthread_mutex_unlock(&mutex_METRICS_TLB);
			i++;
		}
	}
	pthread_mutex_unlock(&mutex_METRICS_TLB);

	//Print de TLB
	for(i = 0; i < MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB; i++){
		if(TLB_ouput[i].idProcess != -1)
			estado = "OCUPADO";
		else
			estado = "LIBRE";
		printf("Entrada: %-10d Estado: %-20s Carpincho: %-10d Pagina: %-10d Marco: %-10d\n", i, estado, TLB_ouput[i].idProcess, TLB_ouput[i].page, TLB_ouput[i].frame_number);

	}
}

void print_memoria(){
	int nro_frames = MEMORIA_CFG->TAMANIO_MEMORIA/MEMORIA_CFG->TAMANIO_PAGINA;


	t_page MEMORIA[nro_frames];

	//inicializar TLB_ouput
	for(int i = 0; i < nro_frames; i++){
		t_page page;
		page.idProcess = -1;
		page.idPage = -1;
		page.memory_frame_number = -1;
		page.modified = -1;
		page.swap_frame_number = -1;

		MEMORIA[i] = page;
	}

	//PRINT PARA DINAMICA
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "DINAMICA") == 0){
		if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){
			int i = 0;
			pthread_mutex_lock(&mutex_paginasEnMemoria);
			if(!list_is_empty(MEMORY_PAGES)){
				t_link_element* elemento = MEMORY_PAGES->head;
				pthread_mutex_unlock(&mutex_paginasEnMemoria);
				t_page* info;

				while (elemento != NULL) {
			//		log_info(LOGGER, "FRAME: %-10d Estado: %-20s Carpincho: %-10d Pagina: %-10d Marco: %-10d\n", i, "OCUPADO", info->idProcess, info->idPage, info->memory_frame_number);
					info = (t_page*) elemento->data;
					MEMORIA[info->memory_frame_number] = *info;

					pthread_mutex_lock(&mutex_paginasEnMemoria);
					elemento = elemento->next;
					pthread_mutex_unlock(&mutex_paginasEnMemoria);
					i++;
				}
			}
			pthread_mutex_unlock(&mutex_paginasEnMemoria);
		}
		else{
			pthread_mutex_lock(&mutex_paginasEnMemoria);
			for(int i = 0; i < nro_frames; i++){
				t_element comparar;
				comparar.page = NULL;
				comparar.use = 0;
				t_element elemento = POINTER_DINAMICA[i].page == NULL? comparar : POINTER_DINAMICA[i];

				if(elemento.page != NULL){
					t_page* page = elemento.page;
					MEMORIA[page->memory_frame_number] = *page;
				}
			}
			pthread_mutex_unlock(&mutex_paginasEnMemoria);
		}
	}
	//PRINT PARA ASIGNACION FIJA
	else if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		if(strcmp(MEMORIA_CFG->ALGORITMO_REEMPLAZO_MMU, "LRU") == 0){

			pthread_mutex_lock(&mutex_paginasEnMemoria);
			if(!list_is_empty(MEMORY_PAGES)){
				t_link_element* elemento = MEMORY_PAGES->head;
				pthread_mutex_unlock(&mutex_paginasEnMemoria);
				t_pages_LRU_fija* info;

				while (elemento != NULL) {
					info = (t_pages_LRU_fija*) elemento->data;

					pthread_mutex_lock(&mutex_paginasEnMemoria);
					t_link_element* nodo = info->pages->head;
					pthread_mutex_unlock(&mutex_paginasEnMemoria);
					t_page* page;

					while(nodo != NULL){
						page = (t_page*) nodo->data;
						MEMORIA[page->memory_frame_number] = *page;

						pthread_mutex_lock(&mutex_paginasEnMemoria);
						nodo = nodo->next;
						pthread_mutex_unlock(&mutex_paginasEnMemoria);
					}

					pthread_mutex_lock(&mutex_paginasEnMemoria);
					elemento = elemento->next;
					pthread_mutex_unlock(&mutex_paginasEnMemoria);
				}
			}
			pthread_mutex_unlock(&mutex_paginasEnMemoria);
		}
		else{
			pthread_mutex_lock(&mutex_paginasEnMemoria);
			if(!list_is_empty(POINTER_FIJA)){
				t_link_element* elemento = POINTER_FIJA->head;
				t_pages_CLOCK_fija* entry;
				pthread_mutex_unlock(&mutex_paginasEnMemoria);

				while (elemento != NULL){
					entry = (t_pages_CLOCK_fija*)elemento->data;

					for(int i = 0; i < MEMORIA_CFG->MARCOS_MAXIMOS; i++){
						t_element comparar;
						comparar.page = NULL;
						comparar.use = 0;
						t_element elemento = entry->pages[i].page == NULL ? comparar : entry->pages[i];

						if(elemento.page != NULL){
							t_page* page = elemento.page;
							MEMORIA[page->memory_frame_number] = *page;
						}
					}
					pthread_mutex_lock(&mutex_paginasEnMemoria);
					elemento = elemento->next;
					pthread_mutex_unlock(&mutex_paginasEnMemoria);
				}
			}
			pthread_mutex_unlock(&mutex_paginasEnMemoria);
		}
	}

	for(int i = 0; i < nro_frames; i++){
		char* estado = "LIBRE";

		if (MEMORIA[i].memory_frame_number != -1)
			estado = "OCUPADO";

		printf("FRAME: %-10d Estado: %-20s Carpincho: %-10d Pagina: %-10d Marco: %-10d\n", i,  estado, MEMORIA[i].idProcess, MEMORIA[i].idPage, MEMORIA[i].memory_frame_number);
	}
}
