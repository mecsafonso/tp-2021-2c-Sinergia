#include "comunicacion.h"

pthread_mutex_t mutex_RESERVAR_FRAME_SWAMP= PTHREAD_MUTEX_INITIALIZER;


typedef struct {
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

static void procesar_conexion(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    t_log* logger = args->log;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);

    op_code cop;
    bool status;
    while (cliente_socket != -1) {

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }
        int id_proceso;

        switch (cop) {
        	case CREAR_PROCESO:;
        		recv_crear_proceso(cliente_socket, &id_proceso);
        		log_info(logger, "CREAR PROCESO - PID %d", id_proceso);

        		//pthread_mutex_lock(&mutex_RESERVAR_FRAME_SWAMP);
        		status = crear_proceso(cliente_socket, &id_proceso);
        		//pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);

        		if(status)
        			log_warning(logger, "proceso No: %d, creado exitosamente", id_proceso);
        		else
        			log_error(logger, "proceso No: %d, creacion fallida", id_proceso);
        		break;

        	case MEM_ALLOC:;

        		uint32_t size;
        		int add;
        		recv_mem_alloc(cliente_socket, &size, &id_proceso);
        	    log_info(logger, "MEM ALLOC - PID %d, size %d", id_proceso, size);

        	    //pthread_mutex_lock(&mutex_RESERVAR_FRAME_SWAMP);

        	    status = mem_alloc(cliente_socket, id_proceso, size, &add);
        	    //pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);

				if(status)
					log_info(logger, " Espacio reservado con exito [PID: %d | DIR_LOG: %d - %d | SIZE: %d]", id_proceso, add, add + size - 1, size);
				else
					log_error(logger, "Reserva de espacio fallida  [PID: %d]", id_proceso);

				//TODO: borrar
				//print_memoria();
        		break;

        	case MEM_FREE:;

        		int32_t addr;

        		recv_mem_free(cliente_socket, &addr, &id_proceso);
				log_info(logger, "MEM FREE - PID %d, addr %d", id_proceso, addr);

				status = mem_free(cliente_socket, id_proceso, addr);

				if(status)
					log_info(logger, "Memoria liberada con exito [PID: %d]", id_proceso);
				else
					log_error(logger, "Fallo en liberar Memoria  [PID: %d]", id_proceso);


        		break;

        	case MEM_READ:;

        		int32_t origin;
        		int sizeToRead;

        		recv_mem_read(cliente_socket, &origin, &sizeToRead, &id_proceso);
				log_info(logger, "MEM READ - PID %d, ptro %d, sizeToRead %d", id_proceso, origin, sizeToRead);

				status = mem_read(cliente_socket, id_proceso, origin, sizeToRead);

				if(status)
					log_info(logger, "Lectura de Memoria Exitosa [PID: %d | DIR_LOG: %d - %d | SIZE: %d]", id_proceso, origin, origin + sizeToRead - 1, sizeToRead);
				else
					log_error(logger, "Fallo en lectura de Memoria, [PID: %d]", id_proceso);

				break;

        	case MEM_WRITE:;

        		void* data;
				int size_To_Read;
				int32_t dest;

				recv_mem_write(cliente_socket, &data, &dest, &size_To_Read, &id_proceso);
				log_info(logger, "MEM WRITE - PID %d, size %d, ptro %d, data %s", id_proceso, size_To_Read, dest, (char*)data);

				status = mem_write(cliente_socket, id_proceso, data, dest, size_To_Read);

				if(status)
					log_info(logger, "Escritura en Memoria Exitosa [PID: %d | DIR_LOG: %d - %d | SIZE: %d]", id_proceso, dest, dest + size_To_Read - 1, size_To_Read);
				else
					log_error(logger, "Fallo en lectura de Memoria [PID: %d]", id_proceso);

				free(data);

        		break;

        	case SUSPENDER_PROCESO:
        		recv_suspender_proceso(cliente_socket, &id_proceso);
				log_info(logger, "SUSPENDER PROCESO - PID %d", id_proceso);

				status = suspender_proceso(cliente_socket, id_proceso);

				if(status)
					log_warning(logger, "proceso No: %d, suspendido exitosamente", id_proceso);
				else
					log_error(logger, "proceso No: %d, suspencion fallida", id_proceso);
        		break;

        	case FINALIZAR_PROCESO:;
        		recv_finalizar_proceso(cliente_socket, &id_proceso);
        		log_info(logger, "FINALIZAR PROCESO - PID %d", id_proceso);

				status = finalizar_proceso(cliente_socket, id_proceso);

				if(status)
					log_warning(logger, "proceso No: %d, finalizado exitosamente", id_proceso);
				else
					log_error(logger, "proceso No: %d, finalizacion fallida", id_proceso);

				log_info(logger, "desconexion de cliente Proceso No: %d", id_proceso);

				//TODO: borrar
					// print_memoria();

        		break;

        	// Errores
            case -1:
                log_error(logger, "Cliente desconectado de %s...", server_name);
                return;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}

int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}


