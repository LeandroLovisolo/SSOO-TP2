#include <casilla.h>
#ifdef DEBUG
#include <cstdio>
#endif

#include <constantes.h>
#include <stdlib.h>
#include <assert.h>
#include <globales.h>



Casilla::Casilla(){
	this->estado = CASILLA_ESTADO_LIBRE;
	this->id_jugador = JUGADOR_SINDEFINIR;
	this->elemento = NULL;
}

Casilla::~Casilla(){
}

int Casilla::apuntar(int s_id) {
	assert(this->elemento != NULL);
	int retorno = ERROR_NO_ERROR;
	if (this->estado != CASILLA_ESTADO_LIBRE) {
		retorno = RESULTADO_APUNTADO_DENEGADO;
	} else {
		this->estado = CASILLA_ESTADO_APUNTADO;
		this->id_jugador = s_id;
		retorno = RESULTADO_APUNTADO_ACEPTADO;
	}
	return retorno;

}

int Casilla::tocar(int s_id) {
	assert(this->elemento != NULL);
	int retorno = ERROR_NO_ERROR;
	if (this->estado != CASILLA_ESTADO_APUNTADO) {
		retorno = -ERROR_ESTADO_INCORRECTO;
	} else if  (this->id_jugador != s_id) {
		retorno = -ERROR_JUGADOR_INCORRECTO;
	} else {
		this->estado = CASILLA_ESTADO_TOCADO;
		retorno = this->elemento->tocar(s_id);
	}
	return retorno;
}

bool Casilla::mismo_jugador(int s_id) {
	assert(this->elemento != NULL);
	return this->elemento->es_mismo_jugador(s_id);

}

void Casilla::ubicar(Elemento * nuevo_elemento) {
	assert(nuevo_elemento != NULL);
	this->elemento = nuevo_elemento;
}

int Casilla::dameId() {
	assert(this->elemento != NULL);
	return this->elemento->dameId();
}

int Casilla::dameEstado() {
    return this->estado;
}

int Casilla::dameResultado() {
    if (this->estado != CASILLA_ESTADO_TOCADO) return -ERROR_ESTADO_INCORRECTO;
	assert(this->elemento != NULL);
	return this->elemento->dameResultado();
}


#ifdef DEBUG

void Casilla::print() {
	printf("CASILLA -- Estado %s, Jugador %d, Elemento %p\n", traducciones[this->estado].c_str(), this->id_jugador, this->elemento);
}
#endif
