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
	this->tiros = new Tiro[max_jugadores];
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
	delete[] this->jugadores;
	delete[] this->tiros;
	delete[] this->eventos;
}

/** Registra un nuevo jugador en la partida */
int Modelo::agregarJugador(std::string nombre) {
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;
	int nuevoid = 0;
	for (nuevoid = 0; nuevoid < max_jugadores && this->jugadores[nuevoid] != NULL; nuevoid++);
	
	if (this->jugadores[nuevoid] != NULL) return -ERROR_MAX_JUGADORES;
	
	this->jugadores[nuevoid] = new Jugador(nombre);
	this->cantidad_jugadores++;
	
	return nuevoid;
}

/** Intenta agregar un nuevo bote para el jugador indicado. 
	Si tiene exito y todos los jugadores terminaron lanza la rutina @Modelo::empezar
	Sino quita todos los barcos del usuario.
*/
error Modelo::ubicar(int t_id, int * xs, int *  ys, int tamanio) {
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;
	if (this->jugadores[t_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	int retorno = this->jugadores[t_id]->ubicar(xs, ys, tamanio);
	if (retorno != ERROR_NO_ERROR){
		this->borrar_barcos(t_id);
	}
	//Si el jugador esta listo
	if (retorno == ERROR_NO_ERROR && this->jugadores[t_id]->listo()) {
		this->jugadores_listos++;
		//Si ya estan listos todos los jugadores
		if(this->jugadores_listos == max_jugadores) {
			this->empezar();
		}
	}
	return retorno;
}

/** Quita todos los barcos del usuario */
error Modelo::borrar_barcos(int t_id) {
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;
	if (this->jugadores[t_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	return this->jugadores[t_id]->quitar_barcos();
}

/** Comienza la fase de tiros
*/
error Modelo::empezar() {
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;
	for (int i = 0; i < max_jugadores; i++) {
		if (this->jugadores[i] != NULL) {
			Evento evento(0, i, 0, 0, EVENTO_START);
			this->eventos[i].push(evento);
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
			this->eventos[i].push(evento);
		}
	}
	this->jugando = FINALIZADO;
	return ERROR_NO_ERROR;
}


/** Para poder finalizar correctamente, necesito la confirmación
	de cada jugador de que sabe que terminamos de jugar. */
error Modelo::ack(int s_id){
	//Guardarme en cada jugador que termino de jugar.
	return this->jugadores[s_id]->ack();
}

bool Modelo::termino() {
    for(int i = 0; i < max_jugadores; i++){
        if(!this->jugadores[i]->termino())
            return false;
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
	if (this->jugando != SETUP) return -ERROR_JUEGO_EN_PROGRESO;
	if (this->jugadores[s_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	delete this->jugadores[s_id];
	
	this->jugadores[s_id] = NULL;
	
	return ERROR_NO_ERROR;
}

/** Intentar apuntar a la casilla de otro jugador.
	Solo comienza el tiro si el jugador actual puede disparar 
	y al otro jugador se le puede disparar
	*/
int Modelo::apuntar(int s_id, int t_id, int x, int y, int *eta) {
	if (this->jugando != DISPAROS) return -ERROR_JUEGO_NO_COMENZADO;
	if (this->jugadores[s_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	if (this->jugadores[t_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	if (! this->jugadores[s_id]->esta_vivo()) return -ERROR_JUGADOR_HUNDIDO;
	int retorno = RESULTADO_APUNTADO_DENEGADO;
	if (this->tiros[s_id].es_posible_apuntar()) {
		retorno = this->jugadores[t_id]->apuntar(s_id, x, y);
		if (retorno == RESULTADO_APUNTADO_ACEPTADO) {
			*eta = this->tiros[s_id].tirar(t_id, x, y);
			Evento nuevoevento(s_id, t_id, x, y, CASILLA_EVENTO_INCOMING);
			this->eventos[t_id].push(nuevoevento);   
		}
	}
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
	if (this->jugando != DISPAROS) return -ERROR_JUEGO_NO_COMENZADO;
	if (this->jugadores[s_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;
	if (this->jugadores[t_id] == NULL) return -ERROR_JUGADOR_INEXISTENTE;

	
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
			this->eventos[t_id].push(evento);
			//Evento para el tirador
			this->eventos[s_id].push(evento);
			if(murio){
				this->cantidad_jugadores--;
				if(this->cantidad_jugadores == 1){
                    printf("Dale viejo!\n");
					this->finalizar(); 
				}
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
int Modelo::hayEventos(int s_id) {
	return this->eventos[s_id].size() > 0;
}

Evento Modelo::dameEvento(int s_id) {
	assert(this->hayEventos(s_id));
	Evento retorno = this->eventos[s_id].front();
	this->eventos[s_id].pop();
    return retorno;
}


/** chequear si hay eventos nuevos para un jugador */
Evento Modelo::actualizar_jugador(int s_id) {
	int tocado = this->tocar(s_id, this->tiros[s_id].t_id);
    if (this->hayEventos(s_id) > 0) {
    	Evento retorno = this->eventos[s_id].front();
		this->eventos[s_id].pop();
		return retorno;
    } else {
		return Evento(s_id, -1, 0, 0, -tocado);
	}
}


