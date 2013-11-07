#include <jugador.h>
#ifdef DEBUG
#include <cstdio>
#endif
#include <globales.h>

Jugador::Jugador(const std::string nuevo_nombre) {
	this->nombre = std::string(nuevo_nombre);
	this->tablero = new Tablero(tamanio_tablero, tamanio_total_barcos);
	this->puntaje = 0;
    this->termino_v = false;
	this->cantidad_barcos_flotando = 0;
}
Jugador::~Jugador() {
	delete this->tablero;
}

int	Jugador::apuntar(int s_id, int x, int y) {
	//Voy a escribir el tablero, lock
	mutexTablero.wlock();
	return this->tablero->apuntar(s_id, x, y);
	mutexTablero.wunlock();
}

int	Jugador::tocar(int s_id, int x, int y, bool* murio) {
	//Voy a escribir el tablero, lock
	//Ver si lock con tablero a toda la función
	mutexTablero.wlock();
	int retorno = this->tablero->tocar(s_id, x, y);
	mutexTablero.wunlock();

	//Leo y escribo cantidad_barcos_flotando
	//Acá hay un problema, ya que esta_vivo se utiliza en otro lugar
	mutexBarcosFlotando.wlock();
    bool vivo = this->esta_vivo();

	if (retorno == EMBARCACION_RESULTADO_HUNDIDO || retorno == EMBARCACION_RESULTADO_HUNDIDO_M) {
		//Antes de esta linea seguro estaba vivo
		this->cantidad_barcos_flotando--;
		//Puede haber muerto por este barco u otro?
		*murio = vivo != this->esta_vivo();
	}
	mutexBarcosFlotando.wunlock();
	return retorno;

}

//Esto parece no necesitar locks, se utiliza al inicio
error Jugador::ubicar(int * xs, int * ys, int tamanio) {
	error retorno = this->tablero->ubicar(xs, ys, tamanio);
	if (retorno == ERROR_NO_ERROR) {
		this->cantidad_barcos_flotando++;
	}
	return retorno;
}

void Jugador::agregar_puntaje(int mas) {
	//Escribo puntaje, wlock
	mutexPuntaje.wlock();
	this->puntaje += mas;
	mutexPuntaje.wunlock();
}
int	Jugador::dame_puntaje() {
	//Leo puntaje, rlock
	mutexPuntaje.rlock();
	return this->puntaje;
	mutexPuntaje.runlock();
}

error Jugador::ack() {
	//Terminó el jugador, escribo, wlock
	mutexTermino.wlock();
    this->termino_v = true;
    mutexTermino.wunlock();
    return ERROR_NO_ERROR;
}

bool Jugador::termino() {
	//Veo si terminó el jugador, rlock
	mutexTermino.rlock();
    return this->termino_v;
    mutexTermino.runlock();
}

bool Jugador::listo() {
	//Veo si estan puestos todos los barcos, rlock
	//Ver como se ponen todos los barcos, si es secuencial, no es necesario
	mutexTablero.rlock();
	return this->tablero->completo();
	mutexTablero.runlock();
}
void Jugador::reiniciar() {
	//Unica utilización en modelo, no se utiliza
	delete this->tablero;
	this->tablero = new Tablero(tamanio_tablero, tamanio_total_barcos);
	this->puntaje = 0;
	this->cantidad_barcos_flotando = 0;
}

//Ver sobre esto, se utiliza en otro lado
bool Jugador::esta_vivo(){
	return this->cantidad_barcos_flotando != 0;
}


std::string Jugador::dame_nombre() {
	//No se modifica, lock no necesario
    return this->nombre;
}

error Jugador::quitar_barcos() {
	//Quita solamente si no se está jugando, lock no necesario
	this->tablero = new Tablero(tamanio_tablero, tamanio_total_barcos);
	this->puntaje = 0;
	this->cantidad_barcos_flotando = 0;
	return ERROR_NO_ERROR;
}

#ifdef DEBUG

void Jugador::print() {
	printf("JUGADOR -- Nombre %s, Puntaje %d\n", this->nombre.c_str(), this->puntaje);
	this->tablero->print();
}
#endif
