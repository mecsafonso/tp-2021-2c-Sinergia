/*
 * lista_procesos.h
 *
 *  Created on: 29 oct. 2021
 *      Author: utnso
 */

#ifndef ASIGNACIONDINAMICA_LISTA_PROCESOS_H_
#define ASIGNACIONDINAMICA_LISTA_PROCESOS_H_

#include "../init.h"

bool existeProceso(int pid);
bool crearProceso_D(int pid);
void borrarEstructurasProcesos_D(int pid);
int asignarEspacio(int pid);
char* buscarArchivoDisponible_D();
int tamanioArchivo(int indiceLista);

int buscarFramesDisponibles_D(int pid);
int liberarPagina_D(int pid, int frame);


#endif /* ASIGNACIONDINAMICA_LISTA_PROCESOS_H_ */
