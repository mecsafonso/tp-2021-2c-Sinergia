/*
 ============================================================================
 Name        : carpinchoTest.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "matelib.h"
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
//#include <lib/matelib.h>
#include <commons/log.h>
#include <semaphore.h>



void *testKernel1(){
	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
	mate_instance mate_ref;
	mate_init(&mate_ref, config_path);

	/*mate_sem_wait(&mate_ref, "SEM2");
	printf("test kernel1 paso el wait al sem 2\n");
	mate_sem_post(&mate_ref, "SEM1");*/
	//sleep(3);
	printf("antes de los post\n");
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	sleep(1);
	mate_sem_post(&mate_ref, "SEM2");
	printf("luego de los post\n");
	mate_close(&mate_ref);

	return 0;
}

void *testKernel2(){
	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
	mate_instance mate_ref;
	mate_init(&mate_ref, config_path);
/*
	mate_sem_wait(&mate_ref, "SEM1");
	printf("test kernel2 paso el wait al sem 1\n");
	sleep(3);
	mate_sem_post(&mate_ref, "SEM2");
	printf("hago el post al test 1\n");
	mate_sem_wait(&mate_ref, "SEM1");
	printf("test kernel2 paso el wait al sem 1\n");*/
	printf("antes de los waits\n");
	mate_sem_wait(&mate_ref, "SEM2");
	mate_sem_wait(&mate_ref, "SEM2");
	mate_sem_wait(&mate_ref, "SEM2");
	mate_sem_wait(&mate_ref, "SEM2");
	mate_sem_wait(&mate_ref, "SEM2");
	mate_sem_wait(&mate_ref, "SEM2");
	printf("queda 1 wait\n");
	mate_sem_wait(&mate_ref, "SEM2");
	printf("luego de los waits\n");
	return 0;
}

void *testKernelAIO(char* recurso){
	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
	mate_instance mate_ref;
	mate_init(&mate_ref, config_path);

	mate_call_io(&mate_ref, recurso, "Un mensaje random para molestar");

	mate_close(&mate_ref);

	printf("un carpincho se fue a IO");

	return 0;

}

void prueba_io_kernel(){
//	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
//	mate_instance mate_ref;
//	mate_init(&mate_ref, config_path);
//
//
//	mate_instance mate_ref2;
//	mate_init(&mate_ref2, config_path);
//
//
//	if(mate_sem_init(&mate_ref2, "SEM2", 1)){
//		printf("El semaforo se creó correctamente \n");
//	}else{
//		printf("fallo la creacion del semaforo \n");
//	}
//	mate_sem_post(&mate_ref2, "SEM2");
//
//	mate_sem_wait(&mate_ref2, "SEM2");
//	printf("\nPrimer semwait\n");
//	mate_sem_wait(&mate_ref2, "SEM2");
//	printf("\nSegundo semwait\n");
//	if(mate_sem_init(&mate_ref, "SEM1", 1)){
//		printf("El semaforo se creó correctamente \n");
//	}else{
//		printf("fallo la creacion del semaforo \n");
//	}
//
//	mate_close(&mate_ref2);
//	mate_close(&mate_ref);

	//pthread_t carpincho_th_1;
	pthread_t carpincho_th_1;
	pthread_t carpincho_th_2;
	pthread_t carpincho_th_3;
	pthread_t carpincho_th_4;
	pthread_t carpincho_th_5;
	pthread_t carpincho_th_6;

	//pthread_create(&carpincho_th_1, NULL, &testKernel1, NULL);
	pthread_create(&carpincho_th_1, NULL, &testKernelAIO, "PILETA");
	pthread_create(&carpincho_th_2, NULL, &testKernelAIO, "SALMON");
	pthread_create(&carpincho_th_3, NULL, &testKernelAIO, "ROBLE");
	pthread_create(&carpincho_th_4, NULL, &testKernelAIO, "VALLE");
	pthread_create(&carpincho_th_5, NULL, &testKernelAIO, "HUMEDAL");
	pthread_create(&carpincho_th_6, NULL, &testKernel1, NULL);
	pthread_join(carpincho_th_1,NULL);
	pthread_join(carpincho_th_2,NULL);
	pthread_join(carpincho_th_3,NULL);
	pthread_join(carpincho_th_4,NULL);
	pthread_join(carpincho_th_5,NULL);
	pthread_join(carpincho_th_6,NULL);
	//pthread_create(&carpincho_th_1, NULL, &testKernel1, NULL);
	//pthread_create(&carpincho_th_2, NULL, &testKernel2, NULL);
//	while(1){
//		sleep(5);
//	}

}

