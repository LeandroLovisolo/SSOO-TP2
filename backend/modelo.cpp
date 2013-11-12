#include <modelo.h>
//#ifdef DEBUG
#include <cstdio>
//#endif
#include <constantes.h>
#include <globales.h> 
#include <stdlib.h>
#include <cassert>

Modelo::Modelo(int njugadores, int tamtablero, int tamtotalbarcos){
	max_jugadores = njugadores;
	tamanio_tablero = tamtablero;
	tamanio_total_barcos = tamtotalbarcos;
	
	this->jugadores = new Jugador*[max_jugadores];
	this->eventos = new std::queue<Evento>[max_jugadores];
	this->mutexEventos = new RWLock[max_jugadores];
	this->tiros = new Tiro[max_jugadores];
	this->mutexJugadores = new RWLock[max_jugadores];
	this->mutexTiros = new RWLock[max_jugadores];
	for (int i = 0; i < max_jugadores; i++) {
		this->jugadores[i] = NULL;
	}
	this->cantidad_jugadores = 0;
	this->jugadores_listos = 0;
	this->jugando = SETUP;

}
Modelo::~Modelo() {
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			delete this->jugadores[i];
		}
	}
	delete[] this->mutexEventos;
	delete[] this->mutexJugadores;
	delete[] this->mutexTiros;

	delete[] this->jugadores;
	delete[] this->tiros;
	delete[] this->eventos;
}

/** Registra un nuevo jugador en la partida */
int Modelo::agregarJugador(std::string nombre) {
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;

	int nuevoid = 0;
	bool agregado = false;
	for (nuevoid = 0; nuevoid < max_jugadores; nuevoid++) {
		this->mutexJugadores[nuevoid].wlock();
		if(this->jugadores[nuevoid] == NULL) {
			this->jugadores[nuevoid] = new Jugador(nombre);
			this->mutexJugadores[nuevoid].wunlock();
			agregado = true;
			break;
		}
		this->mutexJugadores[nuevoid].wunlock();
	}
	if(!agregado) return -ERROR_MAX_JUGADORES;

	mutexCantidadJugadores.wlock();
	this->cantidad_jugadores++;
	mutexCantidadJugadores.wunlock();

	return nuevoid;
}

/** Intenta agregar un nuevo bote para el jugador indicado. 
	Si tiene exito y todos los jugadores terminaron lanza la rutina @Modelo::empezar
	Sino quita todos los barcos del usuario.
*/
error Modelo::ubicar(int t_id, int * xs, int *  ys, int tamanio) {
	//Wunlock, porque si está listo, quiero que escriba listo
	//Es mejor esto o liberar el rlock y hacer un wlock? o es lo mismo por ser al inicio?
	mutexJugando.wlock();
	if (this->jugando != SETUP) {
		mutexJugando.wunlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	//Ver si es necesario mutex
	if (this->jugadores[t_id] == NULL) {
		mutexJugando.wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	//Ver que onda con ubicar, si necesita mutex
	int retorno = this->jugadores[t_id]->ubicar(xs, ys, tamanio);
	if (retorno != ERROR_NO_ERROR){
		retorno = this->borrar_barcos(t_id);
	}
	//Si el jugador esta listo
	if (retorno == ERROR_NO_ERROR && this->jugadores[t_id]->listo()) {
		mutexJugadoresListos.wlock();
		this->jugadores_listos++;
		printf("Jugadores listos: %d \n",this->jugadores_listos);
		//Debería terminar acá el wlock y pedir un rlock?

		//Si ya estan listos todos los jugadores
		if(this->jugadores_listos == max_jugadores) {
			this->empezar();
			printf("Sale de empezar \n");
		}
		mutexJugadoresListos.wunlock();
	}
	mutexJugando.wunlock();
	return retorno;
}

/** Quita todos los barcos del usuario */
error Modelo::borrar_barcos(int t_id) {
	printf("Entro a borrar barcos! \n");
	if (this->jugando != SETUP) {
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	if (this->jugadores[t_id] == NULL) {
		return -ERROR_JUGADOR_INEXISTENTE;
	}
	error quitar = this->jugadores[t_id]->quitar_barcos();
	return quitar;
}

/** Comienza la fase de tiros
*/
error Modelo::empezar() {
	printf("Entra en empezar \n");
	//Por que entraría en otr lado que no sea empezar?
	if (this->jugando != SETUP) {
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			//Creo que hay que meter mutex en eventos
			Evento evento(0, i, 0, 0, EVENTO_START);
			printf("Consigo lock para el evento i \n");
			mutexEventos[i].wlock();
			this->eventos[i].push(evento);
			printf("Devuelvo lock para el evento i \n");
			mutexEventos[i].wunlock();
		}
	}
	this->jugando = DISPAROS;
	return ERROR_NO_ERROR;
	
}
/** LLamado al finalizar la partida.
	Se marca el juego como terminado y se le notifica a todos los participantes */
error Modelo::finalizar() {
	//Enviar un evento a todos avisando que termino.
	//Ver si es necesario mutex
	mutexJugando.rlock();
	if (this->jugando != DISPAROS) {
		mutexJugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;	
	}
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			//Mutex en eventos?
			Evento evento(0, i, 0, 0, EVENTO_END);
			this->eventos[i].push(evento);
		}
	}
	mutexJugando.runlock();
	this->jugando = FINALIZADO;
	return ERROR_NO_ERROR;
}


