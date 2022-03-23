#include "config.h"
#include "kernel.h"


#define MODULENAME "KERNEL"

t_config_kernel* KERNEL_CFG;

t_log* logger;
t_log* logInv;

static t_config_kernel* initialize_cfg() {
    t_config_kernel* cfg = malloc(sizeof(t_config_kernel));
    cfg->ALGORITMO_PLANIFICACION = "SJF";
    cfg->IP_MEMORIA = NULL;
    cfg->IP_ESCUCHA = NULL;
    cfg->DISPOSITIVOS_IO = NULL;
    cfg->DURACIONES_IO = NULL;
    cfg->ESTIMACION_INICIAL = 2000;
    cfg->ALFA = 0.8;
    cfg->GRADO_MULTIPROGRAMACION = 8;
    cfg->GRADO_MULTIPROCESAMIENTO = 3;
    cfg->TIEMPO_DEADLOCK = 20000;
    PROCESOS_TOTALES = 0;
    return cfg;
}

pthread_mutex_t mutexListaIO = PTHREAD_MUTEX_INITIALIZER;

int main(){
	printf("\n Iniciando Kernel \n\n");

    KERNEL_CFG = initialize_cfg();
	logger = log_create(DIR_LOG, "KERNEL", true, LOG_LEVEL_INFO);
    logInv = log_create(DIR_LOG, "KERNEL", false, LOG_LEVEL_TRACE);


    // ****** CARGAR CONFIG  ******
	int memoria_fd;
    if(!cargar_configuracion(KERNEL_CFG) || !generar_conexiones(&memoria_fd, KERNEL_CFG)) {
        cerrar_programa(logger, logInv, KERNEL_CFG);
        return EXIT_FAILURE;
    }


    // ****** CREACION DE PLANIFICADORES ******
    iniciarPlanificadores();


    // ****** CREACION DE HILOS ******
    iniciarHilosCPU();

    // ****** CREACION DE HILO PARA DETECTAR DEADLOCK ******
    pthread_t hiloDeadlock;
    if(!pthread_create(&hiloDeadlock, NULL, (void*) rutinaDeadlock, NULL)){
    	log_info(logger,"Se creó el hilo para detectar y recuperar deadlocks. \n");
    	pthread_detach(hiloDeadlock);
    }


    // ****** CREACION DEL SERVIDOR ******
    char* puerto = string_itoa(KERNEL_CFG->PUERTO_ESCUCHA);
    int kernel_server = iniciar_servidor(logger, "KERNEL_SERVER", KERNEL_CFG->IP_ESCUCHA, puerto);
    free(puerto);

    printf("\n Kernel Iniciado \n\n");

    while (server_escuchar(logger,"KERNEL_SERVER", kernel_server));

    liberar_conexion(&kernel_server);

    cerrar_programa(logger,logInv,KERNEL_CFG);

    return 1;
}

void iniciarPlanificadores(){


	iniciarSemaforos();
	iniciarColas();
	iniciar_mutex();


    pthread_t hiloPlanificadorCortoPlazo;
    if(!pthread_create(&hiloPlanificadorCortoPlazo, NULL, (void*) planificadorCortoPlazo, NULL)){
    	log_info(logger,"Se creó el hilo del planificador de corto plazo.");
    	pthread_detach(hiloPlanificadorCortoPlazo);
    }
    else{
    	log_error(logger,"Error al crear el hilo del planificador de corto plazo");
    }

    pthread_t hiloPlanificadorMedianoPlazo;
    if(!pthread_create(&hiloPlanificadorMedianoPlazo, NULL, (void*) planificadorMedianoPlazo, NULL)){
    	log_info(logger,"Se creó el hilo del planificador de mediano plazo.");
    	pthread_detach(hiloPlanificadorMedianoPlazo);
    }
    else{
    	log_error(logger,"Error al crear el hilo del planificador de mediano plazo");
    }

    pthread_t hiloPlanificadorLargoPlazo;
    if(!pthread_create(&hiloPlanificadorLargoPlazo, NULL, (void*) planificadorLargoPlazo, NULL)){
    	log_info(logger,"Se creó el hilo del planificador de largo plazo. \n");
    	pthread_detach(hiloPlanificadorLargoPlazo);
    }
    else{
    	log_error(logger,"Error al crear el hilo del planificador de largo plazo");
    }
    if(crearEntradasSalidas()){
		usleep(1);// log_info(logger,"Se crearon las entradas salidas correctamente");
	}
	else{
		log_error(logger,"Error al crear las entradas salidas");
	}
    usleep(1);
}