void prueba_mensajes(){
	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
	mate_instance mate_ref;
	mate_init(&mate_ref, config_path);


	mate_instance mate_ref2;
	mate_init(&mate_ref2, config_path);

	//pthread_t carpincho_th_1;
	//pthread_t carpincho_th_2;

	//pthread_create(&carpincho_th_1, NULL, &testKernel, NULL);
	//pthread_create(&carpincho_th_2, NULL, &testKernel, NULL);

	mate_call_io(&mate_ref, "hierbitas", "Un mensaje random para molestar");
	mate_close(&mate_ref);

	mate_call_io(&mate_ref2, "hierbitas", "Un mensaje random para molestar");
	mate_close(&mate_ref2);
	//sleep(80);

	char* origin = "hola mundo aqui llegue con un string bien largo para testear a fondo que el memwrite anda bien y es dinamico";
	printf("sizeof origin: %d \n string %s \n", strlen(origin) + 1, origin);

	//Testeamos memalloc
	mate_pointer ptr = mate_memalloc(&mate_ref,strlen(origin)+1);
	printf("mem alloc exito: %d \n",(int)ptr);

	mate_pointer ptr2 = mate_memalloc(&mate_ref2,strlen(origin)+1);
	printf("mem alloc exito: %d \n",(int)ptr2);

	sleep(2);

	// Testeamos mate_memwrite
	int success = mate_memwrite(&mate_ref, origin, ptr, strlen(origin)+1);
	printf("mem write exito: %d \n", success);

	int success2 = mate_memwrite(&mate_ref2, origin, ptr2, strlen(origin)+1);
	printf("mem write exito: %d \n", success2);

	sleep(2);

	// Testeamos mate_memread
	char* msj = malloc(strlen(origin)+1);
	mate_memread(&mate_ref, ptr, msj, strlen(origin)+1);
	printf("Lei esta wea: %s \n", (char*)msj);

	free(msj);

	char* msj2 = malloc(strlen(origin)+1);
	mate_memread(&mate_ref2, ptr2, msj2, strlen(origin)+1);
	printf("Lei esta wea: %s \n", (char*)msj2);

	free(msj2);


	printf("\n%d\n", mate_memfree(&mate_ref, ptr));
	printf("\n%d\n", mate_memfree(&mate_ref2, ptr2));


	//PRUEBA 2 -INT
	int entero = 12345678;
	//Testeamos memalloc
	ptr = mate_memalloc(&mate_ref,sizeof(entero));
	printf("mem alloc exito: %d \n",(int)ptr);

	sleep(2);

	// Testeamos mate_memwrite
	success = mate_memwrite(&mate_ref, &entero, ptr, sizeof(entero));
	printf("mem write exito: %d \n", success);

	sleep(2);

	// Testeamos mate_memread
	int msj3;// = malloc(sizeof(entero));
	mate_memread(&mate_ref, ptr, &msj3, sizeof(entero));
	printf("prueba 2 - entero leido: %d \n", (int)msj3);


	success = mate_memfree(&mate_ref, ptr);
	printf("mem free exito: %d \n", success);
}

