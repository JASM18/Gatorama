/**
 * \file ConfigPartida.cpp
 * \brief Implementaci&oacute;n de la configuraci&oacute;n de partida.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstring>

#include "ConfigPartida.hpp"

// Los nombres por omision viven aqui, en un solo lugar, porque se usan en dos
// momentos: al llenar los campos de la pantalla de configuracion y al dibujar el
// marcador si alguien borro el suyo.
static const char* NOMBRE_UNO = "Player 1";
static const char* NOMBRE_DOS = "Player 2";

/**
 * \brief Copia un nombre respetando el limite del arreglo.
 *
 * strncpy no pone el cero final si el texto llena todo el espacio, asi que se
 * escribe a mano. Es el clasico desbordamiento silencioso.
 *
 * \param destino Arreglo de LARGO_NOMBRE + 1 caracteres.
 * \param texto   Texto a copiar.
 */
static void copiarNombre(char* destino, const char* texto)
{
    strncpy(destino, texto, LARGO_NOMBRE);
    destino[LARGO_NOMBRE] = '\0';
}

ConfigPartida configPorDefecto()
{
    ConfigPartida config;

    config.modo       = Modo_solitario;
    config.dificultad = Dificultad_facil;

    copiarNombre(config.nombre1, NOMBRE_UNO);
    copiarNombre(config.nombre2, NOMBRE_DOS);

    return config;
}

const char* nombreDeJugador(const ConfigPartida& config, int numJugador)
{
    const char* nombre      = (numJugador == 2) ? config.nombre2 : config.nombre1;
    const char* porOmision  = (numJugador == 2) ? NOMBRE_DOS     : NOMBRE_UNO;

    // Un nombre vacio no es un error del jugador: borro todo y ya. Pero un
    // marcador que dice "Pares: 3" sin nombre arriba se ve descompuesto, asi que
    // se rellena aqui en vez de obligar a cada pantalla a preguntar.
    if(nombre[0] == '\0') return porOmision;

    return nombre;
}
