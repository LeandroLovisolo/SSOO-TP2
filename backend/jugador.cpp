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
	return this->tablero->apuntar(s_id, x, y);
}

int	Jugador::tocar(int s_id, int x, int y, bool* murio) {
	int retorno = this->tablero->tocar(s_id, x, y);
    bool vivo = this->esta_vivo();

	if (retorno == EMBARCACION_RESULTADO_HUNDIDO || retorno == EMBARCACION_RESULTADO_HUNDIDO_M) {
		//Antes de esta linea seguro estaba vivo
		this->cantidad_barcos_flotando--;
		//Puede haber muerto por este barco u otro?
		*murio = vivo != this->esta_vivo();
	}
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
	this->puntaje += mas;
}
int	Jugador::dame_puntaje() {
	return this->puntaje;
}

error Jugador::ack() {
	//Terminó el jugador, escribo, wlock
    this->termino_v = true;
    return ERROR_NO_ERROR;
}

bool Jugador::termino() {
	//Veo si terminó el jugador, rlock
    return this->termino_v;
}

bool Jugador::listo() {
	return this->tablero->completo();
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