void prueba_swamp(){
	typedef struct thread_info {
	  char* mate_cfg_path;
	  uint32_t th_number;
	  sem_t* producer_sem;
	  sem_t* consumer_sem;
	} thread_info;

	char *LOG_PATH = "./probando_el_pantano.log";
	char *PROGRAM_NAME = "probando_el_pantano";
	uint32_t seed;
	sem_t seed_sem_1;
	sem_t seed_sem_2;
	t_log *logger;
	pthread_mutex_t logger_mutex;

	void print_thread_info(char *thread_name, uint32_t value){
	  pthread_mutex_lock(&logger_mutex);
	  log_info(logger, "thread name: %s", thread_name);
	  log_info(logger, "value: %d", value);
	  pthread_mutex_unlock(&logger_mutex);
	}

	void log_message(char *message, va_list args) {
	  pthread_mutex_lock(&logger_mutex);
	  log_info(logger, message, args);
	  pthread_mutex_unlock(&logger_mutex);
	}

	void calculate_value_and_increment_seed(uint32_t *current_value){
	  (*current_value) = seed;
	  seed++;
	}

	void *carpincho_acaparador(void * config) {
	  thread_info *info = (thread_info *) config;

	  char *thread_name = malloc(10);
	  sprintf(thread_name, "%s%d", "CARPINCHO", info->th_number);

	  mate_instance mate_ref;
	  mate_init(&mate_ref, info->mate_cfg_path);

	  mate_pointer key = mate_memalloc(&mate_ref, 10);
	  mate_memwrite(&mate_ref, thread_name, key, 10);

	  //acaparo un archivo de swamp
	  mate_memalloc(&mate_ref, 20000);

	  while(1) {
	    mate_memread(&mate_ref, key, thread_name, 10);
	    log_message("thread name: %s", thread_name);
	  }
	}

	void *carpincho(void *config){
	  thread_info *info = (thread_info *) config;

	  char *thread_name = malloc(10);
	  sprintf(thread_name, "%s%d", "CARPINCHO", info->th_number);

	  mate_instance mate_ref;
	  mate_init(&mate_ref, info->mate_cfg_path);

	  mate_pointer key = mate_memalloc(&mate_ref, 10);
	  mate_memwrite(&mate_ref, thread_name, key, 10);

	  mate_pointer value = mate_memalloc(&mate_ref, sizeof(uint32_t));
	  mate_memwrite(&mate_ref, &seed, value, sizeof(uint32_t));
	  print_thread_info(thread_name, seed);

	  uint32_t current_value;

	  int vuelta = 0;
	  while (vuelta < 1000)  {
	    sem_wait(info->producer_sem);
	    mate_memread(&mate_ref, key, thread_name, 10);
	    mate_memread(&mate_ref, value, &current_value, sizeof(uint32_t));

	    // Pido memoria sin guardar el puntero. No creo que sea un problema
	    mate_memalloc(&mate_ref, 1);

	    calculate_value_and_increment_seed(&current_value);
	    print_thread_info(thread_name, current_value);

	    mate_memwrite(&mate_ref, &current_value, value, sizeof(uint32_t));
	    sem_post(info->consumer_sem);
	    vuelta++;
	  }

	  free(thread_name);
	  mate_close(&mate_ref);
	  return 0;
	}

	/*------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------*/
	logger = log_create(LOG_PATH, PROGRAM_NAME, true, 2);

	char *config_path = "/home/utnso/tp-2021-2c-Sinergia/matelib/matelib.config";
	seed = 1;
	pthread_mutex_init(&logger_mutex, NULL);
	sem_init(&seed_sem_1, 0, 1);
	sem_init(&seed_sem_2, 0, 0);

	pthread_t carpincho_th_1;
	pthread_t carpincho_th_2;
	pthread_t carpincho_th_3;

	thread_info *th_1_info = malloc(sizeof(thread_info));
	th_1_info->mate_cfg_path = config_path;
	th_1_info->th_number = 1;
	th_1_info->consumer_sem = &seed_sem_2;
	th_1_info->producer_sem = &seed_sem_1;

	thread_info *th_2_info = malloc(sizeof(thread_info));
	th_2_info->mate_cfg_path = config_path;
	th_2_info->th_number = 2;
	th_2_info->consumer_sem = &seed_sem_1;
	th_2_info->producer_sem = &seed_sem_2;

	thread_info *th_3_info = malloc(sizeof(thread_info));
	th_3_info->mate_cfg_path = config_path;
	th_3_info->th_number = 3;

	pthread_create(&carpincho_th_3, NULL, &carpincho_acaparador, (void *)th_3_info);
	pthread_create(&carpincho_th_1, NULL, &carpincho, (void *)th_1_info);
	pthread_create(&carpincho_th_2, NULL, &carpincho, (void *)th_2_info);

	pthread_join(carpincho_th_1, NULL);
	pthread_join(carpincho_th_2, NULL);
}

