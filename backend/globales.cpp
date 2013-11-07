#include <globales.h>
#include <constantes.h>

int	max_jugadores = 1;
int tamanio_tablero = 5;
int tamanio_total_barcos = 7; /* 4 + 3 */



std::string traducciones[255];

void inicializar() {
	traducciones[EMBARCACION_RESULTADO_TOCADO] = "T";
	traducciones[EMBARCACION_RESULTADO_HUNDIDO] = "H";
	traducciones[EMBARCACION_RESULTADO_HUNDIDO_M] = "H";
	traducciones[EMBARCACION_RESULTADO_AGUA] = "A";
	traducciones[EMBARCACION_RESULTADO_AGUA_H] = "A";

	traducciones[CASILLA_ESTADO_LIBRE] = "L";
	traducciones[CASILLA_ESTADO_APUNTADO] = "AP";
	traducciones[CASILLA_ESTADO_TOCADO] = "T";
	traducciones[CASILLA_EVENTO_INCOMING] = "I";

	traducciones[ERROR_JUGADOR_INCORRECTO] = "Jugador incorrecto";
	traducciones[ERROR_ESTADO_INCORRECTO] = "Estado incorrecto";
	traducciones[ERROR_POSICIONES_INVALIDAS] = "Posiciones invalidas";
	traducciones[ERROR_ETA_NO_TRANSCURRIDO] = "ETA no transcurrido";
	traducciones[ERROR_JUEGO_NO_COMENZADO] = "Juego no comezado";
	traducciones[ERROR_JUEGO_EN_PROGRESO] = "Juego en progreso";
	traducciones[ERROR_JUGADOR_INEXISTENTE] = "Jugador inexistente";
	traducciones[ERROR_JUGADOR_NO_LISTO] = "Hay al menos un jugador que no está listo";
	traducciones[ERROR_MAX_JUGADORES] = "Máximo número de jugadores alcanzado";
	traducciones[ERROR_JSON_ERROR] = "Error en el json";
	traducciones[ERROR_JUGADOR_HUNDIDO] = "No puedes disparar mas una vez hundidos todos tus barcos";
	traducciones[ERROR_NO_ERROR] = "Ok";
    
	traducciones[RESULTADO_APUNTADO_DENEGADO] = "Denegado";
	traducciones[RESULTADO_APUNTADO_ACEPTADO] = "Aceptado";

}

