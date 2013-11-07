#include <embarcacion.h>
#include <constantes.h>

Embarcacion::Embarcacion(int id, int nuevo_tamanio) : Elemento(id, nuevo_tamanio) {

}

Embarcacion::~Embarcacion(){

}

int Embarcacion::tocar(int s_id) {
	int result = Elemento::tocar(s_id);
	if (this->toques == this->tamanio) {
		result = EMBARCACION_RESULTADO_HUNDIDO;
		if (Elemento::es_mismo_jugador(s_id)) {
			result = EMBARCACION_RESULTADO_HUNDIDO_M;
		}
	} else {
		result = EMBARCACION_RESULTADO_TOCADO;
	}
	return result;
}

int	Embarcacion::dameResultado() {
	if (this->hundido()) {
		return EMBARCACION_RESULTADO_HUNDIDO;
	} else {
		return EMBARCACION_RESULTADO_TOCADO;
	}
}

#ifdef DEBUG
#include <stdio.h>

void Embarcacion::print() {
	printf("EMBARCACION -- Id %d, Tamanio %d, Toques %d, Jugador %d, Mismo Jugador %d\n", this->id, this->tamanio, this->toques, this->id_jugador, this->mismo_jugador);
}
#endif