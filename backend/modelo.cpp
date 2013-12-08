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
	for (int i = 0; i < max_jugadores; i++) {
		this->jugadores[i] = NULL;
	}
	this->eventos = new std::queue<Evento>[max_jugadores];
	this->tiros = new Tiro[max_jugadores];
	this->cantidad_jugadores = 0;
	this->jugadores_listos = 0;
	this->jugando = SETUP;

	this->locks_eventos = new RWLock[max_jugadores];
	this->locks_jugadores = new RWLock[max_jugadores];
}
Modelo::~Modelo() {
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			delete this->jugadores[i];
		}
	}
	delete[] this->jugadores;
	delete[] this->eventos;
	delete[] this->tiros;

	delete[] this->locks_eventos;
	delete[] this->locks_jugadores;
}

/** Registra un nuevo jugador en la partida */
int Modelo::agregarJugador(std::string nombre) {
	lock_jugando.rlock();
	if (this->jugando != SETUP) {
		lock_jugando.runlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}

	int nuevoid = 0;
	bool agregado = false;
	for (nuevoid = 0; nuevoid < max_jugadores; nuevoid++) {
		this->locks_jugadores[nuevoid].wlock();
		if(this->jugadores[nuevoid] == NULL) {
			this->jugadores[nuevoid] = new Jugador(nombre);
			this->locks_jugadores[nuevoid].wunlock();
			agregado = true;
			break;
		}
		this->locks_jugadores[nuevoid].wunlock();
	}
	if(!agregado) {
		lock_jugando.runlock();
		return -ERROR_MAX_JUGADORES;
	}

	lock_cantidad_jugadores.wlock();
	this->cantidad_jugadores++;
	lock_cantidad_jugadores.wunlock();
	
	lock_jugando.runlock();
	return nuevoid;
}

/** Intenta agregar un nuevo bote para el jugador indicado. 
	Si tiene exito y todos los jugadores terminaron lanza la rutina @Modelo::empezar
	Sino quita todos los barcos del usuario.
*/
error Modelo::ubicar(int t_id, int * xs, int *  ys, int tamanio) {
	lock_jugando.wlock();
	if (this->jugando != SETUP) {
		lock_jugando.wunlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}

	locks_jugadores[t_id].wlock();
	if (this->jugadores[t_id] == NULL) {
		locks_jugadores[t_id].wunlock();
		lock_jugando.wunlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	int retorno = this->jugadores[t_id]->ubicar(xs, ys, tamanio);
	if (retorno != ERROR_NO_ERROR){
		this->borrar_barcos(t_id);
	}

	//Si el jugador esta listo
	if (retorno == ERROR_NO_ERROR && this->jugadores[t_id]->listo()) {
		lock_jugadores_listos.wlock();
		this->jugadores_listos++;

		//Si ya estan listos todos los jugadores
		if(this->jugadores_listos == max_jugadores) {
			this->_empezar();
			printf("Sale de empezar \n");
		}
		lock_jugadores_listos.wunlock();
	}

	locks_jugadores[t_id].wunlock();
	lock_jugando.wunlock();
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
	lock_jugando.wlock();
	error out = _empezar();
	lock_jugando.wunlock();
	return out;
}

error Modelo::_empezar() {
	if (this->jugando != SETUP) {
		return -ERROR_JUEGO_EN_PROGRESO;
	}
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			Evento evento(0, i, 0, 0, EVENTO_START);
			locks_eventos[i].wlock();
			this->eventos[i].push(evento);
			locks_eventos[i].wunlock();
		}
	}
	this->jugando = DISPAROS;
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

			locks_eventos[i].wlock();
			this->eventos[i].push(evento);
			locks_eventos[i].wunlock();
		}
	}
	this->jugando = FINALIZADO;
	return ERROR_NO_ERROR;
}


/** Para poder finalizar correctamente, necesito la confirmación
	de cada jugador de que sabe que terminamos de jugar. */
error Modelo::ack(int s_id){
	//Guardarme en cada jugador que termino de jugar.
	locks_jugadores[s_id].wlock();
	error retorno = this->jugadores[s_id]->ack();
	locks_jugadores[s_id].wunlock();

	return retorno;
}