void prueba_Asignacion(char *argv[]){
	sem_t semCarpincho1;
	sem_t semCarpincho2;
	sem_t semCarpincho3;

	void* carpincho1_func(void* config){

		sem_wait(&semCarpincho1);
		mate_instance instanceC1;

		printf("C1 - Llamo a mate_init\n");
		mate_init(&instanceC1, (char*)config);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 10 bytes\n");
		mate_pointer alloc3 = mate_memalloc(&instanceC1, 10);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Escribo en la página 3\n");
		mate_memwrite(&instanceC1, "Hola", alloc3, 5);

		printf("C1 - Escribo en la página 2\n");
		mate_memwrite(&instanceC1, "Hola", alloc2, 5);

		printf("C1 - Escribo en la página 1\n");
		mate_memwrite(&instanceC1, "Hola", alloc1, 5);

		printf("C1 - Escribo en la página 0\n");
		mate_memwrite(&instanceC1, "Hola", alloc0, 5);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc4 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc5 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc6 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc7 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2 para que finalice\n");
		sem_post(&semCarpincho2);

		printf("C1 - Se retira a descansar\n");
		mate_close(&instanceC1);

		return 0;
	}

	void* carpincho2_func(void* config){

		sem_wait(&semCarpincho2);
		mate_instance instanceC2;

		printf("C2 - Llamo a mate_init\n");
		mate_init(&instanceC2, (char*)config);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Reservo un alloc de 10 bytes\n");
		mate_pointer alloc3 = mate_memalloc(&instanceC2, 10);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Escribo en la página 0\n");
		mate_memwrite(&instanceC2, "Hola", alloc0, 5);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Libero al C3 para que finalice\n");
		sem_post(&semCarpincho3);

		printf("C2 - Se retira a descansar\n");
		mate_close(&instanceC2);

		return 0;
	}

	void* carpincho3_func(void* config){

		sem_wait(&semCarpincho3);
		mate_instance instanceC3;

		printf("C3 - Llamo a mate_init\n");
		mate_init(&instanceC3, (char*)config);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Reservo un alloc de 10 bytes\n");
		mate_pointer alloc3 = mate_memalloc(&instanceC3, 10);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Escribo en la página 0\n");
		mate_memwrite(&instanceC3, "Hola", alloc0, 5);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Se retira a descansar\n");
		mate_close(&instanceC3);

		return 0;
	}

	/*------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------*/
	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;

	sem_init(&semCarpincho1, 0, 1);
	sem_init(&semCarpincho2, 0, 0);
	sem_init(&semCarpincho3, 0, 0);

	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);

	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");
}

void prueba_base_carpincho1(int argc,char *argv[]){
	#define SEMAFORO_SALUDO "SEM_HELLO"

	/*------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------*/
	 if(argc < 2){
	        printf("No se ingresó archivo de configuración");
	        exit(EXIT_FAILURE);
	    }

	    char* config = argv[1];

		printf("MAIN - Utilizando el archivo de config: %s\n", config);

		mate_instance instance;

		mate_init(&instance, (char*)config);

	    char saludo[] = "No, ¡hola humedal!\n";

	    mate_pointer saludoRef = mate_memalloc(&instance, strlen(saludo));

	    mate_memwrite(&instance, saludo, saludoRef, strlen(saludo));

	    mate_sem_init(&instance, SEMAFORO_SALUDO, 0);

	    mate_sem_wait(&instance, SEMAFORO_SALUDO);

	    mate_memread(&instance, saludoRef, saludo, strlen(saludo));

	    printf(saludo);

	    mate_close(&instance);


}

