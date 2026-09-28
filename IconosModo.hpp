#pragma once

/**
 * \file IconosModo.hpp
 * \brief Icono de persona y colores de modo, compartidos entre
 *        Instrucciones.cpp y Configuracion.cpp, para que Solitario/Tu solo
 *        y Multijugador/1 vs 1 se vean como la misma idea en las dos
 *        pantallas.
 *
 * Header-only, con static en cada pieza: cada .cpp que lo incluye se lleva
 * su propia copia privada, exactamente como pasaba antes de compartirlo.
 * Por eso no hace falta agregar ningun .cpp nuevo al proyecto de
 * Code::Blocks -nada que olvidar en el arbol de Sources, nada parecido a
 * lo que paso con el archivo duplicado.
 */

#include "raylib.h"

static const Color COLOR_MODO_SOLO = { 255, 138, 101, 255 }; // coral suave
static const Color COLOR_MODO_VS   = {  77, 182, 172, 255 }; // verde azulado

static void iconoPersona(Vector2 c, float r, Color color)
{
    DrawCircleV((Vector2){ c.x, c.y - r * 0.35f }, r * 0.28f, color);
    DrawRing((Vector2){ c.x, c.y + r * 0.55f }, r * 0.40f, r * 0.62f, 180.0f, 360.0f, 16, color);
}

static void iconoDosPersonas(Vector2 c, float r, Color color)
{
    iconoPersona((Vector2){ c.x - r * 0.42f, c.y }, r * 0.72f, color);
    iconoPersona((Vector2){ c.x + r * 0.42f, c.y }, r * 0.72f, color);
}
