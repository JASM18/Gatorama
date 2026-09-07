/**
 * \file Puntaje.cpp
 * \brief Implementaci&oacute;n de las comparaciones entre registros.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Puntaje.hpp"

bool mejorPorPuntos(const Puntaje& a, const Puntaje& b)
{
    // Los desempates no son adorno: sin ellos, dos registros con los mismos puntos
    // quedan en un orden que depende de como los acomodo el monticulo, y la tabla
    // se ve distinta cada vez que se abre el juego sin que nada haya cambiado.
    if(a.puntos != b.puntos) return a.puntos > b.puntos;
    if(a.racha  != b.racha)  return a.racha  > b.racha;

    return a.tiempo < b.tiempo;
}

bool mejorPorTiempo(const Puntaje& a, const Puntaje& b)
{
    // Aqui "mejor" es MENOR: la tabla de tiempos premia al mas rapido.
    if(a.tiempo != b.tiempo) return a.tiempo < b.tiempo;

    return a.puntos > b.puntos;
}

Puntaje puntajeVacio()
{
    Puntaje p;

    p.nombre[0]  = '\0';
    p.modo       = Modo_solitario;
    p.dificultad = Dificultad_facil;
    p.pares      = 0;
    p.puntos     = 0;
    p.racha      = 0;
    p.intentos   = 0;
    p.tiempo     = 0.0f;
    p.gano       = false;

    return p;
}
