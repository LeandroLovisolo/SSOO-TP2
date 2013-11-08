//
//  server.c
//  Batalla Naval
//
//  Created by Federico Raimondo on 4/24/13.
//  Copyright (c) 2013 ar.dc.uba.so. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <modelo.h>
#include <decodificador.h>
#include <globales.h>
#include <pthread.h>


#define MAX_MSG_LENGTH 4096
#define MAX_JUGADORES 100

/* Setea un socket como no bloqueante */
int no_bloqueante(int fd) {
    int flags;
    /* Toma los flags del fd y agrega O_NONBLOCK */
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1 )
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


/* Variables globales del server */
int sock;							// Socket donde se escuchan las conexiones entrantes
struct sockaddr_in name, remote;	// Direcciones
char buf[MAX_JUGADORES][MAX_MSG_LENGTH]; // Buffer de recepción de mensajes
int s[MAX_JUGADORES];				// Sockets de los jugadores
int ids[MAX_JUGADORES];				// Ids de los jugadores
Modelo * model = NULL;				// Puntero al modelo del juego
Decodificador *decoder  = NULL;		// Puntero al decodificador
int n, tamanio, tamanio_barcos;		// Variables de configuracion del juego.


/* Resetea el juego */
void reset() {
	if (model != NULL) {
		delete model;
	}
	if (decoder != NULL) {
		delete decoder;
	}
	model = new Modelo(n, tamanio, tamanio_barcos);
	decoder = new Decodificador(model);
}


/* Socket de comunicación del controlador */
int s_controlador;
/* Para anteder al controlador */
/*
void atender_controlador() {
	int recibido;
	std::string resp;
	recibido = recv(s_controlador, buf, MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");		
	} else if (recibido > 0) {
		buf[recibido]='\0';
		char * pch = strtok(buf, "|");
		while (pch != NULL) {
			//Ejecutar y responder
			resp = decoder->decodificar(pch);
			send(s_controlador,resp.c_str(), resp.length() +1, 0);
		}
	}
}
*/

/* Para atender al i-esimo jugador */
void atender_jugador(int i) {
	int recibido;
	std::string resp;
	recibido = recv(s[i], buf[i], MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");
		
	} else if (recibido > 0) {
		buf[i][recibido]='\0';
		// Separo los mensajes por el caracter |

		char * pch = strtok(buf[i], "|");
		while (pch != NULL) {
			
			// No muestro por pantalla los NOP, son muchos
			if (strstr(pch, "Nop") == NULL) {
				printf("Recibido: %s\n", pch);
			}
			//Decodifico el mensaje y obtengo una respuesta
			resp = decoder->decodificar(pch);
			
			// Si no se cual es el ID de este jugador, trato de obtenerlo de la respuesta
			if (ids[i] == -1) {
				ids[i] = decoder->dameIdJugador(resp.c_str());
			}
			
			// Envio la respuesta
			send(s[i],resp.c_str(), resp.length() +1, 0);
			
			// No muestro por pantalla los NOP, son muchos
			if (strstr(pch, "Nop") == NULL) {
				printf("Resultado %s\n", resp.c_str());
			}
			// Si ya se cual es el jugador
			if (ids[i] != -1) {
				bool hayEventos = model->hayEventos(ids[i]);
				while(hayEventos) {
					resp = decoder->encodeEvent(ids[i]);
					printf("Enviando evento %s", resp.c_str());
					send(s[i],resp.c_str(), resp.length() +1, 0);
					hayEventos = model->hayEventos(ids[i]);
				}
			}
			pch = strtok(NULL, "|");
		}
	}
}

//Funcion que corre cada thread
void *cliente(void *nroJugador) {
	bool sale = false;
	int *ptr_jugador = (int *) nroJugador;
	int jugador = *ptr_jugador;
	delete ptr_jugador;
	printf("Nuevo jugador aceptado! %d \n",jugador);
	while (!sale) {
		atender_jugador(jugador);
		sale = model->termino();
	}
	return 0;
}

/* Acepta todas las conexiones entrantes */
void acceptClients(pthread_t *thread) {
	printf("Iniciando aceptación de clientes \n");
	int t;
	for (int i = 0; i < n; i++) {
		t = sizeof(remote);
		if ((s[i] = accept(sock, (struct sockaddr*) &remote, (socklen_t*) &t)) == -1) {
			perror("aceptando la conexión entrante");
			exit(1);
		}
		printf("Paso el accept %d \n",i);
		ids[i] = -1;
		int flag = 1;
		setsockopt(s[i],            /* socket affected */
				IPPROTO_TCP,     /* set option at TCP level */
				TCP_NODELAY,     /* name of option */
				(char *) &flag,  /* the cast is historical */
				sizeof(int));    /* length of option value */
		int *j = new int;
		*j = i;
		pthread_create(&thread[i], NULL, cliente, j);
	}
}

/*
 * Recibe 4 parametros:
 * argv[1]: Puerto
 * argv[2]: Cantidad de jugadores (N)
 * argv[3]: Tamanio del tablero
 * argv[4]: Tamanio total de los barcos
 */
int main(int argc, char * argv[]) {
	if (argc < 5) {
		printf("Faltan parametros\n");
		printf("Se espera ./server puerto jugadores tamanio_tablero tamanio_barcos\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	n = atoi(argv[2]);
	tamanio = atoi(argv[3]);
	tamanio_barcos = atoi(argv[4]);
	
	inicializar();
	int port_controlador = CONTROLLER_PORT;
	
	
	printf("Escuchando en el puerto %d - controlador en %d\n", port, port_controlador);
	printf("Jugadores %d - Tamanio %d - Tamanio Barcos %d\n", n, tamanio, tamanio_barcos);
	reset();

	
	
	/* Crear socket sobre el que se lee: dominio INET, protocolo TCP (STREAM). */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("abriendo socket");
		exit(1);
	}
	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(port);
	if (bind(sock, (const struct sockaddr*) &name, sizeof(name))) {
		perror("binding socket");
		exit(1);
	}
	
	/* Escuchar en el socket y permitir n conexiones en espera. */
	if (listen(sock, n) == -1) {
		perror("escuchando");
		exit(1);
	}
	//N pthreads, uno por cada jugador
	pthread_t thread[n];

	acceptClients(thread);
	
	
	printf("Corriendo...\n");
	//Espero a que terminen todos los threads
    for (int i = 0; i < n; ++i) {
    	pthread_join(thread[i], NULL);
    }

    printf("Termino el juego, cerrando\n");
    //Cierro los sockets
	for (int i = 0; i < n; i++) {
		close(s[i]);
	}
	close(sock);
	return 0;
}