void prueba_MMU(char* argv[]){
	sem_t semCarpincho1;
	sem_t semCarpincho2;
	sem_t semCarpincho3;

	void* carpincho1_func(void* config){

		sem_wait(&semCarpincho1);
		mate_instance instanceC1;

		printf("C1 - Llamo a mate_init\n");
		mate_init(&instanceC1, (char*)config);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc3 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc4 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc5 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc6 = mate_memalloc(&instanceC1, 23);

		printf("C1 - Escribo en la página 0\n");
		mate_memwrite(&instanceC1, "Hola", alloc0, 5);

		printf("C1 - Escribo en la página 1\n");
		mate_memwrite(&instanceC1, "Chau", alloc1, 5);

		printf("C1 - Libero al C2\n");
		sem_post(&semCarpincho2);

		printf("C1 - Freno a C1\n");
		sem_wait(&semCarpincho1);

		printf("C1 - Escribo en la página 2\n");
		mate_memwrite(&instanceC1, "Carpincho", alloc2, 10);

		printf("C1 - Escribo en la página 3\n");
		mate_memwrite(&instanceC1, "Capibara", alloc3, 9);

		printf("C1 - Escribo en la página 4\n");
		mate_memwrite(&instanceC1, "Hydrochaeris", alloc4, 13);

		printf("C1 - Reservo un alloc de 21 bytes\n");
		mate_pointer alloc7 = mate_memalloc(&instanceC1, 21);

		printf("C1 - Libero al C2 para que finalice\n");
		sem_post(&semCarpincho2);

		printf("C1 - Libero al C3 para que finalice\n");
		sem_post(&semCarpincho3);

		printf("C1 - Se retira a descansar\n");
		mate_close(&instanceC1);

		return 0;
	}

	void* carpincho2_func(void* config){

		sem_wait(&semCarpincho2);
		mate_instance instanceC2;

		printf("C2 - Llamo a mate_init\n");
		mate_init(&instanceC2, (char*)config);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC2, 23);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		void* localMalloc = malloc(5);

		printf("C2 - Leo de la página 0\n");
		mate_memread(&instanceC2, alloc0, localMalloc , 5);

		printf("C2 - Leo de la página 1\n");
		mate_memread(&instanceC2, alloc1, localMalloc, 5);

		printf("C2 - Libero al C3\n");
		sem_post(&semCarpincho3);

		printf("C2 - Freno a C2\n");
		sem_wait(&semCarpincho2);

		printf("C2 - Se retira a descansar\n");
		mate_close(&instanceC2);

		return 0;
	}

	void* carpincho3_func(void* config){

		sem_wait(&semCarpincho3);
		mate_instance instanceC3;

		printf("C3 - Llamo a mate_init\n");
		mate_init(&instanceC3, (char*)config);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc0 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc1 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Reservo un alloc de 23 bytes\n");
		mate_pointer alloc2 = mate_memalloc(&instanceC3, 23);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		void* localMalloc = malloc(5);

		printf("C3 - Leo de la página 0\n");
		mate_memread(&instanceC3, alloc0, localMalloc , 5);

		printf("C3 - Leo de la página 1\n");
		mate_memread(&instanceC3, alloc1, localMalloc, 5);

		printf("C3 - Libero al C1\n");
		sem_post(&semCarpincho1);

		printf("C3 - Freno a C3\n");
		sem_wait(&semCarpincho3);

		printf("C3 - Se retira a descansar\n");
		mate_close(&instanceC3);

		return 0;
	}


	/*------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------*/
	pthread_t carpincho1;
	pthread_t carpincho2;
	pthread_t carpincho3;

	sem_init(&semCarpincho1, 0, 1);
	sem_init(&semCarpincho2, 0, 0);
	sem_init(&semCarpincho3, 0, 0);

	printf("MAIN - Utilizando el archivo de config: %s\n", argv[1]);

	pthread_create(&carpincho1, NULL, carpincho1_func, argv[1]);
	pthread_create(&carpincho2, NULL, carpincho2_func, argv[1]);
	pthread_create(&carpincho3, NULL, carpincho3_func, argv[1]);

	pthread_join(carpincho3, NULL);
	pthread_join(carpincho2, NULL);
	pthread_join(carpincho1, NULL);

	printf("MAIN - Retirados los carpinchos de la pelea, hora de analizar los hechos\n");
}