bool crear_proceso(int cliente_socket, int* id_proceso){
	//se verifica si el proceso ya esta creado
	if(*id_proceso == 0){
		pthread_mutex_lock(&mutex_VGID);
		*id_proceso = VGID;
		VGID++;
		pthread_mutex_unlock(&mutex_VGID);
	}
	else{
		if(esta_proceso_en_TGP(*id_proceso)){
			R_send_crear_proceso(cliente_socket, ERROR_MEMORIA);
			close(cliente_socket);
			return false;
		}
	}

	log_info(LOGGER, "conexion de cliente Proceso No: %d", *id_proceso);


	pthread_mutex_lock(&mutex_RESERVAR_FRAME_SWAMP);
		if(espacio_disponible(*id_proceso, 1) != 1){
			log_warning(LOGGER, "MEMORIA - no hay espacio disponible");
			R_send_crear_proceso(cliente_socket, ERROR_SWAMP_U_OTRO);
			close(cliente_socket);
			pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);
			return false;
		}


		//SE CREA LA TABLA DE PAGINAS
		t_pageTable* new_TP = malloc(sizeof(*new_TP));
		new_TP->idProcess = *id_proceso;
		new_TP->pages = list_create();
		pthread_mutex_init(&new_TP->mutex_allocs, NULL);

		pthread_mutex_lock(&mutex_TGP);
		list_add(TGP, (void*)new_TP);
		pthread_mutex_unlock(&mutex_TGP);
		log_info(LOGGER,"Nueva tabla de paginas creada para PROCESO: %d", *id_proceso);


		//CREAR entrada en la tabla de metricas de la TLB
		if(MEMORIA_CFG->CANTIDAD_ENTRADAS_TLB != 0){
			t_metrics_tlb* entry = malloc(sizeof(*entry));
			entry->id_process = *id_proceso;
			entry->hit = 0;
			entry->miss = 0;

			pthread_mutex_lock(&mutex_METRICS_TLB);
			list_add(TLB_METRICS, (void*)entry);
			pthread_mutex_unlock(&mutex_METRICS_TLB);

			log_info(LOGGER,"Nueva entrada en TLB METRICS creada para PROCESO: %d", *id_proceso);
		}

		//Creacion de nueva pagina
		t_page* new_page = crear_pagina_nueva(new_TP, *id_proceso);
	pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);

	//SE CREA EL HEAPMETADATA Y SE GUARDA EN MEMORIA
	heapMetadata new_metadata;
	new_metadata.prevAlloc = -1;
	new_metadata.nextAlloc = -1;
	new_metadata.isFree = true;

	int offset = new_page->memory_frame_number * MEMORIA_CFG->TAMANIO_PAGINA;
	pthread_mutex_lock(&mutex_MEMORIA);
	memcpy(MEMORY + offset, &new_metadata.prevAlloc, sizeof(uint32_t));
	memcpy(MEMORY + offset + sizeof(uint32_t), &new_metadata.nextAlloc, sizeof(uint32_t));
	memcpy(MEMORY + offset + (sizeof(uint32_t) * 2), &new_metadata.isFree, sizeof(uint8_t));
	pthread_mutex_unlock(&mutex_MEMORIA);

	//cargar a la TLB
	cargar_pagina_TLB(new_page);

	return R_send_crear_proceso(cliente_socket, *id_proceso);
}


