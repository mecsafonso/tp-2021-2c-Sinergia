#include "comunicacion.h"


typedef struct {
    t_log* log;
    int fd;
    char* server_name;
} t_procesar_conexion_args;

static void desearilizar_pagina_recibida(void* stream, uint32_t* pagina){				//ver si se puede poner en un lugar mas bonito :)
	memcpy(pagina, stream, sizeof(uint32_t));
}


static void procesar_conexion_indefinido(void* void_args){
	t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
	int cliente_socket = args->fd;
	free(args);

	op_code cop;


	while (cliente_socket != -1) {

		usleep(SWAMP_CFG->RETARDO_SWAP*1000);

		if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
			log_info(log_swamp, "DISCONNECT!");
			return;
		}


		if(cop == SWAMP_READY){

			char* asignacion;

			if(!recv_swamp_ready(cliente_socket, &asignacion)){
				log_error(log_swamp, "fallo el llegado de tipo de asignacion");
			}
			log_warning(log_swamp, "ha llegado el tipo de asignacion: %s", asignacion);

			SWAMP_CFG->TIPO_ASIGNACION = asignacion;

			send_R_swamp_ready(cliente_socket, 1);
			return;
		}

	}

}


static void procesar_conexion_dinamica(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);
    int pid;

    op_code cop;
    int page;

    while (cliente_socket != -1) {
    	usleep(SWAMP_CFG->RETARDO_SWAP*1000);
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(log_swamp, "DISCONNECT!");
            return;
        }

        switch (cop){
        	case SWAMP_READY:; //TODO: verrificacion si swamp tiene todo listo para arrancar con memoria a la par
        		log_info(log_swamp, "swamp esta ready?");
        		char* tipoAsignacion;
        		recv_swamp_ready(cliente_socket, &tipoAsignacion);
        		if(strcmp(SWAMP_CFG->TIPO_ASIGNACION, tipoAsignacion) == 0){
					log_info(log_swamp, "tipo de asignacion %s", tipoAsignacion);
					send_R_swamp_ready(cliente_socket, 1);
        		}
        		else{
        			log_error(log_swamp, "tipo de asignacion incorrecta. ACTUAL: %s  -  RECIBIDA: %s", SWAMP_CFG->TIPO_ASIGNACION, tipoAsignacion);
					send_R_swamp_ready(cliente_socket, 0);
        		}
        		break;



        	case CONSULTAR_FRAME_LIBRE_SWAMP:;

				int reserva;
				if(!recv_consultar_frame_libre_swamp(cliente_socket, &pid, &reserva)){//recv_consultar_frame_libre_swamp(int fd, int* id_process, int* reserva)
				   log_error(log_swamp, "fallo el pedido de frame libre");
				}
				log_info(log_swamp, "ha llegado un pedido de frame libre para el proceso %d", pid);

				int cantidad_disponible;

				if(!existeProceso(pid)){
					if(!crearProceso_D(pid)){
						send_R_consultar_frame_libre_swamp(cliente_socket, false);
						break;
					}
				}
				cantidad_disponible = buscarFramesDisponibles_D(pid);

				if(cantidad_disponible >= reserva){
					send_R_consultar_frame_libre_swamp(cliente_socket, true);
				}
				else{
					send_R_consultar_frame_libre_swamp(cliente_socket, false);
				}

				break;



        	case RESERVAR_FRAME_LIBRE_SWAMP:;
        	int success;
        		if(!recv_reservar_frame_libre_swamp(cliente_socket, &pid, &success)){
        			log_error(log_swamp, "fallo el pedido de reserva de frame libre");
        		}
        		log_warning(log_swamp, "RESERVAR_FRAME_LIBRE_SWAMP - [PID: %d]", pid);

        		int valor_frame;

				if(!existeProceso(pid)){
					if(!crearProceso_D(pid)){
						send_R_reservar_frame_libre_swamp(cliente_socket, -1);
						break;
					}
				}
				valor_frame = asignarEspacio(pid);
				if(valor_frame >= 0){
					send_R_reservar_frame_libre_swamp(cliente_socket, valor_frame);
				}else{
					send_R_reservar_frame_libre_swamp(cliente_socket, -1);
				}

        		break;


        	case FREE_FRAME_SWAMP:;

				if(!recv_free_frame_swamp(cliente_socket, &pid, &page)){
					log_error(log_swamp, "fallo el pedido de free swamp");
				}
				page = liberarPagina_D(pid, page);
				if(page >= 0){
					send_R_free_frame_swamp(cliente_socket, true);
				}
				else{
					send_R_free_frame_swamp(cliente_socket, false);
				}

				break;

        	case GUARDAR_EN_SWAMP:;
				void* datosAGuardar;
				int size_datosAGuardar;
				if(!recv_guardar_en_swamp(cliente_socket, &pid, &page, &datosAGuardar, &size_datosAGuardar)){
					log_error(log_swamp, "fallo el pedido de guardar en swamp");
				}
				if(!checkearFrame(pid, page)){
					send_R_guardar_en_swamp(cliente_socket, -1);
					break;
				}
				log_info(log_swamp, "se guardó la informacion correctamente");
				escribirFrame(page, datosAGuardar, size_datosAGuardar);
				send_R_guardar_en_swamp(cliente_socket, page); //Envio la página si todo_ sale bien

				free(datosAGuardar);
				break;



        	case PEDIR_PAGE_A_SWAMP:;
        		if(!recv_pedir_page_a_swamp(cliente_socket, &pid, &page)){
					log_error(log_swamp, "fallo el pedido de lectura");
				}
        		if(!checkearFrame(pid, page)){
					send_R_guardar_en_swamp(cliente_socket, -1);
					break;
				}

        		void* dest = devolverFrame(page);
        		send_R_pedir_page_a_swamp(cliente_socket, dest, SWAMP_CFG->TAMANIO_PAGINA);
        		free(dest);
        		break;


        	//de aca para abajo en duda su continuidad en Switch F.C          nutdea


            case FINALIZAR_PROCESO:;
            	if(!recv_finalizar_proceso(cliente_socket, &pid)){
					log_error(log_swamp, "fallo el pedido para eliminar un proceso");
				}
            	log_info(log_swamp, "ha llegado un pedido para eliminar el proceso %d", pid);

            	if(!existeProceso(pid)){
            		log_info(log_swamp, "El proceso %d no existía", pid);
            		break;
				}
            	borrarEstructurasProcesos_D(pid);

            	break;

            // case  MENSAJE: break;

            // Errores
            case -1:;
                log_error(log_swamp, "Cliente desconectado de %s...", server_name);
                return;
            default:;
                log_error(log_swamp, "Algo anduvo mal en el server de %s", server_name);
                log_info(log_swamp, "Cop: %d", cop);
                return;
        }

    }

    log_warning(log_swamp, "El cliente se desconecto de %s server", server_name);
    return;
}



