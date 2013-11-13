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
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;

	mutexJugadores[t_id].wlock();
	if (this->jugadores[t_id] == NULL) {
		mutexJugadores[t_id].wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	int retorno = this->jugadores[t_id]->ubicar(xs, ys, tamanio);
	if (retorno != ERROR_NO_ERROR){
		retorno = this->borrar_barcos(t_id);
	}

	//Si el jugador esta listo
	if (retorno == ERROR_NO_ERROR && this->jugadores[t_id]->listo()) {
		mutexJugadoresListos.wlock();
		this->jugadores_listos++;

		//Si ya estan listos todos los jugadores
		if(this->jugadores_listos == max_jugadores) {
			this->empezar();
			printf("Sale de empezar \n");
		}
		mutexJugadoresListos.wunlock();
	}

	mutexJugadores[t_id].wunlock();
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
	mutexJugando.wlock();
	if (this->jugando != SETUP) {
		mutexJugando.wunlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			Evento evento(0, i, 0, 0, EVENTO_START);
			mutexEventos[i].wlock();
			this->eventos[i].push(evento);
			mutexEventos[i].wunlock();
		}
	}
	this->jugando = DISPAROS;
	mutexJugando.wunlock();
	return ERROR_NO_ERROR;
	
}
/** LLamado al finalizar la partida.
	Se marca el juego como terminado y se le notifica a todos los participantes */
error Modelo::finalizar() {
	//Enviar un evento a todos avisando que termino.
	if (this->jugando != DISPAROS) return -ERROR_JUEGO_NO_COMENZADO;
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			Evento evento(0, i, 0, 0, EVENTO_END);

			mutexEventos[i].wlock();
			this->eventos[i].push(evento);
			mutexEventos[i].wunlock();
		}
	}
	this->jugando = FINALIZADO;
	return ERROR_NO_ERROR;
}


/** Para poder finalizar correctamente, necesito la confirmación
	de cada jugador de que sabe que terminamos de jugar. */
error Modelo::ack(int s_id){
	//Guardarme en cada jugador que termino de jugar.
	mutexJugadores[s_id].wlock();
	error retorno = this->jugadores[s_id]->ack();
	mutexJugadores[s_id].wunlock();

	return retorno;
}

bool Modelo::termino() {
	mutexJugando.rlock();
	if(this->jugando == SETUP) {
		mutexJugando.runlock();
		return false;
	}
	mutexJugando.runlock();

    for(int i = 0; i < max_jugadores; i++){
    	mutexJugadores[i].rlock();
        if(!this->jugadores[i]->termino()) {
        	mutexJugadores[i].runlock();
            return false;
        }
        mutexJugadores[i].runlock();
    }
    return true;
}

/** @Deprecated */
error Modelo::reiniciar() {
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			this->jugadores[i]->reiniciar();
			this->tiros[i].reiniciar();
		}
	}
	this->jugando = SETUP;
	
	return ERROR_NO_ERROR;
	
}

/** Desuscribir a un jugador del juego */
error Modelo::quitarJugador(int s_id) {
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

	// No permito que me dispare a ninguno de los dos
	// barcos mientras proceso el tiro actual.
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

	mutexTiros[s_id].wlock();
	if (this->tiros[s_id].es_posible_apuntar()) {
		retorno = this->jugadores[t_id]->apuntar(s_id, x, y);
		if (retorno == RESULTADO_APUNTADO_ACEPTADO) {
			*eta = this->tiros[s_id].tirar(t_id, x, y);
			Evento nuevoevento(s_id, t_id, x, y, CASILLA_EVENTO_INCOMING);

			mutexEventos[t_id].wlock();
			this->eventos[t_id].push(nuevoevento);
			mutexEventos[t_id].wunlock();
		}
	}
	mutexTiros[s_id].wunlock();

	mutexJugadores[s_id].wunlock();
	mutexJugadores[t_id].wunlock();
	mutexJugando.runlock();
	return retorno;
	
}

/** Obtener un update de cuanto tiempo debo esperar para que se concrete el tiro */
int Modelo::dame_eta(int s_id) {
	if (this->jugando != DISPAROS) return -ERROR_JUEGO_NO_COMENZADO;
	if (this->jugadores[s_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	return this->tiros[s_id].getEta();
}

/** Concretar el tiro efectivamente, solo tiene exito si ya trascurrió el eta.
	y e impacta con algo.*/
int Modelo::tocar(int s_id, int t_id) {
	mutexJugando.wlock();
	if (this->jugando != DISPAROS) {
		mutexJugando.wunlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}

	if (this->jugadores[s_id] == NULL || this->jugadores[t_id] == NULL) {
		mutexJugando.wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;	
	}
	
	int retorno = -ERROR_ETA_NO_TRANSCURRIDO;

	mutexTiros[s_id].wlock();

	if (this->tiros[s_id].es_posible_tocar()) {
		mutexJugadores[t_id].wlock();
		mutexJugadores[s_id].wlock();

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

			//Evento para el tirado
			mutexEventos[t_id].wlock();
			this->eventos[t_id].push(evento);
			mutexEventos[t_id].wunlock();

			//Evento para el tirador
			mutexEventos[s_id].wlock();
			this->eventos[s_id].push(evento);
			mutexEventos[s_id].wunlock();

			if(murio) {
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

		mutexJugadores[t_id].wunlock();
		mutexJugadores[s_id].wunlock();
	}

	mutexTiros[s_id].wunlock();
	mutexJugando.wunlock();

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
	mutexEventos[s_id].wlock();

	// En lugar de llamar a hayEventos (como en el código mono-thread original)
	// verificamos acá mismo si la cola de eventos está vacía, porque hayEventos
	// ahora realiza un read-lock sobre la cola de eventos, pero ésta ya está
	// protegida con un write-lock al comienzo de dameEvento.
	assert(this->eventos[s_id].size() > 0);
	Evento retorno = this->eventos[s_id].front();
	this->eventos[s_id].pop();

	mutexEventos[s_id].wunlock();
    return retorno;
}


/** chequear si hay eventos nuevos para un jugador */
Evento Modelo::actualizar_jugador(int s_id) {
	int tocado = this->tocar(s_id, this->tiros[s_id].t_id);

	// En lugar de llamar a hayEventos (como en el código mono-thread original)
	// verificamos acá mismo si la cola de eventos está vacía, porque hayEventos
	// ahora realiza un read-lock sobre la cola de eventos, pero ésta ya está
	// protegida con un write-lock al comienzo de actualizar_jugador.
	mutexEventos[s_id].wlock();
	if (this->eventos[s_id].size() > 0) {
		Evento retorno = this->eventos[s_id].front();
		mutexEventos[s_id].wunlock();
		this->eventos[s_id].pop();
		return retorno;
	} else {
		mutexEventos[s_id].wunlock();
		return Evento(s_id, -1, 0, 0, -tocado);
	}
}