/** Para poder finalizar correctamente, necesito la confirmación
	de cada jugador de que sabe que terminamos de jugar. */
error Modelo::ack(int s_id){
	//Guardarme en cada jugador que termino de jugar.
	return this->jugadores[s_id]->ack();
}

//Si necesita locks
bool Modelo::termino() {
	mutexJugando.rlock();
	if (this->jugando == SETUP) {
		mutexJugando.runlock();
		return false;
	}
    for(int i = 0; i < max_jugadores; i++){
    	mutexJugadores[i].rlock();
        if(!this->jugadores[i]->termino()) {
        	mutexJugadores[i].runlock();
        	mutexJugando.runlock();
            return false;
        }
        mutexJugadores[i].runlock();
    }
    mutexJugando.runlock();
    return true;
}

/** @Deprecated */
error Modelo::reiniciar() {
	mutexJugando.wlock();
	for (int i = 0; i < max_jugadores; i++) {
		mutexJugadores[i].wlock();
		if (this->jugadores[i] != NULL) {
			this->jugadores[i]->reiniciar();
			this->tiros[i].reiniciar();
		}
		mutexJugadores[i].wunlock();
	}
	this->jugando = SETUP;
	mutexJugando.wunlock();
	return ERROR_NO_ERROR;
	
}