static void procesar_conexion_fija(void* void_args) {
    t_procesar_conexion_args* args = (t_procesar_conexion_args*) void_args;
    int cliente_socket = args->fd;
    char* server_name = args->server_name;
    free(args);
    int pid;

    op_code cop;
    uint32_t pagina;
    int page;

    while (cliente_socket != -1) {

    	usleep(SWAMP_CFG->RETARDO_SWAP*1000);

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(log_swamp, "DISCONNECT!");
            return;
        }

        switch (cop){
        	case SWAMP_READY:; //TODO: verrificacion si swamp tiene todo listo para arrancar con memoria a la par
				log_info(log_swamp, "swamp esta ready?");
				char* tipoAsignacion;
				recv_swamp_ready(cliente_socket, &tipoAsignacion);
				if(strcmp(SWAMP_CFG->TIPO_ASIGNACION, tipoAsignacion) == 0){
					log_info(log_swamp, "tipo de asignacion %s", tipoAsignacion);
					send_R_swamp_ready(cliente_socket, 1);
				}
				else{
					log_error(log_swamp, "tipo de asignacion incorrecta. ACTUAL: %s  -  RECIBIDA: %s", SWAMP_CFG->TIPO_ASIGNACION, tipoAsignacion);
					send_R_swamp_ready(cliente_socket, 0);
				}
				break;

        	case CONSULTAR_FRAME_LIBRE_SWAMP:;
				log_info(log_swamp, "ha llegado un pedido de frame libre");
				int reserva;
				if(!recv_consultar_frame_libre_swamp(cliente_socket, &pid, &reserva)){//recv_consultar_frame_libre_swamp(int fd, int* id_process, int* reserva)
				   log_error(log_swamp, "fallo el pedido de frame libre");
				}

				int cantidad_disponible;

				if(!existeProceso(pid)){
					if(!crearProceso_F(pid)){
						send_R_consultar_frame_libre_swamp(cliente_socket, false);
						break;
					}
					cantidad_disponible = SWAMP_CFG->MARCOS_MAXIMOS;
				}
				else{
					cantidad_disponible = buscarFramesDisponibles_F(pid);//buscarFramesDisponibles_D(pid);
				}

				if(cantidad_disponible >= reserva){
					send_R_consultar_frame_libre_swamp(cliente_socket, true);
				}
				else{
					send_R_consultar_frame_libre_swamp(cliente_socket, false);
				}



				break;

        	case RESERVAR_FRAME_LIBRE_SWAMP:;
        		int success;
        		if(!recv_reservar_frame_libre_swamp(cliente_socket, &pid, &success)){
        			log_error(log_swamp, "fallo el pedido de reserva de frame libre");
        		}
        		int valor_frame;
        		log_warning(log_swamp, "RESERVAR_FRAME_LIBRE_SWAMP - [PID: %d]", pid);
				if(!existeProceso(pid)){
					if(!crearProceso_F(pid)){
						send_R_reservar_frame_libre_swamp(cliente_socket, false);
						break;
					}
				}
				valor_frame = asignarEspacio_F(pid);
				if(valor_frame >= 0){
					send_R_reservar_frame_libre_swamp(cliente_socket, valor_frame);
				}else{
					send_R_reservar_frame_libre_swamp(cliente_socket, -2);
				}

        		break;


        	case FREE_FRAME_SWAMP:;

				if(!recv_free_frame_swamp(cliente_socket, &pid, &page)){
					log_error(log_swamp, "fallo el pedido de free swamp");
				}
				page = liberarPagina_F(pid, page);
				if(page >= 0){
					send_R_free_frame_swamp(cliente_socket, true);
				}
				else{
					send_R_free_frame_swamp(cliente_socket, false);
				}

				break;

        	case GUARDAR_EN_SWAMP:;
				void* datosAGuardar;
				int size_datosAGuardar;
				if(!recv_guardar_en_swamp(cliente_socket, &pid, &page, &datosAGuardar, &size_datosAGuardar)){
					log_error(log_swamp, "fallo el pedido de lectura");
				}
				if(!checkearFrame_F(pid, page)){
					send_R_guardar_en_swamp(cliente_socket, -2);
					break;
				}
				escribirFrame(page, datosAGuardar, size_datosAGuardar);
				send_R_guardar_en_swamp(cliente_socket, page); //Envio la página si todo_ sale bien

				free(datosAGuardar);
				break;


        	case PEDIR_PAGE_A_SWAMP:;
        		if(!recv_pedir_page_a_swamp(cliente_socket, &pid, &page)){
					log_error(log_swamp, "fallo el pedido de lectura");
				}
        		if(!checkearFrame_F(pid, page)){
					send_R_guardar_en_swamp(cliente_socket, -2);
					break;
				}

        		void* dest = devolverFrame(page);
        		send_R_pedir_page_a_swamp(cliente_socket, dest, SWAMP_CFG->TAMANIO_PAGINA);
        		free(dest);
        		break;


            case FINALIZAR_PROCESO:;
            	log_info(log_swamp, "ha llegado un pedido para eliminar un proceso");

            	if(!recv_finalizar_proceso(cliente_socket, &pid)){
					log_error(log_swamp, "fallo el pedido para eliminar un proceso");
					break;
				}

            	if(!existeProceso(pid)){
            		log_info(log_swamp, "El proceso no existía");
            		break;
				}
            	borrarEstructurasProcesos_F(pid);

            	break;

            // case  MENSAJE: break;

            // Errores
            case -1:;
                log_error(log_swamp, "Cliente desconectado de %s...", server_name);
                return;
            default:;
                log_error(log_swamp, "Algo anduvo mal en el server de %s", server_name);
                log_info(log_swamp, "Cop: %d", cop);
                return;
        }

    }

    log_warning(log_swamp, "El cliente se desconecto de %s server", server_name);
    return;
}

int server_escuchar(t_log* logger, char* server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        //pthread_t hilo;
        t_procesar_conexion_args* args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;

        if(strcmp(SWAMP_CFG->TIPO_ASIGNACION,"FIJA") == 0){
        	procesar_conexion_fija(args);
        }
        else if(strcmp(SWAMP_CFG->TIPO_ASIGNACION,"DINAMICA") == 0){
        	procesar_conexion_dinamica(args);
        }
        else{
        	procesar_conexion_indefinido(args);
        }

        //pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);			//aca no deberiamos usar hilos porque usa un unico socket.
        //pthread_detach(hilo);
        close(cliente_socket);
        return 1;
    }
    return 0;
}



bool recv_pagina(int cliente_socket, uint32_t* pagina){
	size_t size = sizeof(uint32_t);
	void* stream = malloc(size);

	if(recv(cliente_socket,stream, size, 0) != size){
		free(stream);
		return false;
	}

	desearilizar_pagina_recibida(stream, pagina);

	free(stream);
	return true;
}



