/**
 * \file Juego.hpp
 * \brief La pantalla de la partida: el tablero real y el juego de voltear cartas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Aqu&iacute; se juntan las dos mitades: el **modelo** (Tablero, Carta) que sabe qu&eacute;
 * cartas hay, y la **vista** (VistaTablero) que sabe d&oacute;nde caen en pantalla. Esta
 * pantalla es la traductora entre las dos y no hace ninguna de sus dos tareas.
 *
 * Lo que **todav&iacute;a no** hace, a prop&oacute;sito: turnos, puntaje, racha y reloj de cuenta
 * regresiva. Eso son reglas, y las reglas las cierra el equipo.
 */

#ifndef JUEGO_HPP_INCLUDED
#define JUEGO_HPP_INCLUDED

#include "Escena.hpp"
#include "ConfigPartida.hpp"

/**
 * \brief Arma una partida nueva con la configuraci&oacute;n dada.
 *
 * Reparte el tablero del tama&ntilde;o que le toca a la dificultad elegida y deja todo
 * en cero: sin cartas volteadas, sin intentos y con el reloj empezando.
 *
 * \param config Modo, dificultad y nombres elegidos en la pantalla anterior.
 */
void IniciarPartida(const ConfigPartida& config);

/**
 * \brief Vuelve a repartir la partida actual, con la misma configuraci&oacute;n.
 *
 * Es lo que hace el bot&oacute;n Reiniciar de la ventana de pausa.
 */
void ReiniciarPartida();

/**
 * \brief Libera el tablero de la partida.
 *
 * Se llama al cerrar el juego. Sin esto el tablero se queda reservado hasta que el
 * sistema operativo lo recoge, que funciona pero no es la costumbre correcta.
 */
void LiberarPartida();

/**
 * \brief Procesa la entrada de la partida (una llamada por fotograma).
 *
 * Si la pausa est&aacute; abierta, la partida entera se congela: el reloj no avanza y los
 * clics sobre las cartas no cuentan.
 *
 * \return La escena a la que hay que cambiar, o Escena_juego si se sigue jugando.
 */
Escena_Estado ActualizarJuego();

/**
 * \brief Dibuja el tablero, el marcador y, si toca, la ventana de pausa encima.
 */
void DibujarJuego();

#endif // JUEGO_HPP_INCLUDED
