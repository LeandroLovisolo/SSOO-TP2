#ifndef __EVENTO_H__
#define __EVENTO_H__

class Evento {

	public:
		Evento(int s_id, int t_id, int x, int y, int status);
		int				t_id;
		int				x;
		int				y;
		int				status;
		int				s_id;
};



#endif /* __EVENTO_H__ */