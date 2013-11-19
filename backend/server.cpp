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
#include <pthread.h>
#include <modelo.h>
#include <decodificador.h>
#include <globales.h>

#define MAX_MSG_LENGTH    4096
#define MAX_JUGADORES     1024
#define MAX_CONTROLADORES 1024

/* Setea un socket como no bloqueante */
int no_bloqueante(int fd) {
    int flags;
    /* Toma los flags del fd y agrega O_NONBLOCK */
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1 )
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


/* Variables globales del server */
int sock_jugadores;									// Socket donde se escuchan las conexiones entrantes
int sock_controladores;			    				// Socket de controladores
char buf_jugadores[MAX_JUGADORES][MAX_MSG_LENGTH]; 	// Buffer de recepción de mensajes de los jugadores
char buf_controladores[MAX_MSG_LENGTH]; 			// Buffer de recepción de mensajes de los controladores
int s_jugadores[MAX_JUGADORES];						// Sockets de los jugadores
int s_controladores[MAX_CONTROLADORES];				// Sockets de los controladores
pthread_t threads_controladores[MAX_CONTROLADORES]; // Threads de los controladores
int num_threads_controladores;						// Número de threads de controladores usados
int ids[MAX_JUGADORES];								// Ids de los jugadores
Modelo * model = NULL;								// Puntero al modelo del juego
Decodificador *decoder  = NULL;						// Puntero al decodificador
int n, tamanio, tamanio_barcos;						// Variables de configuracion del juego.


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


/* Para anteder al controlador */
void atender_controlador(int nro_controlador) {
	int recibido;
	std::string resp;
	printf("Atendiendo controlador %d\n", nro_controlador);
	recibido = recv(s_controladores[nro_controlador], buf_controladores, MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");
	} else if (recibido > 0) {
		buf_controladores[recibido]='\0';
		char * pch = strtok(buf_controladores, "|");
		while (pch != NULL) {
			//Ejecutar y responder
			resp = decoder->decodificar(pch);
			send(s_controladores[nro_controlador], resp.c_str(), resp.length() +1, 0);
			pch = strtok(NULL, "|");
		}
	} else { /* recibido == 0 */
		// Extraído de la man page de recv(): "If no messages are available to be received
		// and the peer hasperformed an orderly shutdown, recv() shall return 0."
		printf("Saliendo del thread del controlador %d.\n", nro_controlador);
		pthread_exit(NULL);
	}
}

/* Punto de entrada de threads de controlador */
void *controlador(void *nro_controlador_ptr) {
	int nro_controlador = * (int*) nro_controlador_ptr;
	delete (int*) nro_controlador_ptr;
	printf("Nuevo controlador aceptado! nro_controlador: %d\n", nro_controlador);
	//Los controladores pueden manterse conectados incluso si el juego termina
	while (true) {
		atender_controlador(nro_controlador);
	}

	printf("Terminando thread de controlador %d\n", nro_controlador);
	return 0;
}

/* Acepta todas las conexiones entrantes */
void* aceptar_controladores(void *arg) {
	struct sockaddr_in remote;
	int t;

	for (int i = 0; i < MAX_CONTROLADORES; i++) {
		t = sizeof(remote);
		if ((s_controladores[i] = accept(sock_controladores, (struct sockaddr*) &remote, (socklen_t*) &t)) == -1) {
			// Si se acepta una conexión luego que se cerro el sock_controladores, se produce
			// un error "Bad file descriptor". En lugar de cerrar el servidor, simplemente
			// terminamos el thread.
			printf("aceptar_controladores: accept() devolvió -1, terminando\n");
			return NULL;

			// perror("aceptando la conexión entrante");
			// exit(1);			
		}
		printf("Se conectó un controlador. i = %d\n", i);
		int flag = 1;
		setsockopt(s_controladores[i],    /* socket affected */
				IPPROTO_TCP,     /* set option at TCP level */
				TCP_NODELAY,     /* name of option */
				(char *) &flag,  /* the cast is historical */
				sizeof(int));    /* length of option value */
		pthread_create(&threads_controladores[i], NULL, controlador, new int(i));
		num_threads_controladores++;
	}

	printf("aceptar_controladores: terminando\n");

	return NULL;
}

/* Para atender al i-esimo jugador */
void atender_jugador(int i) {
	int recibido;
	std::string resp;
	recibido = recv(s_jugadores[i], buf_jugadores[i], MAX_MSG_LENGTH, 0);
	if (recibido < 0) {
		perror("Recibiendo ");
		
	} else if (recibido > 0) {
		buf_jugadores[i][recibido]='\0';
		// Separo los mensajes por el caracter |

		char * pch = strtok(buf_jugadores[i], "|");
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
			send(s_jugadores[i],resp.c_str(), resp.length() +1, 0);
			
			// No muestro por pantalla los NOP, son muchos
			if (strstr(pch, "Nop") == NULL) {
				printf("Resultado %s\n", resp.c_str());
			}
			// Si ya se cual es el jugador
			if (ids[i] != -1) {
				bool hayEventos = model->hayEventos(ids[i]);
				while(hayEventos) {
					resp = decoder->encodeEvent(ids[i]);
					printf("Enviando evento %s_jugadores", resp.c_str());
					send(s_jugadores[i],resp.c_str(), resp.length() +1, 0);
					hayEventos = model->hayEventos(ids[i]);
				}
			}
			pch = strtok(NULL, "|");
		}
	} else { /* recibido == 0 */
		// Extraído de la man page de recv(): "If no messages are available to be received
		// and the peer hasperformed an orderly shutdown, recv() shall return 0."
		printf("Saliendo del thread del jugador %d.\n", i);
		pthread_exit(NULL);
	}
}