/** Desuscribir a un jugador del juego */
error Modelo::quitarJugador(int s_id) {
	//Ver cuando pasa
	mutexJugando.rlock();
	if (this->jugando != SETUP) {
		mutexJugando.runlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	mutexJugadores[s_id].wlock();
	if (this->jugadores[s_id] == NULL) {
		mutexJugadores[s_id].wunlock();
		mutexJugando.runlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}
	delete this->jugadores[s_id];
	this->jugadores[s_id] = NULL;

	mutexJugadores[s_id].wunlock();
	mutexJugando.runlock();
	return ERROR_NO_ERROR;
}

/** Intentar apuntar a la casilla de otro jugador.
	Solo comienza el tiro si el jugador actual puede disparar 
	y al otro jugador se le puede disparar
	*/
int Modelo::apuntar(int s_id, int t_id, int x, int y, int *eta) {
	mutexJugando.rlock();
	if (this->jugando != DISPAROS) {
		mutexJugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}
	//No quiero que me saquen el jugador mientras apunto
	//Voy a escribir al jugador (apuntando)
	//Quiero mantener que esté vivo al comenzar a apuntar
	mutexJugadores[s_id].wlock();
	mutexJugadores[t_id].wlock();
	if (this->jugadores[s_id] == NULL || this->jugadores[t_id] == NULL) {
		mutexJugando.runlock();
		mutexJugadores[s_id].wunlock();
		mutexJugadores[t_id].wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	if(!this->jugadores[s_id]->esta_vivo()) {
		mutexJugando.runlock();
		mutexJugadores[s_id].wunlock();
		mutexJugadores[t_id].wunlock();
		return -ERROR_JUGADOR_HUNDIDO;
	}

	int retorno = RESULTADO_APUNTADO_DENEGADO;

	//Quiero mantener que sea posible apuntar
	//Quiero hacer Wlock para realizar el tiro
	mutexTiros[s_id].wlock();
	if (this->tiros[s_id].es_posible_apuntar()) {
		retorno = this->jugadores[t_id]->apuntar(s_id, x, y);
		if (retorno == RESULTADO_APUNTADO_ACEPTADO) {
			*eta = this->tiros[s_id].tirar(t_id, x, y);
			Evento nuevoevento(s_id, t_id, x, y, CASILLA_EVENTO_INCOMING);

			//Voy a agregar un nuevo evento, WLock a eventos
			printf("Consigo lock para el evento i \n");
			mutexEventos[t_id].wlock();
			this->eventos[t_id].push(nuevoevento);
			mutexEventos[t_id].wunlock();
			printf("Devuelvo lock para el evento i \n");

		}
	}
	mutexJugadores[s_id].wunlock();
	mutexJugadores[t_id].wunlock();
	mutexTiros[s_id].wunlock();
	mutexJugando.runlock();
	return retorno;
	
}

/** Obtener un update de cuanto tiempo debo esperar para que se concrete el tiro */
int Modelo::dame_eta(int s_id) {
	//Quiero que se conserve el estado del juego
	mutexJugando.rlock();
	if (this->jugando != DISPAROS) {
		mutexJugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}
	//Quiero que se conserve el estado del jugador (que no me lo saquen)
	mutexJugadores[s_id].rlock();
	if (this->jugadores[s_id] == NULL) {

		mutexJugadores[s_id].runlock();
		mutexJugando.runlock();

		return -ERROR_JUGADOR_INEXISTENTE;
	}
	//Quiero que se conserven los tiros para conseguir el eta correcto
	mutexTiros[s_id].rlock();
	int eta = this->tiros[s_id].getEta();

	mutexTiros[s_id].runlock();
	mutexJugadores[s_id].runlock();
	mutexJugando.runlock();
	return eta;
}

/** Concretar el tiro efectivamente, solo tiene exito si ya trascurrió el eta.
	y e impacta con algo.*/
int Modelo::tocar(int s_id, int t_id) {
	//Quiero que se conserve el estado del juego
	mutexJugando.rlock();
	if (this->jugando != DISPAROS) {
		mutexJugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}

	//Voy a escribir en los dos jugadores y registrar el tiro
	//No quiero que me lo saquen mientras tiro, ver si es posible
	mutexJugadores[t_id].wlock();
	mutexJugadores[s_id].wlock();

	if (this->jugadores[s_id] == NULL || this->jugadores[t_id] == NULL) {
		mutexJugando.runlock();
		mutexJugadores[t_id].wunlock();
		mutexJugadores[s_id].wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;	
	}
	
	int retorno = -ERROR_ETA_NO_TRANSCURRIDO;

	//Rlock al tiro, no quiero que me lo modifiquen mientras hago el tiro
	mutexTiros[s_id].rlock();

	if (this->tiros[s_id].es_posible_tocar()) {
		int x = this->tiros[s_id].x;
		int y = this->tiros[s_id].y;
		bool murio = false;
		retorno = this->jugadores[t_id]->tocar(s_id, x, y, &murio);
		if (retorno == EMBARCACION_RESULTADO_TOCADO ||
			retorno == EMBARCACION_RESULTADO_HUNDIDO ||
			retorno == EMBARCACION_RESULTADO_HUNDIDO_M ||
			retorno == EMBARCACION_RESULTADO_AGUA ||
			retorno == EMBARCACION_RESULTADO_AGUA_H
			) {
			
			this->tiros[s_id].estado = TIRO_LIBRE;
			Evento evento(s_id, t_id, x, y, retorno);


			this->eventos[t_id].push(evento);
			//Evento para el tirador
			this->eventos[s_id].push(evento);

			if(murio){
				//Voy a escribir y checkear la cantidad de jugadores, WLock
				mutexCantidadJugadores.wlock();

				this->cantidad_jugadores--;
				if(this->cantidad_jugadores == 1){
                    printf("Dale viejo!\n");
					this->finalizar(); 
				}

				mutexCantidadJugadores.wunlock();
			}
		}
		if (retorno == EMBARCACION_RESULTADO_HUNDIDO) {
			this->jugadores[s_id]->agregar_puntaje(PUNTAJE_HUNDIDO);
		} else if (retorno == EMBARCACION_RESULTADO_HUNDIDO_M) {
			this->jugadores[s_id]->agregar_puntaje(PUNTAJE_HUNDIDO+PUNTAJE_MISMO_JUGADOR);
		} else if (retorno == EMBARCACION_RESULTADO_TOCADO) {
			this->jugadores[s_id]->agregar_puntaje(PUNTAJE_TOCADO);
		} else if (retorno == EMBARCACION_RESULTADO_AGUA_H) {
			this->jugadores[s_id]->agregar_puntaje(PUNTAJE_MAGALLANES);
		} 
	}
	mutexJugadores[t_id].wunlock();
	mutexJugadores[s_id].wunlock();
	mutexTiros[s_id].runlock();
	mutexJugando.runlock();
	return retorno;
}

#ifdef DEBUG
void Modelo::print() {
	printf("MODELO -- NJugadores %d, Jugando %d\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n", this->cantidad_jugadores, this->jugando);
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			this->jugadores[i]->print();
			printf( "Tiro: id %d, stamp (%lu, %lu), eta %d, estado %d\n", this->tiros[i]->t_id, this->tiros[i]->stamp.tv_sec, (long unsigned int)this->tiros[i]->stamp.tv_usec, this->tiros[i]->eta, this->tiros[i]->estado);
		}
		printf("\n");
	}
	
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
}
#endif

