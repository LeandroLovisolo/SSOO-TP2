#include <tiro.h>
#ifdef DEBUG
#include <cstdio>
#endif
#include <stdlib.h>

#include <constantes.h>


Tiro::Tiro() {
	this->t_id = JUGADOR_SINDEFINIR;
	this->stamp.tv_sec = 0;
	this->stamp.tv_usec = 0;
	this->eta = 0;
	this->estado = TIRO_LIBRE;
	this->x = -1;
	this->y = -1;
}

void Tiro::reiniciar() {
	this->t_id = JUGADOR_SINDEFINIR;
	this->stamp.tv_sec = 0;
	this->stamp.tv_usec = 0;
	this->eta = 0;
	this->estado = TIRO_LIBRE;	
}

bool Tiro::es_posible_apuntar() {
	return this->estado == TIRO_LIBRE;
}
int Tiro::getEta() {
	if (this->estado != TIRO_APUNTADO) return -ERROR_ESTADO_INCORRECTO;
	return this->eta;
}


bool  Tiro::es_posible_tocar() {
	bool es_posible_tocar = this->estado == TIRO_APUNTADO;
	if (es_posible_tocar) {
		struct timeval ahora;
		struct timeval * antes = &this->stamp;
		gettimeofday(&ahora, NULL);
		int msec = (ahora.tv_sec - antes->tv_sec) * 1000;
		msec += (ahora.tv_usec - antes->tv_usec) / 1000;
		
		if (msec < this->eta) {
			es_posible_tocar = false;
		}
	}
	return es_posible_tocar;
}

int Tiro::tirar(int t_id,int x,int y) {
	this->t_id = t_id;
	gettimeofday(&this->stamp, NULL);
	this->eta = rand() % (MAX_ETA) + MIN_ETA;
	this->estado = TIRO_APUNTADO;
	this->x = x;
	this->y = y;
	return this->eta;
}