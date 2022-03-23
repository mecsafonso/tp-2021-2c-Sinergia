/*
 * tlb.h
 *
 *  Created on: 16 dic. 2021
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_

#include "init.h"

extern pthread_mutex_t mutex_TLB_bitmap;

int puntero_TLB;
t_bitarray* TLB_BITMAP;


void init_TLB();
int cargar_pagina_TLB(t_page* new_page);
int buscar_en_TLB(int id_process, int nro_page);
int consultar_TLB(t_pageTable* TP, t_page* page);
int posicion_en_TLB(t_page* page);
void eliminar_entrada_en_tlb(t_page* victima);


#endif /* TLB_H_ */

