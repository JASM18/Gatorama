/**
 * \file Puntajes.hpp
 * \brief Pantalla de mejores puntajes y registro de las partidas terminadas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Este m&oacute;dulo es el &uacute;nico que toca el archivo de puntajes. La pantalla de juego no
 * sabe que existe un JSON: solo avisa que una partida termin&oacute;.
 */

#ifndef PUNTAJES_HPP_INCLUDED
#define PUNTAJES_HPP_INCLUDED

#include "Escena.hpp"
#include "ConfigPartida.hpp"
#include "Partida.hpp"

/**
 * \brief Anota una partida terminada en el archivo.
 *
 * Guarda **un registro por jugador**: en multijugador se anotan los dos, porque la
 * tabla compara personas, no partidas.
 *
 * \param config  Modo, dificultad y nombres con los que se jug&oacute;.
 * \param partida La partida ya terminada.
 */
void GuardarResultado(const ConfigPartida& config, const Partida& partida);

/**
 * \brief Prepara la pantalla antes de entrar a ella.
 *
 * Vuelve a leer el archivo y deja seleccionada la primera pesta&ntilde;a.
 */
void PrepararPuntajes();

/**
 * \brief Procesa la entrada de la pantalla (una llamada por fotograma).
 * \return Escena_menu si el jugador quiere volver, o Escena_puntajes si sigue ah&iacute;.
 */
Escena_Estado ActualizarPuntajes();

/**
 * \brief Dibuja las dos tablas.
 */
void DibujarPuntajes();

#endif // PUNTAJES_HPP_INCLUDED
