#include "planificador.h"


// Funcion y variable para buscar en colas
//

static uint32_t pid_buscado = 0;

pthread_mutex_t MUTEX_COLA_NEW = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_COLA_EXIT = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_LISTA_BLOQUEADOS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_LISTA_READY = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_LISTA_SUSPENDIDOS_READY = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_SEMAFOROS = PTHREAD_MUTEX_INITIALIZER;

void iniciarSemaforos(){

	sem_init(&carpinchosEnNew,0,0);
	sem_init(&carpinchosEnReady,0,0);
	sem_init(&gradoMultiprocesamiento,0,KERNEL_CFG->GRADO_MULTIPROCESAMIENTO);
	sem_init(&carpinchoAHiloCpu,0,0);
	sem_init(&carpinchoEnHiloCpu,0,0);



	SEMAFOROS = list_create();
}


void iniciarColas(){
	COLA_NEW = queue_create();
	LISTA_BLOQUEADOS = list_create();
	LISTA_EXIT = list_create();
	LISTA_SUSPENDIDOS_READY = list_create();
	LISTA_READY = list_create();
}


bool filter_by_pid(void* t_c) {
    t_carpincho* c = (t_carpincho*) t_c;
    return c->pid == pid_buscado;
}

bool filter_t_running_thread_by_pid(void* item) {
    t_running_thread* t_r = (t_running_thread*) item;
    return t_r->c->pid == pid_buscado;
}

void free_t_carpincho(void* carpincho) {
    t_carpincho* c = (t_carpincho*) carpincho;

    // if(c->semaforos != NULL) list_destroy_and_destroy_elements(c->semaforos, (void*) free);
    free(c);
}

void cambiar_estado(t_carpincho* c, t_status nuevo) {
    c->status = nuevo;
}


void iniciar_mutex() {

//    pthread_mutex_init(&MUTEX_LISTA_READY, NULL);
//    pthread_mutex_init(&MUTEX_SEMAFOROS, NULL);
//    pthread_mutex_init(&MUTEX_COLA_NEW, NULL);
//    pthread_mutex_init(&MUTEX_COLA_EXIT, NULL);
//    pthread_mutex_init(&MUTEX_COLA_BLOQUEADOS, NULL);
    sem_init(&CARPINCHOS_EN_COLA, 0, 0);
}

void finalizar_mutex() {
    pthread_mutex_destroy(&MUTEX_LISTA_READY);
    pthread_mutex_destroy(&MUTEX_COLA_NEW);
    pthread_mutex_destroy(&MUTEX_COLA_EXIT);
    pthread_mutex_destroy(&MUTEX_LISTA_BLOQUEADOS);
    pthread_mutex_destroy(&MUTEX_LISTA_SUSPENDIDOS_READY);
    sem_destroy(&CARPINCHOS_EN_COLA);
}

//
// Cola new
//

void push_cola_new(t_running_thread* n) {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    queue_push(COLA_NEW, n);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
}

t_running_thread* pop_cola_new() {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    void* p = queue_pop(COLA_NEW);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
    return (t_running_thread*) p;
}

void iterar_cola_new(void (*function)(void*)) {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    list_iterate(COLA_NEW->elements, function);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
}

t_running_thread* buscar_cola_new(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    pid_buscado = pid;
    t_running_thread* ret = list_find(COLA_NEW->elements, filter_t_running_thread_by_pid);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
    return ret;
}

void* remover_cola_new(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    pid_buscado = pid;
    void* p = list_remove_by_condition(COLA_NEW->elements, filter_t_running_thread_by_pid);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
    return p;
}

uint32_t largo_cola_new() {
    pthread_mutex_lock(&MUTEX_COLA_NEW);
    uint32_t ret = queue_size(COLA_NEW);
    pthread_mutex_unlock(&MUTEX_COLA_NEW);
    return ret;
}

//
// Cola new
//




//
// Bloqueados E/S
//

void push_cola_bloqueados(t_running_thread* thread) {
    pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
    list_add(LISTA_BLOQUEADOS, thread);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
}

t_running_thread* pop_cola_bloqueados() {
    pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
    t_running_thread* t = list_remove(LISTA_BLOQUEADOS, 0);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
    return t;
}

