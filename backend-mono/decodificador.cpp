//
//  decodificador.cpp
//  Batalla Naval
//
//  Created by Federico Raimondo on 4/18/13.
//  Copyright (c) 2013 ar.dc.uba.so. All rights reserved.
//
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <decodificador.h>
#include <constantes.h>
#include <globales.h> 

using namespace std; 

Decodificador::Decodificador(Modelo *mod) {
    this->modelo = mod;
	this->jsonificador = new Jsonificador(mod);

}

Decodificador::~Decodificador() {
	delete this->jsonificador;
}

std::string  Decodificador::decodificar(const char * mensaje) {
	std::string retorno;
	//cout << "Parsing..." << endl;
	bool parsingSuccessful = this->reader.parse( mensaje, this->root );
	if (parsingSuccessful) {
		const char * name = this->root["Name"].asString().c_str();
		if (strcmp(JSON_CMD_SUSCRIBIR,name) == 0) {
			retorno = this->subscribe();
		} else if (strcmp(JSON_CMD_CONFIGURAR, name) == 0) {
			retorno = this->setup();
		} else if (strcmp(JSON_CMD_INFORMACION, name) == 0) {
			retorno = this->get_info();
		} else if (strcmp(JSON_CMD_COMENZAR, name) == 0) {
			retorno = this->start();
		} else if (strcmp(JSON_CMD_ACTUALIZACION, name) == 0) {
			retorno = this->get_update();
		} else if (strcmp(JSON_CMD_DESUSCRIBIR, name) == 0) {
			retorno = this->unsubscribe();
		} else if (strcmp(JSON_CMD_TIRO, name) == 0) {
			retorno = this->shoot();
		} else if (strcmp(JSON_CMD_NOP, name) == 0) {
			retorno = "";
		} else if (strcmp(JSON_CMD_SCORES, name) == 0) {
			retorno = this->scores();
        } else if (strcmp(JSON_CMD_ACK, name) == 0) {
            retorno = this->ack();
		} else {
			cout << "Comando no implementado: " << this->root["Name"].asString() << endl;
		}
	} else {
		cerr << "Jason error:";
		std::cerr << this->reader.getFormattedErrorMessages();
		retorno = -ERROR_JSON_ERROR;
	}
	return retorno;
}

int	Decodificador::dameIdJugador(const char * mensaje) {
	int retorno = -1;
	bool parsingSuccessful = this->reader.parse( mensaje, this->root );
	if (parsingSuccessful) {
		const char * name = this->root["Name"].asString().c_str();
		if (strcmp(JSON_CMD_SUSCRIBIR,name) == 0) {
			retorno = this->root["Data"]["Id"].asInt();
		}
	}
	return retorno;
}

std::string Decodificador::subscribe() {
	int retorno = this->modelo->agregarJugador(this->root["Data"]["Name"].asString());
	if (retorno < 0) {
		return this->jsonificador->error(retorno);
	}
	return this->jsonificador->subscribe_resp(retorno);
}

std::string Decodificador::setup() {
	error retorno = ERROR_NO_ERROR;
	int nboats = this->root["Data"]["boats"].size();
	int boatsize;
	int xs[MAX_BOAT_SIZE];
	int ys[MAX_BOAT_SIZE];
	
	//Por cada bote intenta ubicar
	for (int i = 0; i < nboats && retorno == ERROR_NO_ERROR; i++) {
		boatsize = this->root["Data"]["boats"][i].size();
		for (int j = 0; j < boatsize; j++) {
			xs[j] = this->root["Data"]["boats"][i][j]["x"].asInt();
			ys[j] = this->root["Data"]["boats"][i][j]["y"].asInt();
		}
		retorno = this->modelo->ubicar(this->root["Data"]["s_id"].asInt(), (int*)xs, (int*)ys, boatsize);
	}
	
	if (retorno == ERROR_NO_ERROR) {
		return this->jsonificador->setup_resp();
	} else {
		return this->jsonificador->error(retorno);
	}
}

std::string Decodificador::start() {
	error retorno = this->modelo->empezar();
	return this->jsonificador->error(retorno);
}

std::string Decodificador::get_info() {
	int t_id = this->root["Data"]["t_id"].asInt();
	return this->jsonificador->player_info(t_id);
}

std::string Decodificador::shoot() {
	int eta;
	int t_id = this->root["Data"]["t_id"].asInt();
	int s_id = this->root["Data"]["s_id"].asInt();
	int x = atoi(this->root["Data"]["coord"]["x"].asString().c_str());
	int y = atoi(this->root["Data"]["coord"]["y"].asString().c_str());
	char st = this->root["Data"]["status"].asString().c_str()[0];
	cout << t_id << " " << s_id<< " " <<x << " " << y << " " << st << endl;
	int retorno = this->modelo->apuntar(s_id, t_id, x, y, &eta);
	return this->jsonificador->shoot_resp(retorno, eta);
}

std::string Decodificador::ack() {
    int s_id = this->root["Data"]["s_id"].asInt();
    int retorno = this->modelo->ack(s_id);
    return this->jsonificador->error(retorno);
}


std::string Decodificador::get_update() {
	int s_id = this->root["Data"]["s_id"].asInt();
	Evento evento = this->modelo->actualizar_jugador(s_id);
	return this->jsonificador->update(evento);
}

std::string Decodificador::encodeEvent(int s_id) {
	Evento evento = this->modelo->dameEvento(s_id);
	return this->jsonificador->update(evento);
}

std::string Decodificador::unsubscribe() {
	int s_id = this->root["Data"]["s_id"].asInt();
	return this->jsonificador->error(this->modelo->quitarJugador(s_id));
}

std::string Decodificador::scores() {
	return this->jsonificador->scores();
}