bool mem_alloc(int cliente_socket, int id_proceso, uint32_t size_reserve, int* dir_logica){
	//buscar tabla de pagina del proceso
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	*dir_logica = 0;
	heapMetadata* hpmtd_actual = malloc(sizeof(*hpmtd_actual));

	//consultar si hay alloc disponible TODO:REVISAR ajustar size de alloc
	if(alloc_disponible(TP, size_reserve, hpmtd_actual, dir_logica)){
		pthread_mutex_lock(&TP->mutex_allocs);
		int size_alloc = hpmtd_actual->nextAlloc != -1? hpmtd_actual->nextAlloc - (*dir_logica + SIZE_HEAPMETADATA) : (list_size(TP->pages) *  MEMORIA_CFG->TAMANIO_PAGINA) - (*dir_logica + SIZE_HEAPMETADATA);
		pthread_mutex_unlock(&TP->mutex_allocs);

		if(size_alloc != size_reserve)
			ajustar_size_de_alloc(TP, size_reserve, hpmtd_actual, *dir_logica);

		free(hpmtd_actual);
		*dir_logica += SIZE_HEAPMETADATA;
		return send_R_mem_alloc(cliente_socket, *dir_logica);
	}
	else{
		//calcular el espacio libre del ultimo heapmetadata
		pthread_mutex_lock(&TP->mutex_allocs);
		int free_space = list_size(TP->pages) * MEMORIA_CFG->TAMANIO_PAGINA - (*dir_logica + SIZE_HEAPMETADATA);
		pthread_mutex_unlock(&TP->mutex_allocs);

		//calcular cant de pages a reservar
		int remainder = (size_reserve + SIZE_HEAPMETADATA + 1) - free_space;
		int cant_pages = remainder / MEMORIA_CFG->TAMANIO_PAGINA;

		if(remainder % MEMORIA_CFG->TAMANIO_PAGINA != 0)
			cant_pages += 1;

		pthread_mutex_lock(&mutex_RESERVAR_FRAME_SWAMP);
			//consultar si swamp tiene frame(s) libre(s)
			if(encontrar_frame_libre_swamp(id_proceso, cant_pages)){
				//crear nueva(s) pagina(s)
				for(int i = 1; i <= cant_pages; i++){
					crear_pagina_nueva(TP, id_proceso);

					if(i == cant_pages){
						//crear heap del ESPACIO LIBRE que queda
						heapMetadata* new_hpmtd = malloc(sizeof(*new_hpmtd));
						new_hpmtd->isFree = true;
						new_hpmtd->nextAlloc = hpmtd_actual->nextAlloc;
						new_hpmtd->prevAlloc = *dir_logica;


						//actualizar heapmetadata de alloc a reservar
						hpmtd_actual->nextAlloc = *dir_logica + SIZE_HEAPMETADATA + size_reserve;
						hpmtd_actual->isFree = false;
						guardar_data_en_memoria(TP, SIZE_HEAPMETADATA, (void*)hpmtd_actual, *dir_logica);

						*dir_logica = hpmtd_actual->nextAlloc;
						guardar_data_en_memoria(TP, SIZE_HEAPMETADATA, (void*)new_hpmtd, *dir_logica);

						free(new_hpmtd);
					}
				}

				free(hpmtd_actual);
				*dir_logica -= size_reserve;
				send_R_mem_alloc(cliente_socket, *dir_logica);
				pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);
				return  true;
			}
		pthread_mutex_unlock(&mutex_RESERVAR_FRAME_SWAMP);

		send_R_mem_alloc(cliente_socket, ERROR_MEMORIA);
	}
	return false;
}


bool mem_free(int cliente_socket, int id_proceso, int32_t dir_logica){
	//buscar tabla de pagina del proceso
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	heapMetadata* heapmetadata = malloc(sizeof(*heapmetadata));
	int dir_log_heap = 0;

	//verificar que dir_logica, pertenezca al bloque de direcciones del alloc
	if (validar_dir_logica(TP, dir_logica, &dir_log_heap, heapmetadata)){
		log_info(LOGGER, "PID: %d, Dir Logica VALIDA: %d, RANGO [ %d - %d]", id_proceso, dir_logica, dir_log_heap + SIZE_HEAPMETADATA, heapmetadata->nextAlloc - 1);
	}

	//verificar que la dir log, sea la sgte al alloc
	if(dir_log_heap + SIZE_HEAPMETADATA == dir_logica){
		free_alloc(TP, dir_logica, heapmetadata);
		send_R_mem_free(cliente_socket, true);
		free(heapmetadata);
		return true;
	}

	send_R_mem_free(cliente_socket, MATE_FREE_FAULT);
	return false;
}