void iniciarHilosCPU(){
	printf("\n");

	int aux = KERNEL_CFG->GRADO_MULTIPROCESAMIENTO;
	int i;

	for(i = 1; i <= aux; i++){

		pthread_t hiloCPU;
		pthread_create(&hiloCPU, NULL, (void*) hiloCpu, NULL);
		log_info(logger,"Se creó un hilo de CPU.");

	}
	printf("\n");
}

bool crearEntradasSalidas(){
	pthread_mutex_lock(&mutexListaIO);
	LIST_IO = list_create();
	pthread_mutex_unlock(&mutexListaIO);
	for(int i = 0; KERNEL_CFG->DISPOSITIVOS_IO[i] != NULL; i++){
		sem_t* semaforo_io = malloc(sizeof(sem_t));
		sem_init(semaforo_io, 0, 0);
		pthread_t threadIO;
		dispositivo_io* dispositivo = malloc(sizeof(dispositivo_io));
		dispositivo->duracion = atoi(KERNEL_CFG->DURACIONES_IO[i]);
		dispositivo->semaforo = semaforo_io;
		dispositivo->recurso = KERNEL_CFG->DISPOSITIVOS_IO[i];
		dispositivo->index = i;
		//pthread_mutex_t aux = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_init(&(dispositivo->mutexLista), NULL);
		//dispositivo->mutexLista = &aux;
		//pthread_mutex_lock(dispositivo->mutexLista);
		dispositivo->listaEspera = list_create();
		//pthread_mutex_unlock(dispositivo->mutexLista);
		pthread_mutex_lock(&mutexListaIO);
		list_add(LIST_IO, dispositivo);;
		pthread_mutex_unlock(&mutexListaIO);
		// log_error(logger, "Se creó el hilo del dispositivo de IO '%s'  id: %d - sem pointer: %d - mutex pointer: %d", dispositivo->recurso, i, semaforo_io, &(dispositivo->mutexLista));
		pthread_create(&threadIO, NULL, (void*) dispositivoIO, i);
		pthread_detach(threadIO);
	}

	return 1;
}

void dispositivoIO(void* args){
	int i = (int)args;
	pthread_mutex_lock(&mutexListaIO);
	if(list_is_empty(LIST_IO)){
		pthread_mutex_unlock(&mutexListaIO);
		log_error(logger, "LIST_IO vacia");
	}
	dispositivo_io* dispositivo = list_get(LIST_IO, i);
	pthread_mutex_unlock(&mutexListaIO);
	log_info(logger, "Recurso %10s conectado - Duracion de IO: %3d", dispositivo->recurso, dispositivo->duracion);

	while(1){
		sem_wait(dispositivo->semaforo);
		pthread_mutex_lock(&(dispositivo->mutexLista));
		if(list_is_empty(dispositivo->listaEspera)){
			pthread_mutex_unlock(&(dispositivo->mutexLista));
			log_error(logger, "Error no hay ningun proceso a la espera de la IO");
		}
		else{
			t_running_thread* carpincho = list_remove(dispositivo->listaEspera, 0);
			pthread_mutex_unlock(&(dispositivo->mutexLista));
			log_info(logger, "El proceso %d está utilizando el dispositivo %s - Hay %d carpinchos en espera.", carpincho->c->pid, dispositivo->recurso, list_size(dispositivo->listaEspera));
			usleep(dispositivo->duracion * 1000);

			carpincho->blocked = true;

			if(carpincho->c->status == BLOCKED){
				bool mismoID(t_running_thread* carp){
					return (carpincho->c->pid == carp->c->pid);
				}

				pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
				list_remove_by_condition(LISTA_BLOQUEADOS, (void*) mismoID);
				pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);


				//pthread_mutex_lock(&(carpincho->c->mutexCarpincho));
				carpincho->c->status = READY; // VALGRIND MARCA RACE CONDITION
				//pthread_mutex_unlock(&(carpincho->c->mutexCarpincho));
				add_lista_ready(carpincho);
				log_info(logger, "El proceso %d dejó de utilizar el dispositivo %s y pasó a READY", carpincho->c->pid, dispositivo->recurso);
				sem_post(&carpinchosEnReady);
			}
			else if(carpincho->c->status == SUSPENDED){
				list_add(LISTA_SUSPENDIDOS_READY, carpincho);
				log_info(logger, "El proceso %d dejó de utilizar el dispositivo %s y pasó a SUSPENDIDO-LISTO", carpincho->c->pid, dispositivo->recurso);
				sem_post(&carpinchosEnNew);
			}



			//send_R_call_io(carpincho->socketMateLib);

		}
	}
}







