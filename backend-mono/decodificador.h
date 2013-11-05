//
//  decodificador.h
//  Batalla Naval
//
//  Created by Federico Raimondo on 4/18/13.
//  Copyright (c) 2013 ar.dc.uba.so. All rights reserved.
//

#ifndef __DECODIFICADOR_H__
#define __DECODIFICADOR_H__

#include <iostream>
#include <json/json.h>
#include <modelo.h>
#include <jsonificador.h>

class Decodificador {
public:
	Decodificador(Modelo *mod);
	~Decodificador();
	std::string     decodificar(const char * mensaje);
    int             dameIdJugador(const char * mensaje);
	std::string		encodeEvent(int s_id);
	
private:
	Modelo			*modelo;
	Json::Reader	reader;
	Json::Value		root;
	Jsonificador    *jsonificador;
	std::string 	subscribe();
	std::string 	setup();
	std::string 	get_info();
	std::string 	shoot();
	std::string 	start();
	std::string 	get_update();
	std::string 	unsubscribe();
    std::string     scores();
    std::string     ack();
};


#endif /* defined(__DECODIFICADOR_H__) */