bool mem_read(int cliente_socket, int id_proceso, int32_t dir_logica, int sizeToRead){
	//buscar tabla de pagina del proceso
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	heapMetadata* heapmetadata = malloc(sizeof(*heapmetadata));
	int dir_log_heap = 0;

	//verificar que dir_logica, pertenezca al bloque de direcciones del alloc
	if (validar_dir_logica(TP, dir_logica, &dir_log_heap, heapmetadata)){
		pthread_mutex_lock(&TP->mutex_allocs);
		int total_dir = list_size(TP->pages) * MEMORIA_CFG->TAMANIO_PAGINA;
		pthread_mutex_unlock(&TP->mutex_allocs);

		log_info(LOGGER, "PID: %d, Dir Logica VALIDA: %d, RANGO [ %d - %d]", id_proceso, dir_logica, dir_log_heap + SIZE_HEAPMETADATA, heapmetadata->nextAlloc != - 1? heapmetadata->nextAlloc -1 : total_dir -1 );
	}

	//verificar que bloque a leer pertenece al alloc
	if(dir_logica + sizeToRead -1 < heapmetadata->nextAlloc){
		void* data = malloc(sizeToRead);
		traer_data_de_memoria(TP, sizeToRead, data, dir_logica);
		send_R_mem_read(cliente_socket, data, sizeToRead);

		free(data);
		free(heapmetadata);
		return true;
	}

	//ERROR => posicion de lectura de memoria erronea
	log_info(LOGGER, "Acceso invalido a MEMORY [PID: %d, DIR_LOG : %d]", id_proceso, heapmetadata->nextAlloc);
	send_R_mem_read(cliente_socket, (void*)MATE_READ_FAULT , sizeof(int));
	free(heapmetadata);
	return false;
}


bool mem_write(int cliente_socket, int id_proceso, void* data, int32_t dir_logica, int sizeToRead){
	//buscar tabla de pagina del proceso
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	heapMetadata* heapmetadata = malloc(sizeof(*heapmetadata));
	int dir_log_heap = 0;

	//verificar que dir_logica, pertenezca al bloque de direcciones del alloc
	if (validar_dir_logica(TP, dir_logica, &dir_log_heap, heapmetadata)){
		pthread_mutex_lock(&TP->mutex_allocs);
		int total_dir = list_size(TP->pages) * MEMORIA_CFG->TAMANIO_PAGINA;
		pthread_mutex_unlock(&TP->mutex_allocs);

		log_info(LOGGER, "PID: %d, Dir Logica VALIDA: %d, RANGO [ %d - %d]",id_proceso, dir_logica, dir_log_heap + SIZE_HEAPMETADATA, heapmetadata->nextAlloc != - 1? heapmetadata->nextAlloc -1 : total_dir -1 );
	}

	//verificar que bloque a escribir pertenece al alloc
	if(dir_logica + sizeToRead - 1 < heapmetadata->nextAlloc){
		guardar_data_en_memoria(TP, sizeToRead, data, dir_logica);
		send_R_mem_write(cliente_socket, true);

		free(heapmetadata);
		return true;
	}

	//ERROR => posicion de escritura de memoria erronea
	log_info(LOGGER, "Acceso invalido a MEMORY [PID: %d, DIR_LOG : %d]", id_proceso, heapmetadata->nextAlloc);
	send_R_mem_write(cliente_socket, MATE_WRITE_FAULT);
	free(heapmetadata);
	return false;
}


