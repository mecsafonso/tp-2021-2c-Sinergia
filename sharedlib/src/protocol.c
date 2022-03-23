/*
 * protocol.c
 *
 *  Created on: 24 sep. 2021
 *      Author: utnso
 */
#include "protocol.h"


// FUNCIONES PARA LA SERIALIZACION Y DESERIALIZACION
void serializarVariable(void* AEnviar, void* ASerializar, int tamanio, int *offset){
	memcpy(AEnviar + *offset, ASerializar, tamanio);
	*offset += tamanio;
}

void serializarChar(void* mensajeSerializado, char* mensaje, uint32_t tamanio, int* offset){
	serializarVariable(mensajeSerializado, &(tamanio), sizeof(tamanio), offset);
	serializarVariable(mensajeSerializado, mensaje, tamanio, offset);
}


void deserializarVariable(void* variable, void* stream, int tamanio, int* offset){
	memcpy(variable, stream + *offset, tamanio);
	*offset += tamanio;
}

void deserializarChar(char** receptor, void* stream, int* offset){
	uint32_t tamanio;
	deserializarVariable(&(tamanio), stream, sizeof(tamanio), offset);
	*receptor = malloc(tamanio);
	deserializarVariable(*receptor, stream, tamanio, offset);
}




/* --------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/
static void* serializar_un_int(int sucess, op_code cop) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &sucess, sizeof(int), &offset);
    return stream;
}

static void deserializar_un_int(void* stream, int* sucess) {
    memcpy(sucess, stream, sizeof(int));
}
/* --------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/



// CREAR_PROCESO
static void* serializar_crear_proceso(int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    op_code cop = CREAR_PROCESO;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);
    serializarVariable(stream,&pid,sizeof(int),&offset);

    return stream;
}

static void deserializar_crear_proceso(void* stream, int* fd) {
    memcpy(fd, stream, sizeof(int));
}

bool send_crear_proceso(int fd, int pid) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_crear_proceso(pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_crear_proceso(int fd, int* pid) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_crear_proceso(stream, pid);

    free(stream);
    return true;
}


// Respuesta CREAR_PROCESO
static void* serializar_R_crear_proceso(int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    op_code cop = R_CREAR_PROCESO;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &pid, sizeof(int), &offset);
    return stream;
}

static void deserializar_R_crear_proceso(void* stream, int* fd) {
    memcpy(fd, stream, sizeof(int));
}

bool R_send_crear_proceso(int fd, int pid) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_R_crear_proceso(pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool R_recv_crear_proceso(int fd, int* pid) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }if (pid < 0){
    	free(stream);
    	return 0;
    }

    deserializar_R_crear_proceso(stream, pid);

    free(stream);
    return true;
}





// FINALIZAR_PROCESO
static void* serializar_finalizar_proceso(int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    op_code cop = FINALIZAR_PROCESO;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &pid, sizeof(int), &offset);
    //memcpy(stream+sizeof(op_code), &fd, sizeof(int));
    return stream;
}

static void deserializar_finalizar_proceso(void* stream, int* pid) {
    memcpy(pid, stream, sizeof(int));
}

bool send_finalizar_proceso(int fd, int pid) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_finalizar_proceso(pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_finalizar_proceso(int fd, int* numeroDeProceso) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_finalizar_proceso(stream, numeroDeProceso);

    free(stream);
    return true;
}


//R FINALIZAR_PROCESO
bool send_R_finalizar_proceso(int fd, int success) {
    size_t size = sizeof(op_code) + sizeof(int) * 2;
    void* stream = serializar_un_int(success, R_FINALIZAR_PROCESO);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_finalizar_proceso(int fd, int* success) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, success);

    free(stream);
    return true;
}

// SUSPENDER_PROCESO
static void* serializar_suspender_proceso(int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));
    op_code cop = SUSPENDER_PROCESO;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);
    serializarVariable(stream,&pid,sizeof(int),&offset);

    return stream;
}

static void deserializar_suspender_proceso(void* stream, int* fd) {
    memcpy(fd, stream, sizeof(int));
}

bool send_suspender_proceso(int fd, int pid) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_suspender_proceso(pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_suspender_proceso(int fd, int* pid) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_suspender_proceso(stream, pid);

    free(stream);
    return true;
}


//R SUSPENDER_PROCESO
bool send_R_suspender_proceso(int fd, int success) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(success, R_SUSPENDER_PROCESO);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_suspender_proceso(int fd, int* success) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, success);

    free(stream);
    return true;
}



// MEM_ALLOC
static void* serializar_mem_alloc(uint32_t tamanio, int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(uint32_t) + sizeof(int));

    op_code cop = MEM_ALLOC;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &tamanio, sizeof(uint32_t), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    return stream;
}

static void deserializar_mem_alloc(void* stream, uint32_t* tamanio, int* pid) {
	int offset = 0;
	deserializarVariable(tamanio, stream, sizeof(uint32_t), &offset);
	deserializarVariable(pid, stream, sizeof(int), &offset);
    //memcpy(tamanio, stream, sizeof(uint32_t));
}

bool send_mem_alloc(int fd, uint32_t tamanio, int pid) {
    size_t size = sizeof(op_code) + sizeof(uint32_t) + sizeof(int);
    void* stream = serializar_mem_alloc(tamanio, pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mem_alloc(int fd, uint32_t* tamanio, int* pid) {
    size_t size = sizeof(uint32_t) + sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_mem_alloc(stream, tamanio, pid);

    free(stream);
    return true;
}

// R_MEM_ALLOC
static void* serializar_R_mem_alloc(int direccionLogica) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    op_code cop = R_MEM_ALLOC;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &direccionLogica, sizeof(int), &offset);
    //memcpy(stream+sizeof(op_code), &fd, sizeof(int));
    return stream;
}

static void deserializar_R_mem_alloc(void* stream, int* direccionLogica) {
    memcpy(direccionLogica, stream, sizeof(int));
}

bool send_R_mem_alloc(int fd, int direccionLogica) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_R_mem_alloc(direccionLogica);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_R_mem_alloc(int fd, int* direccionLogica) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_R_mem_alloc(stream, direccionLogica);

    free(stream);
    return true;
}


// MEM_FREE
static void* serializar_mem_free(int32_t addr, int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int32_t) + sizeof(int));

    op_code cop = MEM_FREE;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &addr, sizeof(int32_t), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    return stream;
}

static void deserializar_mem_free(void* stream, int32_t* addr, int* pid) {
	int offset = 0;
	deserializarVariable(addr, stream, sizeof(int32_t), &offset);
	deserializarVariable(pid, stream, sizeof(int), &offset);
    //memcpy(tamanio, stream, sizeof(uint32_t));
}

bool send_mem_free(int fd, int32_t addr, int pid) {
    size_t size = sizeof(op_code) + sizeof(int32_t) + sizeof(int);
    void* stream = serializar_mem_free(addr, pid);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mem_free(int fd, int32_t* addr, int* pid) {
    size_t size = sizeof(int32_t) + sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_mem_free(stream, addr, pid);

    free(stream);
    return true;
}


// R_MEM_FREE
static void* serializar_R_mem_free(int direccionLogica) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));

    op_code cop = R_MEM_FREE;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &direccionLogica, sizeof(int), &offset);
    //memcpy(stream+sizeof(op_code), &fd, sizeof(int));
    return stream;
}

static void deserializar_R_mem_free(void* stream, int* direccionLogica) {
    memcpy(direccionLogica, stream, sizeof(int));
}

bool send_R_mem_free(int fd, int direccionLogica) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_R_mem_free(direccionLogica);
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_R_mem_free(int fd, int* direccionLogica) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_R_mem_free(stream, direccionLogica);

    free(stream);
    return true;
}


// MEM_WRITE
static void* serializar_mem_write(int sizeOrigin, void *origin, int32_t dest, int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int32_t) + sizeof(int)*2 + sizeOrigin);

    op_code cop = MEM_WRITE;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &sizeOrigin, sizeof(int), &offset);
    serializarVariable(stream, origin, sizeOrigin, &offset);
    serializarVariable(stream, &dest, sizeof(int32_t), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    return stream;
}

static void deserializar_mem_write(void* stream, int sizeOrigin, void **origin, int32_t* dest, int* pid) {
	int offset = 0;
	*origin = malloc(sizeOrigin);
	deserializarVariable(*origin, stream, sizeOrigin, &offset);
	deserializarVariable(dest, stream, sizeof(int32_t), &offset);
	deserializarVariable(pid, stream, sizeof(int), &offset);
}

bool send_mem_write(int fd, void *origin, int32_t dest, int sizeOrigin, int pid) {
    size_t size = 	sizeof(op_code) + 	//op_code
    				sizeof(int32_t) + 	//dest
					sizeof(int)*2 + 	//sizeOrigin - pid
					sizeOrigin;			//origin

    void* stream = serializar_mem_write(sizeOrigin, origin, dest, pid); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mem_write(int fd, void **origin, int32_t* dest, int* sizeOrigin, int* pid) {
	//printf("stream en recv mem write %s", (char*)origin);
	recv(fd, sizeOrigin, sizeof(int), 0);

    size_t size = sizeof(int32_t) + sizeof(int) + *sizeOrigin;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_mem_write(stream, *sizeOrigin, origin, dest, pid);

    free(stream);
    return true;
}


// R_MEM_WRITE
static void* serializar_R_mem_write(int rta) {
    void* stream = malloc(sizeof(op_code) + sizeof(int));
    op_code cop = R_MEM_WRITE;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    serializarVariable(stream, &rta, sizeof(int), &offset);
    return stream;
}

static void deserializar_R_mem_write(void* stream, int* rta) {
    memcpy(rta, stream, sizeof(int));
}

bool send_R_mem_write(int fd, int rta) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_R_mem_write(rta);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_R_mem_write(int fd, int* rta) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    deserializar_R_mem_write(stream, rta);

    free(stream);
    return true;
}


// MEM_READ
static void* serializar_mem_read(int sizeToRead, int32_t origin, int pid) {
    void* stream = malloc(sizeof(op_code) + sizeof(int32_t) + sizeof(int)*2);

    op_code cop = MEM_READ;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &sizeToRead, sizeof(int), &offset);
    serializarVariable(stream, &origin, sizeof(int32_t), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    return stream;
}

static void deserializar_mem_read(void* stream, int* sizeToRead, int32_t *origin, int* pid) {
	int offset = 0;
	deserializarVariable(sizeToRead, stream, sizeof(int), &offset);
	deserializarVariable(origin, stream, sizeof(int32_t), &offset);
	deserializarVariable(pid, stream, sizeof(int), &offset);
}

bool send_mem_read(int fd, int32_t origin, int sizeToRead, int pid) {
    size_t size = 	sizeof(op_code) + 	//op_code
    				sizeof(int32_t) + 	//origin
					sizeof(int)*2; 		//sizeToRead - pid

    void* stream = serializar_mem_read(sizeToRead, origin, pid); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_mem_read(int fd, int32_t* origin, int* sizeToRead, int* pid) {
    size_t size = sizeof(int32_t) + sizeof(int)*2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_mem_read(stream, sizeToRead, origin, pid);

    free(stream);
    return true;
}

// R_MEM_READ
static void* serializar_R_mem_read(int sizeToRead, void* dest) {
    void* stream = malloc(sizeof(op_code) + sizeof(int) + sizeToRead);

    op_code cop = R_MEM_READ;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &sizeToRead, sizeof(int), &offset);
    serializarVariable(stream, dest, sizeToRead, &offset);
    return stream;
}



bool send_R_mem_read(int fd, void* dest, int sizeToRead) {
    size_t size = 	sizeof(op_code) + 	//op_code
					sizeof(int) + 		//sizeToRead
					sizeToRead;			//dest

    void* stream = serializar_R_mem_read(sizeToRead, dest); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_R_mem_read(int fd, void* dest, int* sizeToRead) {
	recv(fd, sizeToRead, sizeof(int), 0);

    size_t size = *sizeToRead;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        printf("entre aca");
        return false;
    }

    memcpy(dest, stream, *sizeToRead);

    free(stream);
    return true;
}




// INICIAR_SEM

static void* serializar_iniciar_sem(int pid, char* nombreSem, int valorInicial) {
    void* stream = malloc(sizeof(op_code) + sizeof(int)*3 + sizeof(uint32_t) + strlen(nombreSem) + 1);
    int header;
    header = sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1;
    op_code cop = INICIAR_SEM;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &header, sizeof(int), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    serializarVariable(stream, &valorInicial, sizeof(int), &offset);
    serializarChar(stream, nombreSem, strlen(nombreSem) + 1, &offset);
    return stream;
}

static void deserializar_iniciar_sem(void* stream, int sizeHeader, int* pid, int* valorInicial, char** nombreSem) {
	int offset = 0;
	deserializarVariable(pid, stream, sizeof(int), &offset);
	deserializarVariable(valorInicial, stream, sizeof(int), &offset);
	deserializarChar(nombreSem,stream,&offset);
}


bool send_iniciar_sem(int fd, int pid, char* nombreSem, int valorInicial){

    size_t size = sizeof(op_code) + sizeof(int)*3 + sizeof(uint32_t) + strlen(nombreSem) + 1;

	void* stream = serializar_iniciar_sem(pid,nombreSem,valorInicial); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);

	return true;
}

bool recv_iniciar_sem(int fd, int* pid, int* valorInicial, char** nombreSem){
	int sizeHeader;

	recv(fd, &sizeHeader, sizeof(int), 0);
    size_t size = sizeHeader;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_iniciar_sem(stream, sizeHeader, pid, valorInicial, nombreSem);
    free(stream);

	return true;
}


// Respuesta INICIAR_SEM
bool send_R_iniciar_sem(int fd, int success){
	size_t size = sizeof(op_code) + sizeof(int);
	void* stream = serializar_un_int(success, R_INICIAR_SEM);

	if (send(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	free(stream);
	return true;
}
bool recv_R_iniciar_sem(int fd, int* success){
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, success);

    free(stream);
    return true;
}


// WAIT_SEM


static void* serializar_wait_sem(int pid, char* nombreSem) {
    void* stream = malloc(sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1);
    int header;
    header = sizeof(int)*1 + sizeof(uint32_t) + strlen(nombreSem) + 1;
    op_code cop = WAIT_SEM;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &header, sizeof(int), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    serializarChar(stream, nombreSem, strlen(nombreSem) + 1, &offset);
    return stream;
}

static void deserializar_wait_sem(void* stream, int* pid, char** nombreSem) {
	int offset = 0;
	deserializarVariable(pid, stream, sizeof(int), &offset);
	deserializarChar(nombreSem,stream,&offset);
}

bool send_wait_sem(int fd, int pid, char* nombreSem){

    size_t size = sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1;

	void* stream = serializar_wait_sem(pid,nombreSem); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);

	return true;
}

bool recv_wait_sem(int fd, int* pid,  char** nombreSem){

	int sizeHeader;

	recv(fd, &sizeHeader, sizeof(int), 0);
	size_t size = sizeHeader;
	void* stream = malloc(size);

	if (recv(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	deserializar_wait_sem(stream, pid, nombreSem);
	free(stream);

	return true;
}



// Respuesta CALL_IO

static void* serializar_opcode(op_code cop) {
    void* stream = malloc(sizeof(op_code));
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    return stream;
}


bool send_R_call_io(int fd){
	size_t size = sizeof(op_code);
	void* stream = serializar_opcode(CALL_IO);

	if (send(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	free(stream);
	return true;
}

bool recv_R_call_io(int fd){return true;}





// Respuesta WAIT_SEM

bool send_R_wait_sem(int fd){
	size_t size = sizeof(op_code);
	void* stream = serializar_opcode(R_WAIT_SEM);

	if (send(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	free(stream);
	return true;
}

bool send_R_wait_sem_eliminado(int fd){
	size_t size = sizeof(op_code);
	void* stream = serializar_opcode(R_WAIT_SEM_ELIMINADO);

	if (send(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	free(stream);
	return true;
}


bool recv_R_wait_sem(int fd){return true;}


// POST_SEM


static void* serializar_post_sem(int pid, char* nombreSem) {
    void* stream = malloc(sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1);
    int header;
    header = sizeof(int)*1 + sizeof(uint32_t) + strlen(nombreSem) + 1;
    op_code cop = POST_SEM;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &header, sizeof(int), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    serializarChar(stream, nombreSem, strlen(nombreSem) + 1, &offset);
    return stream;
}

static void deserializar_post_sem(void* stream, int* pid, char** nombreSem) {
	int offset = 0;
	deserializarVariable(pid, stream, sizeof(int), &offset);
	deserializarChar(nombreSem,stream,&offset);
}

bool send_post_sem(int fd, int pid, char* nombreSem){

    size_t size = sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1;

	void* stream = serializar_post_sem(pid,nombreSem); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);

	return true;
}

bool recv_post_sem(int fd, int* pid,  char** nombreSem){

	int sizeHeader;

	recv(fd, &sizeHeader, sizeof(int), 0);
	size_t size = sizeHeader;
	void* stream = malloc(size);

	if (recv(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	deserializar_post_sem(stream, pid, nombreSem);
	free(stream);

	return true;
}


// Respuesta POST_SEM

bool send_R_post_sem(int fd){return true;}

bool recv_R_post_sem(int fd){return true;}


// DESTROY_SEM


static void* serializar_destroy_sem(int pid, char* nombreSem) {
    void* stream = malloc(sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1);
    int header;
    header = sizeof(int)*1 + sizeof(uint32_t) + strlen(nombreSem) + 1;
    op_code cop = DESTROY_SEM;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &header, sizeof(int), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    serializarChar(stream, nombreSem, strlen(nombreSem) + 1, &offset);
    return stream;
}

static void deserializar_destroy_sem(void* stream, int* pid, char** nombreSem) {
	int offset = 0;
	deserializarVariable(pid, stream, sizeof(int), &offset);
	deserializarChar(nombreSem,stream,&offset);
}

bool send_destroy_sem(int fd, int pid, char* nombreSem){

    size_t size = sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t) + strlen(nombreSem) + 1;

	void* stream = serializar_destroy_sem(pid,nombreSem); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);

	return true;
}

bool recv_destroy_sem(int fd, int* pid,  char** nombreSem){

	int sizeHeader;

	recv(fd, &sizeHeader, sizeof(int), 0);
	size_t size = sizeHeader;
	void* stream = malloc(size);

	if (recv(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	deserializar_destroy_sem(stream, pid, nombreSem);
	free(stream);

	return true;
}

// Respuesta DESTROY_SEM

bool send_R_destoy_sem(int fd){return true;}

bool recv_R_destoy_sem(int fd){return true;}



// CALL_IO

static void* serializar_call_io(int pid, char* recurso, char* mensaje) {
    void* stream = malloc(sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t)*2 + strlen(recurso) + strlen(mensaje) + 2);
    int header;
    header = sizeof(int)*1 + sizeof(uint32_t)*2 + strlen(recurso) + strlen(mensaje) + 2;
    op_code cop = CALL_IO;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &header, sizeof(int), &offset);
    serializarVariable(stream, &pid, sizeof(int), &offset);
    serializarChar(stream, recurso, strlen(recurso) + 1, &offset);
    serializarChar(stream, mensaje, strlen(mensaje) + 1, &offset);
    return stream;
}

static void deserializar_call_io(void* stream, int* pid, char** recurso, char** mensaje) {
	int offset = 0;
	deserializarVariable(pid, stream, sizeof(int), &offset);
	deserializarChar(recurso,stream,&offset);
	deserializarChar(mensaje,stream,&offset);
}

bool send_call_io(int fd, int pid, char* recurso, char* mensaje){

    size_t size = sizeof(op_code) + sizeof(int)*2 + sizeof(uint32_t)*2 + strlen(recurso) + strlen(mensaje) + 2;

	void* stream = serializar_call_io(pid,recurso,mensaje); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);

	return true;
}


bool recv_call_io(int fd, int* pid, char** recurso, char** mensaje){

	int sizeHeader;

	recv(fd, &sizeHeader, sizeof(int), 0);
	size_t size = sizeHeader;
	void* stream = malloc(size);

	if (recv(fd, stream, size, 0) != size) {
		free(stream);
		return false;
	}

	deserializar_call_io(stream, pid, recurso,mensaje);
	free(stream);

	return true;
}







/*	-------------------------------------------------------------------
    				MEMORIA
    	-------------------------------------------------------------------*/
