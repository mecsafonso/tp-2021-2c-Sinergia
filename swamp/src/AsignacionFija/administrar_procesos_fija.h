/*
 * administrar_procesos_fija.h
 *
 *  Created on: 9 nov. 2021
 *      Author: utnso
 */

#ifndef ASIGNACIONFIJA_ADMINISTRAR_PROCESOS_FIJA_H_
#define ASIGNACIONFIJA_ADMINISTRAR_PROCESOS_FIJA_H_

#include "../init.h"

bool crearProceso_F(int pid);
char* buscarArchivoDisponible_F();
void borrarEstructurasProcesos_F(int pid);
int buscarFramesDisponibles_F(int pid);
int asignarEspacio_F(int pid);
int liberarPagina_F(int pid, int frameId);
bool checkearFrame_F(int pid, int page);



#endif /* ASIGNACIONFIJA_ADMINISTRAR_PROCESOS_FIJA_H_ */
