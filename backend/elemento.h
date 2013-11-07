#ifndef __ELEMENTO_H__
#define __ELEMENTO_H__


class Elemento {
	public:
						Elemento(int id, int nuevo_tamanio);
		virtual			~Elemento();
		virtual int		tocar(int s_id);
		bool			es_mismo_jugador(int s_id);
		bool			hundido();
		int				dameId();
		virtual	int		dameResultado();
#ifdef DEBUG
		virtual void	print();
#endif

	protected:
		int 			tamanio;			/* tamanio del elemento */
		int				toques;				/* cantidad de toques obtenidos */
		int				id_jugador;			/* id del jugador que hizo el primer toque */
		int				id;					/* id del elemento */
		bool			mismo_jugador;		/* si todos los toques los hizo el primer jugador */
};


#endif /* __ELEMENTO_H__ */