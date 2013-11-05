
#include <tablero.h>
#ifdef DEBUG
#include <cstdio>
#endif

#include <globales.h>
#include <assert.h>

Tablero::Tablero(int tamanio, int totalbarcos) {
	assert(tamanio > 0);
	this->tamanio = tamanio;
	this->totalbarcos = totalbarcos;
	this->ocupado_barcos = 0;
	this->cantidad_barcos = 0;
	this->casillas = new Casilla[tamanio * tamanio]();
	this->barcos = new Embarcacion*[totalbarcos];
	for (int i = 0; i < totalbarcos; i++) {
		this->barcos[i] = NULL;
	}
	this->agua = new Elemento(0, (tamanio * tamanio) - totalbarcos);
	for (int i = 0; i < tamanio; i++) {
		for (int j = 0; j < tamanio; j++) {
			this->casillas[traducir_cord(i,j)].ubicar(agua);
		}
	}
}

Tablero::~Tablero() {
	delete[] this->casillas;
	for (int i = 0; i < this->totalbarcos; i++) {
		if (this->barcos[i] != NULL) {
			delete this->barcos[i];
		}
	}
	delete this->agua;
}

int Tablero::apuntar(int s_id, int x, int y) {
	return this->casillas[this->traducir_cord(x,y)].apuntar(s_id);
}

int Tablero::tocar(int s_id, int x, int y) {
	return this->casillas[this->traducir_cord(x,y)].tocar(s_id);
}

bool Tablero::mismo_jugador(int s_id, int x, int y) {
	return this->casillas[this->traducir_cord(x,y)].mismo_jugador(s_id);
}

error Tablero::ubicar(int * xs, int * ys, int tamanio) {
	assert(this->ocupado_barcos + tamanio <= this->totalbarcos && tamanio > 0);
	int retorno;
	int x;
	int y;
	bool aceptable = true;
	if (tamanio > 1) {
		if (xs[0] != xs[1] && ys[0] != ys[1]) {
			aceptable = false;
//			dprintf("Barco en diagonal");
		} else if (xs[0] == xs[1]) {
//			dprintf("Barco horizontal");
			for (int i = 0 ; i < tamanio && aceptable; i++) {
				for (int j = i+1; j < tamanio && aceptable; j++) {
					aceptable = aceptable && xs[i] == xs[j] && ys[i] != ys[j];				// Esta en posicion horizontal
				}
			}
		} else {
//			dprintf("Barco vertical");
			for (int i = 0 ; i < tamanio && aceptable; i++) {
				for (int j = i+1; j < tamanio && aceptable; j++) {
					aceptable = aceptable &&  xs[i] != xs[j] && ys[i] == ys[j];				// Esta en posicion vertical
				}
			}
		}
	}
	
	for (int i = 0; i < tamanio && aceptable; i++) {
		aceptable = aceptable && xs[i] < this->tamanio && ys[i] < this->tamanio;
	}
	
	
	for (int i = 0; i < tamanio && aceptable; i++) {
		x = xs[i];
		y = ys[i];
		aceptable = aceptable && this->casillas[traducir_cord(x, y)].dameId() == 0;			// Posicion actual
		if (y > 0) {
			aceptable = aceptable && this->casillas[traducir_cord(x, y-1)].dameId() == 0;	// Arriba
		}
		if (x > 0 && y > 0) {
			aceptable = aceptable && this->casillas[traducir_cord(x-1, y-1)].dameId() == 0;	// Izquierda arriba
		} 
		if (x > 0) {
			aceptable = aceptable && this->casillas[traducir_cord(x-1, y)].dameId() == 0;	// Izquierda
		}
		
		if (x > 0 && y < (tamanio -1)) {
			aceptable = aceptable && this->casillas[traducir_cord(x-1, y+1)].dameId() == 0;	// Izquierda abajo
		} 
		if (y < (tamanio -1)) {
			aceptable = aceptable && this->casillas[traducir_cord(x, y+1)].dameId() == 0;	// Abajo
		}
		if (x < (tamanio -1) && y < (tamanio -1)) {
			aceptable = aceptable && this->casillas[traducir_cord(x+1, y+1)].dameId() == 0;	// Derecha abajo
		} 
		if (x < (tamanio -1)) {
			aceptable = aceptable && this->casillas[traducir_cord(x+1, y)].dameId() == 0;	// Derecha
		}
		
		if (x < (tamanio -1) && y >0) {
			aceptable = aceptable && this->casillas[traducir_cord(x+1, y-1)].dameId() == 0;	// Derecha arriba
		} 
	}
	if (aceptable) {
		this->ocupado_barcos += tamanio;
		int id = ++this->cantidad_barcos;
		Embarcacion * nuevo = new Embarcacion(id, tamanio);
		for (int i = 0; i < tamanio; i++) {
			this->casillas[traducir_cord(xs[i], ys[i])].ubicar(nuevo);
		}
		this->barcos[id-1]=nuevo;
		retorno = ERROR_NO_ERROR;
	} else {
		retorno = -ERROR_POSICIONES_INVALIDAS;
	}
	return retorno;
}

inline 
int	Tablero::traducir_cord(int x, int y) {
	return x + y * this->tamanio;
}

bool Tablero::completo() {
	return this->ocupado_barcos == this->totalbarcos;
}


#ifdef DEBUG
#include <stdio.h>

void Tablero::print() {
	printf("TABLERO -- Tamanio %d, Total %d, Ocupados %d, Barcos %d\n=========================\n", this->tamanio, this->totalbarcos, this->ocupado_barcos, this->cantidad_barcos);
	for (int i = 0; i < tamanio; i++) {
		for (int j = 0; j < tamanio; j++) {
			printf("%d ", this->casillas[traducir_cord(i,j)].dameId());
		}
		printf("\n");
	}
	printf("=========================\n");
}
#endif