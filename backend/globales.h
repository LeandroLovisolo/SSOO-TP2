#ifndef __GLOBALES_H__
#define __GLOBALES_H__

#include <string>

extern int	max_jugadores;
extern int 	tamanio_tablero;
extern int	tamanio_total_barcos;

extern std::string traducciones[255];

extern void inicializar();

#ifdef DEBUG
#define dprintf(...) printf("DEBUG::");printf(__VA_ARGS__); printf("\n");
#else
#define dprintf(...)
#endif

#endif /* __GLOBALES_H__ */