t_running_thread* buscar_cola_bloqueados(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
    pid_buscado = pid;
    t_running_thread* ret = list_find(LISTA_BLOQUEADOS, filter_t_running_thread_by_pid);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
    return ret;
}

void iterar_cola_bloqueados(void (*f)(void*)) {
    pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
    list_iterate(LISTA_BLOQUEADOS, f);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
}

uint32_t largo_cola_bloqueados() {
    pthread_mutex_lock(&MUTEX_LISTA_BLOQUEADOS);
    uint32_t ret = list_size(LISTA_BLOQUEADOS);
    pthread_mutex_unlock(&MUTEX_LISTA_BLOQUEADOS);
    return ret;
}

//
// Cola exit
//

void agregar_lista_exit(void* c) {
    cambiar_estado(c, EXIT);
    pthread_mutex_lock(&MUTEX_COLA_EXIT);
    list_add(LISTA_EXIT, c);
    pthread_mutex_unlock(&MUTEX_COLA_EXIT);
}

void* remover_lista_exit(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_COLA_EXIT);
    pid_buscado = pid;
    void* p = list_remove_by_condition(LISTA_EXIT, filter_by_pid);
    pthread_mutex_unlock(&MUTEX_COLA_EXIT);
    return p;
}

void iterar_lista_exit(void (*f)(void*)) {
    pthread_mutex_lock(&MUTEX_COLA_EXIT);
    list_iterate(LISTA_EXIT, f);
    pthread_mutex_unlock(&MUTEX_COLA_EXIT);
}

t_carpincho* obtener_lista_exit(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_COLA_EXIT);
    pid_buscado = pid;
    void* p = list_find(LISTA_EXIT, filter_by_pid);
    pthread_mutex_unlock(&MUTEX_COLA_EXIT);
    return p;
}


//
// Funciones de LISTA_NEW
//

void add_lista_ready(void* t) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);

	if( clock_gettime( CLOCK_REALTIME, &(((t_running_thread*) t)->arriveTime)) == -1 ) {
		 log_error(logger,"Error en la obtencion del tiempo \n");
	}

    pid_buscado = (((t_running_thread*) t)->c)->pid;
    if(list_any_satisfy(LISTA_READY, filter_t_running_thread_by_pid)) {
        pthread_mutex_unlock(&MUTEX_LISTA_READY);
        return;
    }

    list_add(LISTA_READY, t);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
}

void add_lista_ready_sin_mutex(void* t) {

	if( clock_gettime( CLOCK_REALTIME, &(((t_running_thread*) t)->arriveTime)) == -1 ) {
		 log_error(logger,"Error en la obtencion del tiempo \n");
	}

    pid_buscado = (((t_running_thread*) t)->c)->pid;
    if(list_any_satisfy(LISTA_READY, filter_t_running_thread_by_pid)) {
        pthread_mutex_unlock(&MUTEX_LISTA_READY);
        return;
    }

    list_add(LISTA_READY, t);
}

void* remove_by_condition_lista_ready(bool (*f)(void*)) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    void* ret = list_remove_by_condition(LISTA_READY, f);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return ret;
}

void iterar_lista_ready(void (*f)(void*)) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    list_iterate(LISTA_READY, f);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
}

t_running_thread* buscar_lista_ready(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    pid_buscado = pid;
    t_running_thread* p = list_find(LISTA_READY, filter_t_running_thread_by_pid);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return p;
}

t_running_thread* remover_lista_ready_porIndice(int index) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    t_running_thread* p = list_remove(LISTA_READY, index);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return p;
}

void* remover_lista_ready(uint32_t tid) {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    pid_buscado = tid;
    void* p = list_remove_by_condition(LISTA_READY, filter_t_running_thread_by_pid);

    while(list_remove_by_condition(LISTA_READY, filter_t_running_thread_by_pid) != NULL);

    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return p;
}

uint32_t largo_lista_ready() {
    pthread_mutex_lock(&MUTEX_LISTA_READY);
    uint32_t ret = list_size(LISTA_READY);
    pthread_mutex_unlock(&MUTEX_LISTA_READY);
    return ret;
}

