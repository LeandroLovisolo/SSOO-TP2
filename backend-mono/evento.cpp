#include <evento.h>
#ifdef DEBUG
#include <cstdio>
#endif
#include <stdlib.h>

Evento::Evento(int s_id, int t_id, int x, int y, int status) {
	this->s_id = s_id;
	this->t_id = t_id;
	this->x = x;
	this->y = y;
	this->status = status;
}
