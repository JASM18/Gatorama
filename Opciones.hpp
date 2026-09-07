/**
 * \file Opciones.hpp
 * \brief Pantalla de opciones del juego.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Por ahora solo el volumen. Es la misma barra que est&aacute; en la pausa, y de hecho el
 * mismo valor: las dos le hablan al m&oacute;dulo de Audio, as&iacute; que mover una mueve la
 * otra. No hay dos volumenes que se puedan desincronizar.
 */

#ifndef OPCIONES_HPP_INCLUDED
#define OPCIONES_HPP_INCLUDED

#include "Escena.hpp"

/**
 * \brief Procesa la entrada de la pantalla de opciones.
 * \return Escena_menu si el jugador quiere volver, o Escena_opciones si sigue ah&iacute;.
 */
Escena_Estado ActualizarOpciones();

/**
 * \brief Dibuja la pantalla de opciones.
 */
void DibujarOpciones();

#endif // OPCIONES_HPP_INCLUDED
