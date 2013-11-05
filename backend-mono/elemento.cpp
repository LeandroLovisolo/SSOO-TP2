#include <elemento.h>

#ifdef DEBUG
#include <cstdio>
#endif

#include <constantes.h>
#include <assert.h>



Elemento::Elemento(int id, int nuevo_tamanio) {
	assert(nuevo_tamanio > 0);
	this->id = id;
	this->tamanio = nuevo_tamanio;
	this->toques = 0;
	this->id_jugador = JUGADOR_SINDEFINIR;
	this->mismo_jugador = true;
}

Elemento::~Elemento() {

}

int Elemento::dameId() {
	return this->id;
}
	
int Elemento::tocar(int s_id) {
	assert(this->toques < this->tamanio);
	this->toques ++;
	if (this->toques == 1) {
		this->mismo_jugador = true;
		this->id_jugador = s_id;
	} else {
		this->mismo_jugador = this->mismo_jugador && this->id_jugador == s_id;
	}
	int retorno = EMBARCACION_RESULTADO_AGUA;
	if (this->hundido()) {
		retorno = EMBARCACION_RESULTADO_AGUA_H;
	}
	return retorno;
}

bool Elemento::es_mismo_jugador(int s_id) {
	return this->mismo_jugador && this->id_jugador == s_id;
}

bool Elemento::hundido() {
	return this->toques == this->tamanio;
}


int	Elemento::dameResultado() {
	if (this->hundido()) {
		return EMBARCACION_RESULTADO_AGUA_H;
	} else {
		return EMBARCACION_RESULTADO_AGUA;
	}
}

#ifdef DEBUG

void Elemento::print() {
	printf("ELEMENTO -- Id %d, Tamanio %d, Toques %d, Jugador %d, Mismo Jugador %d\n",  this->id, this->tamanio, this->toques, this->id_jugador, this->mismo_jugador);
}
#endif