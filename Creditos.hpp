/**
 * \file Creditos.hpp
 * \brief Pantalla de cr&eacute;ditos.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 */

#ifndef CREDITOS_HPP_INCLUDED
#define CREDITOS_HPP_INCLUDED

#include "Escena.hpp"

/**
 * \brief Procesa la entrada de la pantalla de cr&eacute;ditos.
 * \return Escena_menu si el jugador quiere volver, o Escena_creditos si sigue ah&iacute;.
 */
Escena_Estado ActualizarCreditos();

/**
 * \brief Dibuja los cr&eacute;ditos.
 */
void DibujarCreditos();

#endif // CREDITOS_HPP_INCLUDED