bool Modelo::termino() {
	lock_jugando.rlock();
	if(this->jugando == SETUP) {
		lock_jugando.runlock();
		return false;
	}

    for(int i = 0; i < max_jugadores; i++){
    	locks_jugadores[i].rlock();
        if(!this->jugadores[i]->termino()) {
        	locks_jugadores[i].runlock();
        	lock_jugando.runlock();
            return false;
        }
        locks_jugadores[i].runlock();
    }
    lock_jugando.runlock();
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
	lock_jugando.rlock();
	if (this->jugando != SETUP) {
		lock_jugando.runlock();
		return -ERROR_JUEGO_EN_PROGRESO;
	}

	locks_jugadores[s_id].wlock();
	if (this->jugadores[s_id] == NULL) {
		locks_jugadores[s_id].wunlock();
		lock_jugando.runlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	delete this->jugadores[s_id];
	this->jugadores[s_id] = NULL;

	locks_jugadores[s_id].wunlock();
	lock_jugando.runlock();
	return ERROR_NO_ERROR;
}

/** Intentar apuntar a la casilla de otro jugador.
	Solo comienza el tiro si el jugador actual puede disparar 
	y al otro jugador se le puede disparar
	*/
int Modelo::apuntar(int s_id, int t_id, int x, int y, int *eta) {
	lock_jugando.rlock();
	if (this->jugando != DISPAROS) {
		lock_jugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}

	// No permito que me dispare a ninguno de los dos
	// barcos mientras proceso el tiro actual.
	// Solo hago el lock si no se apunta a si mismo, si no, existe deadlock
	// Lock a los jugadores los jugadores
	if(s_id < t_id) {
		locks_jugadores[s_id].wlock();
		locks_jugadores[t_id].wlock();
	}
	else {
		locks_jugadores[t_id].wlock();
		if(s_id != t_id) {
			locks_jugadores[s_id].wlock();
		}
	}

	if (this->jugadores[s_id] == NULL || this->jugadores[t_id] == NULL) {
		lock_jugando.runlock();

		// Unlock los jugadores
		if(s_id < t_id) {
			locks_jugadores[t_id].wunlock();
			locks_jugadores[s_id].wunlock();
		}
		else {
			if(s_id != t_id) {
				locks_jugadores[s_id].wunlock();
			}
			locks_jugadores[t_id].wunlock();
		}

		return -ERROR_JUGADOR_INEXISTENTE;
	}

	if(!this->jugadores[s_id]->esta_vivo()) {
		lock_jugando.runlock();

		// Unlock los jugadores
		if(s_id < t_id) {
			locks_jugadores[t_id].wunlock();
			locks_jugadores[s_id].wunlock();
		}
		else {
			if(s_id != t_id) {
				locks_jugadores[s_id].wunlock();
			}
			locks_jugadores[t_id].wunlock();
		}

		return -ERROR_JUGADOR_HUNDIDO;
	}

	int retorno = RESULTADO_APUNTADO_DENEGADO;

	if (this->tiros[s_id].es_posible_apuntar()) {
		retorno = this->jugadores[t_id]->apuntar(s_id, x, y);
		if (retorno == RESULTADO_APUNTADO_ACEPTADO) {
			*eta = this->tiros[s_id].tirar(t_id, x, y);
			Evento nuevoevento(s_id, t_id, x, y, CASILLA_EVENTO_INCOMING);

			locks_eventos[t_id].wlock();
			this->eventos[t_id].push(nuevoevento);
			locks_eventos[t_id].wunlock();
		}
	}
	// Unlock los jugadores
	if(s_id < t_id) {
		locks_jugadores[t_id].wunlock();
		locks_jugadores[s_id].wunlock();
	}
	else {
		if(s_id != t_id) {
			locks_jugadores[s_id].wunlock();
		}
		locks_jugadores[t_id].wunlock();
	}

	lock_jugando.runlock();
	return retorno;
	
}

/** Obtener un update de cuanto tiempo debo esperar para que se concrete el tiro */
int Modelo::dame_eta(int s_id) {
	lock_jugando.rlock();
	if (this->jugando != DISPAROS) {
		lock_jugando.runlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}

	locks_jugadores[s_id].rlock();

	if (this->jugadores[s_id] == NULL) {
		lock_jugando.runlock();
		locks_jugadores[s_id].runlock();
		return -ERROR_JUGADOR_INEXISTENTE;
	}

	int eta = this->tiros[s_id].getEta();
	locks_jugadores[s_id].runlock();

	return eta;
}

/** Concretar el tiro efectivamente, solo tiene exito si ya trascurrió el eta.
	y e impacta con algo.*/
int Modelo::tocar(int s_id, int t_id) {
	lock_jugando.wlock();
	if (this->jugando != DISPAROS) {
		lock_jugando.wunlock();
		return -ERROR_JUEGO_NO_COMENZADO;
	}

	if(s_id < t_id) {
		locks_jugadores[s_id].wlock();
		locks_jugadores[t_id].wlock();
	}
	else {
		locks_jugadores[t_id].wlock();
		if(s_id != t_id) {
			locks_jugadores[s_id].wlock();
		}
	}

	if (this->jugadores[s_id] == NULL || this->jugadores[t_id] == NULL) {
		lock_jugando.wunlock();
		if(s_id < t_id) {
			locks_jugadores[t_id].wunlock();
			locks_jugadores[s_id].wunlock();
		}
		else {
			if(s_id != t_id) {
				locks_jugadores[s_id].wunlock();
			}
			locks_jugadores[t_id].wunlock();
		}
		return -ERROR_JUGADOR_INEXISTENTE;	
	}
	
	int retorno = -ERROR_ETA_NO_TRANSCURRIDO;

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

			//Evento para el tirado
			if(s_id != t_id) {
				locks_eventos[t_id].wlock();
				this->eventos[t_id].push(evento);
				locks_eventos[t_id].wunlock();
			}

			//Evento para el tirador
			locks_eventos[s_id].wlock();
			this->eventos[s_id].push(evento);
			locks_eventos[s_id].wunlock();

			if(murio) {
				lock_cantidad_jugadores.wlock();
				this->cantidad_jugadores--;
				if(this->cantidad_jugadores == 1){
                    printf("Dale viejo!\n");
					this->finalizar(); 
				}
				lock_cantidad_jugadores.wunlock();
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

		// Unlock los jugadores
		if(s_id < t_id) {
			locks_jugadores[t_id].wunlock();
			locks_jugadores[s_id].wunlock();
		}
		else {
			if(s_id != t_id) {
				locks_jugadores[s_id].wunlock();
			}
			locks_jugadores[t_id].wunlock();
		}
	}

	lock_jugando.wunlock();

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
	locks_eventos[s_id].rlock();
	int eventos = this->eventos[s_id].size() > 0;
	locks_eventos[s_id].runlock();

	return eventos;
}

Evento Modelo::dameEvento(int s_id) {
	locks_eventos[s_id].wlock();

	// En lugar de llamar a hayEventos (como en el código mono-thread original)
	// verificamos acá mismo si la cola de eventos está vacía, porque hayEventos
	// ahora realiza un read-lock sobre la cola de eventos, pero ésta ya está
	// protegida con un write-lock al comienzo de dameEvento.
	assert(this->eventos[s_id].size() > 0);
	Evento retorno = this->eventos[s_id].front();
	this->eventos[s_id].pop();

	locks_eventos[s_id].wunlock();
    return retorno;
}


/** chequear si hay eventos nuevos para un jugador */
Evento Modelo::actualizar_jugador(int s_id) {
	int tocado = this->tocar(s_id, this->tiros[s_id].t_id);

	// En lugar de llamar a hayEventos (como en el código mono-thread original)
	// verificamos acá mismo si la cola de eventos está vacía, porque hayEventos
	// ahora realiza un read-lock sobre la cola de eventos, pero ésta ya está
	// protegida con un write-lock al comienzo de actualizar_jugador.
	locks_eventos[s_id].wlock();
	if (this->eventos[s_id].size() > 0) {
		Evento retorno = this->eventos[s_id].front();
		locks_eventos[s_id].wunlock();
		this->eventos[s_id].pop();
		return retorno;
	} else {
		locks_eventos[s_id].wunlock();
		return Evento(s_id, -1, 0, 0, -tocado);
	}
}

void Modelo::printPuntajes() {
	for(int i = 0; i < jugadores_listos; i++) {
		printf("Nombre: %s - Puntaje: %d\n", jugadores[i]->dame_nombre().c_str(), jugadores[i]->dame_puntaje());
	}
}