/* Punto de entrada de threads de jugador */
void *jugador(void *nro_jugador_ptr) {
	int nro_jugador = * (int*) nro_jugador_ptr;
	delete (int*) nro_jugador_ptr;

	bool sale = false;
	printf("Nuevo jugador aceptado! nro_jugador: %d \n", nro_jugador);
	while (!sale) {
		atender_jugador(nro_jugador);
		sale = model->termino();
	}

	printf("Terminando thread de jugador %d\n", nro_jugador);
	return 0;
}

/* Acepta todas las conexiones entrantes */
void acceptar_jugadores(pthread_t *threads) {
	struct sockaddr_in remote;

	printf("Iniciando aceptación de clientes \n");
	int t;
	for (int i = 0; i < n; i++) {
		t = sizeof(remote);
		if ((s_jugadores[i] = accept(sock_jugadores, (struct sockaddr*) &remote, (socklen_t*) &t)) == -1) {
			perror("aceptando la conexión entrante");
			exit(1);
		}
		printf("Paso el accept %d \n",i);
		ids[i] = -1;
		int flag = 1;
		setsockopt(s_jugadores[i],         /* socket affected */
				IPPROTO_TCP,     /* set option at TCP level */
				TCP_NODELAY,     /* name of option */
				(char *) &flag,  /* the cast is historical */
				sizeof(int));    /* length of option value */
		pthread_create(&threads[i], NULL, jugador, new int(i));
	}
}

void abrir_socket_jugadores(int port) {
	struct sockaddr_in name;

	/* Crear socket sobre el que se lee: dominio INET, protocolo TCP (STREAM). */
	sock_jugadores = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_jugadores < 0) {
		perror("abriendo socket");
		exit(1);
	}

	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(port);
	if (bind(sock_jugadores, (const struct sockaddr*) (&name), sizeof(name))) {
		perror("binding socket");
		exit(1);
	}

	/* Escuchar en el socket y permitir n conexiones en espera. */
	if (listen(sock_jugadores, n) == -1) {
		perror("escuchando");
		exit(1);
	}
}

void abrir_socket_controladores() {
	struct sockaddr_in name;

	/* Crear socket sobre el que se lee: dominio INET, protocolo TCP (STREAM). */
	sock_controladores = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_controladores < 0) {
		perror("abriendo socket");
		exit(1);
	}

	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede enviar aquí. */
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(CONTROLLER_PORT);
	if (bind(sock_controladores, (const struct sockaddr*) (&name), sizeof(name))) {
		perror("binding socket");
		exit(1);
	}

	/* Escuchar en el socket y permitir n conexiones en espera. */
	if (listen(sock_controladores, MAX_CONTROLADORES) == -1) {
		perror("escuchando");
		exit(1);
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

	printf("Corriendo...\n");

	// Atender controladores
	abrir_socket_controladores();
	pthread_t thread_aceptar_controladores;
	pthread_create(&thread_aceptar_controladores, NULL, aceptar_controladores, NULL);

	// Atender jugadores
	abrir_socket_jugadores(port);
	pthread_t threads_jugadores[n];
	acceptar_jugadores(threads_jugadores);

	// Esperar a que terminen todos los threads
    for (int i = 0; i < n; ++i) {
    	pthread_join(threads_jugadores[i], NULL);
    	printf("Joineó el thread del jugador %d\n", i);
    }
    
    // Imprimir los puntajes.
	printf("Los puntajes finales son:\n");
	model->printPuntajes();

    printf("Termino el juego, esperando a que se desconecten los controladores...\n");

    // Cerrar el socket de controladores para evitar que se conecten nuevos controladores.
    // Usamos shutdown en lugar de close para despertar al accept() en aceptar_controladores,
    // de acuerdo a lo que dice acá: http://stackoverflow.com/questions/2486335/wake-up-thread-blocked-on-accept-call
    shutdown(sock_controladores, SHUT_RDWR);

    // Esperar a que se desconecten todos los controladores
    for(int i = 0; i < num_threads_controladores; i++) {
    	pthread_join(threads_controladores[i], NULL);
    	printf("Joineó el thread del controlador %d\n", i);
    }

    printf("Cerrando\n");

    // Cerrar los sockets
	for (int i = 0; i < n; i++) close(s_jugadores[i]);
	for (int i = 0; i < MAX_CONTROLADORES; i++) close(s_controladores[i]);
	close(sock_jugadores);

	return 0;
}