bool suspender_proceso(int cliente_socket, int id_proceso){
	//buscar tabla de pagina del proceso
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	pthread_mutex_lock(&TP->mutex_allocs);
	t_link_element* elemento_page = TP->pages->head;
	pthread_mutex_unlock(&TP->mutex_allocs);
	t_page* page;

	int dir_logica = 0;

	//enviar a swamp las paginas
	while (elemento_page != NULL){
		page = (t_page*)elemento_page->data;

		//consultar si la pagina esta en memoria
		if(page->memory_frame_number != -1){
			void* data = malloc(MEMORIA_CFG->TAMANIO_PAGINA);
			traer_data_de_memoria(TP, MEMORIA_CFG->TAMANIO_PAGINA, data, dir_logica);

			guardar_en_swamp(id_proceso, page->swap_frame_number, data);
			log_info(LOGGER, "Data enviada a swamp [PID: %d, PAG: %d]", id_proceso, page->idPage);

			//actualizar flag del frame en bitmap de memoria
			bitarray_clean_bit(MEMORY_BITMAP, page->memory_frame_number);

			//libera el frame en memoria
			page->memory_frame_number = -1;
			page->modified = false;

			free(data);
		}
		else {
			log_info(LOGGER, "[PID: %d, PAG: %d] pagina no está en memoria, no proceso SWAP", id_proceso, page->idPage);
		}
		pthread_mutex_lock(&TP->mutex_allocs);
		elemento_page = elemento_page->next;
		pthread_mutex_unlock(&TP->mutex_allocs);

		dir_logica += MEMORIA_CFG->TAMANIO_PAGINA;
	}

	send_R_suspender_proceso(cliente_socket, true);

	return true;
}


bool finalizar_proceso(int cliente_socket, int id_proceso){
	t_pageTable* TP = get_tabla_de_paginas(id_proceso);

	pthread_mutex_lock(&TP->mutex_allocs);
	t_link_element* elemento_page = TP->pages->head;
	pthread_mutex_unlock(&TP->mutex_allocs);
	t_page* page;

	//enviar a swamp las paginas
	while (elemento_page != NULL){
		page = (t_page*)elemento_page->data;

		//consultar si la pagina esta en memoria
		if(page->swap_frame_number != -1){
			free_frame_swamp(id_proceso, page->swap_frame_number);
			log_info(LOGGER, "free frame en SWAMP [PID: %d, PAG: %d, FRAME: %d]", id_proceso, page->idPage, page->swap_frame_number);

			eliminar_entrada_en_tlb(page);
			page->swap_frame_number = -1;
		}

		pthread_mutex_lock(&TP->mutex_allocs);
		elemento_page = elemento_page->next;
		pthread_mutex_unlock(&TP->mutex_allocs);
	}
/*	pthread_mutex_lock(&mutex_TGP);
	t_pageTable* TP = list_remove(TGP, indice_TP);
	pthread_mutex_unlock(&mutex_TGP);

	//elimino las paginas en memoria y swamp
	pthread_mutex_lock(&TP->mutex_allocs);
	while (!list_is_empty(TP->pages)){
		t_page* page = (t_page*)list_remove(TP->pages, list_size(TP->pages) - 1);

		//cosultar page en TLB
		int frame = consultar_frame(TP, page);

		//consultar si la pagina esta en memoria
		if(frame != -1){
			free_frame_memoria(frame);

			actualizar_algoritmo_reemplazo_FIJA(page);

			//actualizar flag del frame en bitmap de memoria
			bitarray_clean_bit(MEMORY_BITMAP,frame);

			log_info(LOGGER, "free frame en MEMORIA [PID: %d, PAG: %d, FRAME: %d]", id_proceso, page->idPage, page->memory_frame_number);
		}

		free_frame_swamp(id_proceso, page->swap_frame_number);
		log_info(LOGGER, "free frame en SWAMP [PID: %d, PAG: %d, FRAME: %d]", id_proceso, page->idPage, page->swap_frame_number);

		eliminar_entrada_en_tlb(page);
		free(page);
	}
	pthread_mutex_unlock(&TP->mutex_allocs);

	//se liberan las estructuras para ASIG. FIJA
	if(strcmp(MEMORIA_CFG->TIPO_ASIGNACION, "FIJA") == 0){
		//actualizar_algoritmo_reemplazo_FIJA(page, frame); // TODO: terminar
		liberar_estructuras_admin_FIJA(id_proceso);
	}


	pthread_mutex_lock(&TP->mutex_allocs);
	//limpio la TP del proceso
	list_clean(TP->pages);
	pthread_mutex_unlock(&TP->mutex_allocs);

	free(TP);*/


	//enviar finalizar proceso a swamp
	pthread_mutex_lock(&mutex_puertoSWAMP);
	bool status = send_finalizar_proceso(FD_SWAMP, id_proceso);
	pthread_mutex_unlock(&mutex_puertoSWAMP);

	return status;
}
