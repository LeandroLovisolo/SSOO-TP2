#ifndef __EMBARCACION_H__
#define __EMBARCACION_H__

#include <elemento.h>

class Embarcacion : public Elemento {
	public:
		Embarcacion(int id, int tamanio);
		~Embarcacion();
		int			tocar(int s_id);
		int			dameResultado();
		
#ifdef DEBUG
		void		print();
#endif
};


#endif /* __EMBARCACION_H__ */