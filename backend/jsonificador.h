#ifndef __JSONIFICADOR_H__
#define __JSONIFICADOR_H__

#include <modelo.h>
#include <string>

class Jsonificador {
    public:
        Jsonificador(Modelo * mod);
        ~Jsonificador();
    
    /* Respuestas / Envios */
    std::string     subscribe_resp(int id);
    std::string     setup_resp();
    std::string     error(int err);
    std::string     start(void);
    std::string     player_info(int id);
    std::string     shoot_resp(int resp, int eta);
    
    std::string     update(Evento event);
    std::string     finish(void);
	std::string		scores(void);
    
    private:
        Modelo      * modelo;
        std::string traducir_estado_casilla(int estado);
};

#endif /* __JSONIFICADOR_H__ */
