#ifndef __CASILLA_H__
#define __CASILLA_H__

#include <elemento.h>

class Casilla {
	public:
						Casilla();
						~Casilla();
		int				apuntar(int s_id);	
		int				tocar(int s_id);
		bool			mismo_jugador(int s_id);
		void			ubicar(Elemento * elemento);
		int				dameId();
		int				dameEstado();
        int             dameResultado();
#ifdef DEBUG
		void			print();
#endif
	private:
		int 			estado;				/* estado de la casilla */
		int				id_jugador;			/* jugador que apunto/toco */
		Elemento *		elemento;			/* elemento en la casilla */
};


#endif /* __CASILLA_H__ */