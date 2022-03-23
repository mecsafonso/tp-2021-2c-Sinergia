/*
 * sharedlib.h
 *
 *  Created on: 10 sep. 2021
 *      Author: utnso
 */

#ifndef SRC_SHAREDLIB_H_
#define SRC_SHAREDLIB_H_

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <commons/string.h>
#include <pthread.h>

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>


#include <inttypes.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


// Codigos de los mensajes
/*todo: esto se envió a protocol.h
 * typedef enum {

	DEBUG,

	//Swamp
	PEDIDO_DE_LECTURA,
	PEDIDO_DE_ESCRITURA,
	BORRAR_PARTICION_PROCESO,

}op_code;*/

// Sockets
int iniciar_servidor(t_log* logger, const char* name, char* ip, char* puerto);
int esperar_cliente(t_log* logger, const char* name, int socket_servidor);
int crear_conexion(t_log* logger, const char* server_name, char* ip, char* puerto);
void liberar_conexion(int* socket_cliente);

// Protocolo


// Utils
bool config_has_all_properties(t_config*, char**);




#endif /* SRC_SHAREDLIB_H_ */
