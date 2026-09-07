/**
 * \file Tema.hpp
 * \brief Paleta de colores del juego, declarada en un solo lugar.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 07/09/2026
 */

#ifndef TEMA_HPP_INCLUDED
#define TEMA_HPP_INCLUDED

#include "raylib.h"

// Los colores viven aqui y no regados por los archivos que dibujan. Cambiar el
// aspecto del juego debe ser editar este archivo, no salir a cazar literales.
//
// El 07/09/2026 el juego paso de tema oscuro a tema claro por politica del evento,
// y ese cambio se hizo SOLO aqui: ninguna pantalla necesito una linea. Esa es la
// prueba de que la paleta estaba bien puesta.
//
// La familia de colores sale del arte de las cartas: violeta y dorado sobre papel
// tibio. No es gris neutro a proposito -un gris puro se ve apagado junto a ese
// arte-, sino un blanco con una pizca de calidez.

const Color COLOR_FONDO     = { 247, 244, 238, 255 };   ///< Fondo de todas las pantallas
const Color COLOR_TITULO    = { 106,  52, 158, 255 };   ///< T&iacute;tulos grandes
const Color COLOR_TEXTO     = {  52,  48,  62, 255 };   ///< Texto normal
const Color COLOR_SELECCION = { 138,  84, 196, 255 };   ///< Opci&oacute;n resaltada
const Color COLOR_TENUE     = { 124, 118, 136, 255 };   ///< Ayudas y notas al pie

// Colores de las cartas. El dorso y el resaltado solo se usan cuando falta el arte;
// con las ilustraciones puestas, lo unico que se dibuja es el contorno.
const Color COLOR_CARTA_DORSO = { 228, 222, 234, 255 };   ///< Cara oculta sin arte
const Color COLOR_CARTA_BORDE = {  20,  18,  26, 255 };   ///< Contorno de la carta
const Color COLOR_CARTA_HOVER = { 210, 200, 220, 255 };   ///< Carta bajo el puntero

// Botones y paneles. El boton "activo" es el de una opcion ya elegida -por ejemplo
// la dificultad seleccionada-, distinto del que solo tiene el raton encima.
const Color COLOR_BOTON       = { 236, 231, 241, 255 };   ///< Bot&oacute;n en reposo
const Color COLOR_BOTON_HOVER = { 222, 213, 233, 255 };   ///< Bot&oacute;n bajo el puntero
const Color COLOR_BOTON_ACTIVO= { 138,  84, 196, 255 };   ///< Bot&oacute;n de la opci&oacute;n elegida
const Color COLOR_PANEL       = { 253, 251, 247, 255 };   ///< Fondo de las ventanas virtuales
const Color COLOR_VELO        = {  58,  46,  74, 165 };   ///< Oscurecido detr&aacute;s de un panel

#endif // TEMA_HPP_INCLUDED
