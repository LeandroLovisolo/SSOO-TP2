#ifndef __CONSTANTES_H__
#define __CONSTANTES_H__

typedef int error;

#define CASILLA_ESTADO_LIBRE			1		/* Casilla que no fue tocada -> 
												 * Unknown para los otros */

#define CASILLA_ESTADO_APUNTADO			2		/* Casilla que fue apuntada ->
												 * Unknown para los otros */

#define CASILLA_ESTADO_TOCADO			3       /* Casilla que fue disparada -> 
												 * Tocado / Hundido para los otros */

#define CASILLA_EVENTO_INCOMING			4		/* Casilla que fue apuntada 
												 * -> Incoming para el jugador */

#define CASILLA_EVENTO_TIRO				5

#define EMBARCACION_RESULTADO_TOCADO	6
#define EMBARCACION_RESULTADO_HUNDIDO	7
#define EMBARCACION_RESULTADO_AGUA		8
#define EMBARCACION_RESULTADO_HUNDIDO_M 9		/* mismo jugador */
#define EMBARCACION_RESULTADO_AGUA_H	10		/* hundio el agua */

#define JUGADOR_SINDEFINIR				-1

#define RESULTADO_APUNTADO_DENEGADO		11
#define RESULTADO_APUNTADO_ACEPTADO		12

#define TIRO_LIBRE						13
#define TIRO_APUNTADO					14

#define EVENTO_START					15
#define EVENTO_END						16

#define MIN_ETA							700
#define MAX_ETA							1700	/* + MIN_ETA */


#define PUNTAJE_TOCADO					1
#define PUNTAJE_HUNDIDO					2
#define PUNTAJE_MISMO_JUGADOR			2		/* duplica al hundido*/
#define PUNTAJE_MAGALLANES				100		/* hunde el agua */

/*	Los Errores son valores mayores o iguales que 100 */

#define ERROR_JUGADOR_INCORRECTO		100
#define ERROR_ESTADO_INCORRECTO			101
#define ERROR_POSICIONES_INVALIDAS		102
#define ERROR_ETA_NO_TRANSCURRIDO		103
#define ERROR_JUEGO_NO_COMENZADO		104
#define ERROR_JUEGO_EN_PROGRESO			105
#define ERROR_JUGADOR_INEXISTENTE		106
#define ERROR_JUGADOR_NO_LISTO			107
#define ERROR_MAX_JUGADORES				108
#define ERROR_JSON_ERROR				109
#define ERROR_JUGADOR_HUNDIDO			110
#define ERROR_NO_ERROR					0


/* Comandos JSON */
#define	JSON_CMD_SUSCRIBIR				"Subscribe"
#define	JSON_CMD_CONFIGURAR				"Setup"
#define JSON_CMD_COMENZAR				"Start"
#define JSON_CMD_INFORMACION			"Get_Info"
#define JSON_CMD_TIRO					"Shoot"
#define JSON_CMD_ACTUALIZACION			"Get_Update"
#define JSON_CMD_DESUSCRIBIR			"Unsubscribe"
#define JSON_CMD_NOP					"Nop"
#define JSON_CMD_SCORES					"Get_Scores"
#define JSON_CMD_ACK                    "Ack"

#define MAX_BOAT_SIZE					10

#define CONTROLLER_PORT					12346

#endif /* __CONSTANTES_H__ */
