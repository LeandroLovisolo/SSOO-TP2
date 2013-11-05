#ifndef __TIRO_H__
#define __TIRO_H__

#include <sys/time.h>

class Tiro {
	public:

		Tiro();
		bool es_posible_apuntar();
		bool es_posible_tocar();
		int getEta();
		void reiniciar();
		int tirar(int t_id,int x,int y);

		int				t_id;		/* a que jugador se hizo el tiro */
		struct timeval	stamp;		/* cuando se hizo el tiro */
		int				eta;		/* tiempo de espera hasta finalizar el tiro (en milisgeundos) */
		int				estado;		/* Libre o Apuntado */
	    int             x;
	    int             y;
};


#endif /* __TIRO_H__ */