int prueba_TLB_fifo(int argc, char *argv[]){
	typedef struct thread_info
	{
	  char* mate_cfg_path;
	  uint32_t th_number;
	  sem_t* producer_sem;
	  sem_t* consumer_sem;
	} thread_info;

	char *LOG_PATH = "./secuencia.log";
	char *PROGRAM_NAME = "secuencia";
	uint32_t seed;
	sem_t seed_sem_1;
	sem_t seed_sem_2;
	t_log *logger;
	pthread_mutex_t logger_mutex;

	void print_thread_info(char *thread_name, uint32_t value)
	{
	  pthread_mutex_lock(&logger_mutex);
	  log_info(logger, "thread name: %s", thread_name);
	  log_info(logger, "value: %d", value);
	  pthread_mutex_unlock(&logger_mutex);
	}

	void log_message(char *message) {
	  pthread_mutex_lock(&logger_mutex);
	  log_info(logger, message);
	  pthread_mutex_unlock(&logger_mutex);
	}

	void calculate_value_and_increment_seed(uint32_t *current_value)
	{
	  (*current_value) = seed;
	  seed++;
	}

	void *carpincho(void *config)
	{
	  thread_info *info = (thread_info *) config;

	  char *thread_name = malloc(10);
	  sprintf(thread_name, "%s%d", "CARPINCHO", info->th_number);

	  mate_instance mate_ref;
	  mate_init(&mate_ref, info->mate_cfg_path);

	  mate_pointer key = mate_memalloc(&mate_ref, 10);
	  mate_memwrite(&mate_ref, thread_name, key, 10);

	  mate_pointer value = mate_memalloc(&mate_ref, sizeof(uint32_t));
	  mate_memwrite(&mate_ref, &seed, value, sizeof(uint32_t));
	  print_thread_info(thread_name, seed);

	  uint32_t current_value;

	  while (1)
	  {
	    sem_wait(info->producer_sem);
	    mate_memread(&mate_ref, key, thread_name, 10);
	    mate_memread(&mate_ref, value, &current_value, sizeof(uint32_t));

	    calculate_value_and_increment_seed(&current_value);
	    print_thread_info(thread_name, current_value);

	    mate_memwrite(&mate_ref, &current_value, value, sizeof(uint32_t));
	    sem_post(info->consumer_sem);
	  }

	  free(thread_name);
	  mate_close(&mate_ref);
	  return 0;
	}


	/*-------------------------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------------------------*/

	logger = log_create(LOG_PATH, PROGRAM_NAME, true, 2);
	if (argc != 2){
		log_info(logger, "Debe ingresar el path del archivo de config\n");
		return -1;
	}

	char *config_path = argv[1];
	seed = 1;
	pthread_mutex_init(&logger_mutex, NULL);
	sem_init(&seed_sem_1, 0, 1);
	sem_init(&seed_sem_2, 0, 0);

	pthread_t carpincho_th_1;
	pthread_t carpincho_th_2;

	thread_info *th_1_info = malloc(sizeof(thread_info));
	th_1_info->mate_cfg_path = config_path;
	th_1_info->th_number = 1;
	th_1_info->consumer_sem = &seed_sem_2;
	th_1_info->producer_sem = &seed_sem_1;

	thread_info *th_2_info = malloc(sizeof(thread_info));
	th_2_info->mate_cfg_path = config_path;
	th_2_info->th_number = 2;
	th_2_info->consumer_sem = &seed_sem_1;
	th_2_info->producer_sem = &seed_sem_2;

	pthread_create(&carpincho_th_1, NULL, &carpincho, (void *)th_1_info);
	pthread_create(&carpincho_th_2, NULL, &carpincho, (void *)th_2_info);

	pthread_join(carpincho_th_1, NULL);
	pthread_join(carpincho_th_2, NULL);

	return 0;
}