/******* RUTINAS DE EVENTOS  *************/
/*
	Los eventos se lockean por usuario (arreglo de RWLock por usuario)
*/
int Modelo::hayEventos(int s_id) {
	mutexEventos[s_id].rlock();
	int eventos = this->eventos[s_id].size() > 0;
	mutexEventos[s_id].runlock();

	return eventos;
}

Evento Modelo::dameEvento(int s_id) {
	//Lock para la cola de eventos asociada al usuario s_id
	printf("Consigo lock para el evento dameEvento \n");
	mutexEventos[s_id].wlock();
	//Ya no se usa más hayEventos, se copia el código para poner un wlock
	assert(this->eventos[s_id].size() > 0);
	Evento retorno = this->eventos[s_id].front();
	this->eventos[s_id].pop();
	mutexEventos[s_id].wunlock();
	printf("Devuelvo lock para el evento dameEvento \n");
    return retorno;
}


/** chequear si hay eventos nuevos para un jugador */
Evento Modelo::actualizar_jugador(int s_id) {
	printf("Consigo lock para el evento actualizar_jugador \n");

	//Voy a escribir en las colas dentro de tiros (Fijarse si hay algo mejor)
	mutexEventos[this->tiros[s_id].t_id].wlock();
	mutexEventos[s_id].wlock();
	int tocado = this->tocar(s_id, this->tiros[s_id].t_id);
	//Ya no se usa más hayEventos, se copia el código para poner un wlock
    if (this->eventos[s_id].size() > 0) {
    	Evento retorno = this->eventos[s_id].front();
		this->eventos[s_id].pop();
		printf("Devuelvo lock para el evento actualizar_jugador \n");
		mutexEventos[this->tiros[s_id].t_id].wunlock();
		mutexEventos[s_id].wunlock();
		return retorno;
    } else {
    	printf("Devuelvo lock para el evento actualizar_jugador \n");
		mutexEventos[this->tiros[s_id].t_id].wunlock();
		mutexEventos[s_id].wunlock();
		return Evento(s_id, -1, 0, 0, -tocado);
	}
}


