/**
 * \file Menu.hpp
 * \brief Escenas del juego y men&uacute; principal.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#ifndef MENU_HPP_INCLUDED
#define MENU_HPP_INCLUDED

#include "Escena.hpp"

/**
 * \brief Procesa la entrada del men&uacute; principal (una llamada por fotograma).
 *
 * Mueve la opci&oacute;n resaltada con las flechas y confirma con Enter. Cu&aacute;l opci&oacute;n
 * est&aacute; resaltada es asunto interno de Menu.cpp: quien llama no necesita saberlo.
 *
 * \return La escena a la que hay que cambiar, o Escena_menu si el usuario sigue
 *         navegando sin haber elegido nada.
 */
Escena_Estado ActualizarMenu();

/**
 * \brief Dibuja el men&uacute; principal (una llamada por fotograma).
 *
 * Solo dibuja: no lee entrada ni cambia nada. Mantener separado el actualizar del
 * dibujar es lo que evita que el juego se comporte distinto seg&uacute;n los fotogramas.
 */
void DibujarMenu();

#endif // MENU_HPP_INCLUDED
