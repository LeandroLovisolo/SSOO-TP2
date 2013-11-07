#ifndef __JUGADOR_H__
#define __JUGADOR_H__

#include <string>
#include <tablero.h>
#include "RWLock.h"

class Jugador {
	public:
						Jugador(const std::string nombre);
						~Jugador();
		int				apuntar(int s_id, int x, int y);
		int				tocar(int s_id, int x, int y, bool* murio);
		error			ubicar(int * xs, int * ys, int tamanio);
		void			agregar_puntaje(int mas);
		int				dame_puntaje();
		bool			listo();
		void 			reiniciar();
		bool			esta_vivo();
		std::string		dame_nombre();
		error		    quitar_barcos();
        error           ack();
        bool            termino();
#ifdef DEBUG
		void			print();
#endif
	private:
		RWLock			mutexNombre;
		RWLock			mutexTablero;
		RWLock			mutexPuntaje;
		RWLock			mutexBarcosFlotando;
		RWLock			mutexTermino;

		std::string 	nombre;						/* nombre del jugador */
		Tablero * 		tablero;					/* tablero del jugador */
		int				puntaje;					/* puntaje del jugador */
		int				cantidad_barcos_flotando;	/* cantidad de barcos del jugador */
	    bool            termino_v;
		friend class	Jsonificador;					/* para poder acceder a los elementos */

};


#endif /* __JUGADOR_H__ */