int prueba_TLB_LRU(int argc, char *argv[]){
	char *LOG_PATH = "./fibonacci.log";
	char *PROGRAM_NAME = "fibonacci";
	t_log *logger;


	/*-------------------------------------------------------------------
	 * 						MAIN
	 *-------------------------------------------------------------------*/
	uint32_t n_1 = 1;
	  uint32_t n_2 = 0;
	  uint32_t n = 0;
	  char *label_n_1 = strdup("n-1");
	  char *label_n_2 = strdup("n-2");

	  logger = log_create(LOG_PATH, PROGRAM_NAME, true, 2);
	  if (argc != 2)
	  {
	    log_info(logger, "Debe ingresar el path del archivo de config\n");
	    return -1;
	  }

	  char *config_path = argv[1];

	  //Instancio la lib
	  mate_instance mate_ref;
	  mate_init(&mate_ref, config_path);

	  //Pido memoria y asigno valores estáticos e iniciales de la sucesión de fibonacci
	  mate_pointer key_2 = mate_memalloc(&mate_ref, 4);
	  log_info(logger, "key_2: %d", key_2);
	  mate_memwrite(&mate_ref, label_n_2, key_2, 4);

	  mate_pointer value_2 = mate_memalloc(&mate_ref, sizeof(uint32_t));
	  mate_memwrite(&mate_ref, &n_2, value_2, sizeof(uint32_t));
	  log_info(logger, "value_2: %d", value_2);

	  mate_pointer key_1 = mate_memalloc(&mate_ref, 4);
	  mate_memwrite(&mate_ref, label_n_1, key_1, 4);
	  log_info(logger, "key_1: %d", key_1);

	  mate_pointer value_1 = mate_memalloc(&mate_ref, sizeof(uint32_t));
	  mate_memwrite(&mate_ref, &n_1, value_1, sizeof(uint32_t));
	  log_info(logger, "value_1: %d", value_1);

	  while(1) {
	    mate_memread(&mate_ref, value_1, &n_1, sizeof(uint32_t));
	    mate_memread(&mate_ref, value_2, &n_2, sizeof(uint32_t));
	    mate_memread(&mate_ref, key_2, label_n_2, 4);
	    mate_memread(&mate_ref, key_1, label_n_1, 4);

	    n = n_1 + n_2;

	    log_info(logger, "valores de la sucesión:");
	    log_info(logger, "%s: %d", label_n_2, n_2);
	    log_info(logger, "%s: %d", label_n_1, n_1);
	    log_info(logger, "n: %d", n);

	    n_2 = n_1;
	    n_1 = n;

	    mate_memwrite(&mate_ref, &n_1, value_1, sizeof(uint32_t));
	    mate_memwrite(&mate_ref, &n_2, value_2, sizeof(uint32_t));
	  }

	  return 0;
}



/*-------------------------------------------------------------------
 * 						MAIN
 *-------------------------------------------------------------------*/

int main(int argc, char *argv[]){
	//PRUEBAS GENERALES
	prueba_io_kernel();
	//prueba_mensajes();


	//PRUEBAS KERNEL
	//prueba_Asignacion(argv[1]);
	//prueba_base_carpincho1(argc, argv[1]);


	//PRUEBAS MEMORIA
	//prueba_MMU(argv[1]);
	//prueba_TLB_fifo(argc, argv[1]);
	//prueba_TLB_LRU(argc, argv[1]);


	//PRUEBAS SWAMP
	//prueba_swamp();

  return 0;
}