static void* serializar_swamp_ready(char* mensaje, size_t* size) {
	*size = sizeof(op_code) +
			sizeof(uint32_t) +
			sizeof(uint32_t) +
			strlen(mensaje)+1;

    void* stream = malloc(*size);

    op_code cop = SWAMP_READY;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    uint32_t tamanio_header = sizeof(uint32_t)+strlen(mensaje)+1;
    serializarVariable(stream, &tamanio_header,sizeof(uint32_t), &offset);
    serializarChar(stream, mensaje, strlen(mensaje)+1, &offset);

    return stream;
}

static void deserializar_swamp_ready(void* stream, char** receptor) {
    int offset = 0;
    deserializarChar(receptor, stream, &offset);
}

bool send_swamp_ready(int fd, char* tipo_asignacion) {
    //size_t size = sizeof(op_code)+strlen(tipo_asignacion)+1+sizeof(uint32_t)*2;
	size_t size = 0;
    void* stream = serializar_swamp_ready(tipo_asignacion, &size);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    printf("llego mensaje");
    return true;
}

bool recv_swamp_ready(int fd, char** tipoAsignacion) {
    size_t size_header = sizeof(uint32_t);
    uint32_t tamanio_header;

    if (recv(fd, &tamanio_header, size_header, 0) != size_header) {
	   return false;
    }

    void* stream = malloc(tamanio_header);

    if (recv(fd, stream, tamanio_header, 0) != tamanio_header) {
        free(stream);
        return false;
    }

    deserializar_swamp_ready(stream, tipoAsignacion);

    free(stream);
    return true;
}


