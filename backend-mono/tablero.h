#ifndef __TABLERO_H__
#define __TABLERO_H__


#include <casilla.h>
#include <embarcacion.h>
#include <constantes.h>
#include <vector>

class Tablero {
	public:
						Tablero(int tamanio, int totalbarcos);
						~Tablero();
		int				apuntar(int s_id, int x, int y);
		int				tocar(int s_id, int x, int y);
		bool			mismo_jugador(int s_id, int x, int y);
		error			ubicar(int * xs, int * ys, int tamanio);
		bool			completo();
#ifdef DEBUG
		void			print();
#endif
	private:
		int							tamanio;			/* tamanio del tablero */
		Casilla * 					casillas;			/* arreglo de tamanio x tamanio */
		Embarcacion **				barcos;				/* lista de barcos del tablero */
		int							totalbarcos;		/* cantidad de espacio que deben ocupar los barcos */
		int							ocupado_barcos;		/* espacio ocupado por los barcos en el tablero */
		int							cantidad_barcos;	/* cantidad de barcos en el tablero */
		int							traducir_cord(int x, int y);	/* de x y a offset lineal */
		Elemento *					agua;				/* El elemento que representa el agua en el tablero (Id = 0) */
	
		friend class				Jsonificador;		/* para poder acceder a los elementos */
};


#endif /* __TABLERO_H__ */