/**
 * \file Tema.hpp
 * \brief Paleta de colores del juego, declarada en un solo lugar.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#ifndef TEMA_HPP_INCLUDED
#define TEMA_HPP_INCLUDED

#include "raylib.h"

// Los colores viven aqui y no regados por los archivos que dibujan. Cambiar el
// aspecto del juego debe ser editar este archivo, no salir a cazar literales.
//
// Mas adelante estos valores saldran de tema.json y se podran cambiar sin
// recompilar, que es lo que le permitira a la companera de arte trabajar sin
// tocar C++. Por ahora son constantes: mismo lugar, mismo proposito, un paso
// menos de complejidad.

const Color COLOR_FONDO     = {  30,  30,  46, 255 };   ///< Fondo de todas las pantallas
const Color COLOR_TITULO    = { 203, 166, 247, 255 };   ///< T&iacute;tulos grandes
const Color COLOR_TEXTO     = { 166, 173, 200, 255 };   ///< Texto normal
const Color COLOR_SELECCION = { 137, 180, 250, 255 };   ///< Opci&oacute;n resaltada del men&uacute;
const Color COLOR_TENUE     = { 108, 112, 134, 255 };   ///< Ayudas y notas al pie

// Colores de las cartas. El dorso es la cara oculta -la que se ve al empezar la
// partida-; el borde la separa del fondo y el resaltado avisa cual carta se va a
// escoger si se hace clic ahora.
const Color COLOR_CARTA_DORSO = {  69,  71,  90, 255 };   ///< Cara oculta de la carta
const Color COLOR_CARTA_BORDE = { 137, 180, 250, 255 };   ///< Contorno de la carta
const Color COLOR_CARTA_HOVER = {  88,  91, 112, 255 };   ///< Carta bajo el puntero

// Botones y paneles. El boton "activo" es el de una opcion ya elegida -por ejemplo
// la dificultad seleccionada-, distinto del que solo tiene el raton encima.
const Color COLOR_BOTON       = {  49,  50,  68, 255 };   ///< Bot&oacute;n en reposo
const Color COLOR_BOTON_HOVER = {  69,  71,  90, 255 };   ///< Bot&oacute;n bajo el puntero
const Color COLOR_BOTON_ACTIVO= { 137, 180, 250, 255 };   ///< Bot&oacute;n de la opci&oacute;n elegida
const Color COLOR_PANEL       = {  36,  37,  54, 255 };   ///< Fondo de las ventanas virtuales
const Color COLOR_VELO        = {  17,  17,  27, 200 };   ///< Oscurecido detr&aacute;s de un panel

#endif // TEMA_HPP_INCLUDED
