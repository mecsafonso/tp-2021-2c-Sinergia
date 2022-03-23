/*
 * paginas.h
 *
 *  Created on: 9 oct. 2021
 *      Author: utnso
 */

#ifndef PAGINAS_H_
#define PAGINAS_H_
#include "sharedlib.h"
#include "log.h"
#include "comunicacion.h"
#include "init.h"
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

int devolver_frame_disponible(int fd, int procedencia, void* proceso);
void crear_bitmap();

bool checkearFrame(int pid, int page);
void escribirFrame(int frameId, void* datosAGuardar, int size_datosAGuardar);

void* devolverFrame(int frameId);



#endif /* PAGINAS_H_ */
