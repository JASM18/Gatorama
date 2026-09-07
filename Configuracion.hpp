/**
 * \file Configuracion.hpp
 * \brief Pantalla donde se elige modo, dificultad y nombres antes de jugar.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Sigue el boceto: los tres pasos a la izquierda -modo, dificultad, jugadores- y
 * a la derecha un resumen de lo elegido con el bot&oacute;n de iniciar.
 *
 * Esta pantalla **no arma la partida**: solo llena un ConfigPartida y avisa que ya
 * qued&oacute;. Quien arma el tablero es la pantalla de juego. Separarlo permite iniciar
 * una partida sin pasar por aqu&iacute;, que es justo lo que hace falta para probarla.
 */

#ifndef CONFIGURACION_HPP_INCLUDED
#define CONFIGURACION_HPP_INCLUDED

#include "Escena.hpp"
#include "ConfigPartida.hpp"

/**
 * \brief Prepara la pantalla para entrar a ella.
 *
 * Quita el foco de los campos de texto, para que al volver del men&uacute; no se quede
 * escribiendo en un cuadro que el jugador ya ni recuerda haber tocado.
 */
void PrepararConfiguracion();

/**
 * \brief Procesa la entrada de la pantalla (una llamada por fotograma).
 *
 * \param config Configuraci&oacute;n que se va llenando. Se modifica conforme el jugador
 *               elige, y es la que despu&eacute;s recibe la partida.
 * \return Escena_juego si le dio a Iniciar, Escena_menu si cancel&oacute; con ESC, o
 *         Escena_configuracion si sigue eligiendo.
 */
Escena_Estado ActualizarConfiguracion(ConfigPartida& config);

/**
 * \brief Dibuja la pantalla con las opciones y el resumen.
 * \param config Configuraci&oacute;n actual, para marcar lo que ya est&aacute; elegido.
 */
void DibujarConfiguracion(const ConfigPartida& config);

#endif // CONFIGURACION_HPP_INCLUDED