//R SWAMP READY
bool send_R_swamp_ready(int fd, int sucess) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(sucess, R_SWAMP_READY);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_swamp_ready(int fd, int* sucess) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, sucess);

    free(stream);
    return true;
}



/* --------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/
static void* serializar_dos_int(int id_process, int sucess, op_code cop) {
    void* stream = malloc(sizeof(op_code) + sizeof(int) * 2);

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &id_process, sizeof(int), &offset);
    serializarVariable(stream, &sucess, sizeof(int), &offset);
    return stream;
}

static void deserializar_dos_int(void* stream, int* id_process, int* sucess) {
	memcpy(id_process, stream, sizeof(int));
	memcpy(sucess, stream + sizeof(int), sizeof(int));
}
/* --------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/


// CONSULTAR_FRAME_LIBRE_SWAMP
bool send_consultar_frame_libre_swamp(int fd, int id_proceso, int cant_frames) {
    size_t size = sizeof(op_code) + sizeof(int) * 2;
    void* stream = serializar_dos_int(id_proceso, cant_frames, CONSULTAR_FRAME_LIBRE_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_consultar_frame_libre_swamp(int fd, int* id_process, int* cant_frames) {
    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_dos_int(stream, id_process, cant_frames);

    free(stream);
    return true;
}


//R CONSULTAR_FRAME_LIBRE_SWAMP
bool send_R_consultar_frame_libre_swamp(int fd, int cant_frames) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(cant_frames, R_CONSULTAR_FRAME_LIBRE_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_consultar_frame_libre_swamp(int fd, int* cant_frames) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, cant_frames);

    free(stream);
    return true;
}


// RESERVAR_FRAME_LIBRE_SWAMP
bool send_reservar_frame_libre_swamp(int fd, int id_proceso, int success) {
    size_t size = sizeof(op_code) + sizeof(int) * 2;
    void* stream = serializar_dos_int(id_proceso, success, RESERVAR_FRAME_LIBRE_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_reservar_frame_libre_swamp(int fd, int* id_process, int* success) {
    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_dos_int(stream, id_process, success);

    free(stream);
    return true;
}


//R RESERVAR_FRAME_LIBRE_SWAMP
bool send_R_reservar_frame_libre_swamp(int fd, int sucess) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(sucess, R_RESERVAR_FRAME_LIBRE_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_reservar_frame_libre_swamp(int fd, int* sucess) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, sucess);

    free(stream);
    return true;
}


//FREE FRAME SWAMP
bool send_free_frame_swamp(int fd, int id_process, int frame) {
    size_t size = sizeof(op_code) + sizeof(int) * 2;
    void* stream = serializar_dos_int(id_process, frame, FREE_FRAME_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_free_frame_swamp(int fd, int* id_process, int* frame) {
    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_dos_int(stream, id_process, frame);

    free(stream);
    return true;
}


//R FREE FRAME SWAMP
bool send_R_free_frame_swamp(int fd, int frame) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(frame, R_FREE_FRAME_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_free_frame_swamp(int fd, int* frame) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, frame);

    free(stream);
    return true;
}


//GUARDAR EN SWAMP
static void* serializar_guardar_en_swamp(int id_process, int frame, int sizeToRead, void* data) {
	int size_payload = (sizeof(int) * 3) + sizeToRead;
    void* stream = malloc(sizeof(op_code) + sizeof(int) + size_payload);

    op_code cop = GUARDAR_EN_SWAMP;
    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &size_payload, sizeof(int), &offset);
    serializarVariable(stream, &id_process, sizeof(int), &offset);
    serializarVariable(stream, &frame, sizeof(int), &offset);
    serializarVariable(stream, &sizeToRead, sizeof(int), &offset);
    serializarVariable(stream, data, sizeToRead, &offset);
    return stream;
}

static void deserializar_guardar_en_swamp( void* stream, int* id_process, int* frame, void** data, int* sizeToRead) {
	int offset = 0;
	deserializarVariable(id_process, stream, sizeof(int), &offset);
	deserializarVariable(frame, stream, sizeof(int), &offset);
	deserializarVariable(sizeToRead, stream, sizeof(int), &offset);

	*data = malloc(*sizeToRead);
	deserializarVariable(*data, stream, *sizeToRead, &offset);
}

bool send_guardar_en_swamp(int fd, int id_process, int frame, void* data, int sizeToRead) {
    size_t size = 	sizeof(op_code) + 	//op_code
    				(sizeof(int)*4) + 	// size payload + id_process + frame + sizeToRead
					sizeToRead;			//data

    void* stream = serializar_guardar_en_swamp(id_process, frame, sizeToRead, data); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_guardar_en_swamp(int fd, int* id_process, int* frame, void** data, int* sizeToRead) {
	int size_payload;
	recv(fd, &size_payload, sizeof(int), 0);

    size_t size = size_payload;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_guardar_en_swamp(stream, id_process, frame, data, sizeToRead);

    free(stream);
    return true;
}


//R GUARDAR EN SWAMP
bool send_R_guardar_en_swamp(int fd, int success) {
    size_t size = sizeof(op_code) + sizeof(int);
    void* stream = serializar_un_int(success, R_GUARDAR_EN_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_R_guardar_en_swamp(int fd, int* success) {
    size_t size = sizeof(int);
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_un_int(stream, success);

    free(stream);
    return true;
}


//PEDIR PAGE A SWAMP
bool send_pedir_page_a_swamp(int fd, int id_process, int page) {
    size_t size = sizeof(op_code) + sizeof(int) * 2;
    void* stream = serializar_dos_int(id_process, page, PEDIR_PAGE_A_SWAMP);

    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    free(stream);
    return true;
}

bool recv_pedir_page_a_swamp(int fd, int* id_process, int* page) {
    size_t size = sizeof(int) * 2;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    deserializar_dos_int(stream, id_process, page);

    free(stream);
    return true;
}


//R PEDIR PAGE A SWAMP
static void* serializar_R_pedir_page_a_swamp(int size, void* dest) {
    void* stream = malloc(sizeof(op_code) + sizeof(int) + size);

    op_code cop = R_PEDIR_PAGE_A_SWAMP;

    //Serializo el OP_CODE
    memcpy(stream, &cop, sizeof(op_code));
    int offset = sizeof(op_code);

    //Se serializa el resto del mensaje
    serializarVariable(stream, &size, sizeof(int), &offset);
    serializarVariable(stream, dest, size, &offset);
    return stream;
}



bool send_R_pedir_page_a_swamp(int fd, void* dest, int sizeToRead) {
    size_t size = 	sizeof(op_code) + 	//op_code
					sizeof(int) + 		//sizeToRead
					sizeToRead;			//dest

    void* stream = serializar_R_pedir_page_a_swamp(sizeToRead, dest); //paso las cosas en orden de serializacion y deserializacion
    if (send(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }
    free(stream);
    return true;
}

bool recv_R_pedir_page_a_swamp(int fd, void** dest) {
	int* sizeToRead = malloc(sizeof(int));
	recv(fd, sizeToRead, sizeof(int), 0);

    size_t size = *sizeToRead;
    void* stream = malloc(size);

    if (recv(fd, stream, size, 0) != size) {
        free(stream);
        return false;
    }

    //*dest = malloc(*sizeToRead);
    memcpy(*dest, stream, *sizeToRead);

    free(sizeToRead);
    free(stream);
    return true;
}
