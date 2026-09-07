/**
 * \file ConfigPartida.hpp
 * \brief Lo que el jugador elige antes de empezar: modo, dificultad y nombres.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Vive en su propio archivo, y no dentro de la pantalla de configuraci&oacute;n, porque
 * son dos cosas distintas: la pantalla **recoge** estos datos y el juego los
 * **usa**. Si el juego tuviera que incluir la pantalla para conocerlos, no se
 * podr&iacute;a arrancar una partida sin pasar por ella -y eso complica hasta probarla-.
 */

#ifndef CONFIGPARTIDA_HPP_INCLUDED
#define CONFIGPARTIDA_HPP_INCLUDED

#include "Dificultad.hpp"

/**
 * \brief Cu&aacute;nta gente juega.
 */
enum ModoJuego {
    Modo_solitario,      ///< Un jugador contra el reloj
    Modo_multijugador    ///< Dos jugadores por turnos
};

/**
 * \brief Letras que cabe escribir en un nombre.
 *
 * Corto a prop&oacute;sito: el nombre se dibuja debajo del tablero y uno largo se saldr&iacute;a
 * de su lugar. Adem&aacute;s nadie va a teclear una novela en un stand.
 */
const int LARGO_NOMBRE = 12;

/**
 * \brief Todo lo que hay que saber para armar una partida.
 */
struct ConfigPartida {
    ModoJuego  modo;                        ///< Solitario o dos jugadores
    Dificultad dificultad;                  ///< Cu&aacute;l de los tres tableros
    char       nombre1[LARGO_NOMBRE + 1];   ///< Nombre del primer jugador
    char       nombre2[LARGO_NOMBRE + 1];   ///< Nombre del segundo; se ignora en solitario
};

/**
 * \brief La configuraci&oacute;n con la que arranca el juego.
 *
 * Los nombres **vienen llenos** con "Player 1" y "Player 2" en lugar de vac&iacute;os. Es
 * a prop&oacute;sito: escribir es lo m&aacute;s lento que puede hacer un ni&ntilde;o frente al stand,
 * as&iacute; que quien quiera su nombre lo borra y lo escribe, y quien no, le pica a
 * Iniciar y ya. Un campo vac&iacute;o obligar&iacute;a a todos a resolver el problema.
 *
 * \return Configuraci&oacute;n lista para usarse: solitario, f&aacute;cil y nombres por omisi&oacute;n.
 */
ConfigPartida configPorDefecto();

/**
 * \brief El nombre que hay que mostrar para un jugador.
 *
 * Si el campo qued&oacute; vac&iacute;o -porque lo borraron todo- devuelve el nombre por omisi&oacute;n
 * en vez de una cadena vac&iacute;a, para que el marcador nunca se vea roto.
 *
 * \param config    Configuraci&oacute;n de la partida.
 * \param numJugador 1 o 2.
 * \return Cadena con el nombre a dibujar.
 */
const char* nombreDeJugador(const ConfigPartida& config, int numJugador);

#endif // CONFIGPARTIDA_HPP_INCLUDED
