/**
 * \file Puntaje.hpp
 * \brief El registro de una partida terminada.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Modelo puro: ni raylib ni pantalla. Es lo que se guarda en el archivo y lo que
 * se ordena para armar las tablas.
 */

#ifndef PUNTAJE_HPP_INCLUDED
#define PUNTAJE_HPP_INCLUDED

#include "ConfigPartida.hpp"

/**
 * \brief Lo que logr&oacute; un jugador en una partida.
 *
 * Es **un registro por jugador**, no por partida: en dos jugadores se guardan dos,
 * porque la tabla compara personas y no partidas.
 */
struct Puntaje {
    char       nombre[LARGO_NOMBRE + 1];  ///< Qui&eacute;n lo hizo
    ModoJuego  modo;                      ///< Solitario o multijugador
    Dificultad dificultad;                ///< Tablero en el que se jug&oacute;
    int        pares;                     ///< Parejas que junt&oacute;
    int        puntos;                    ///< Puntos por rachas: la medalla
    int        racha;                     ///< Su racha m&aacute;s larga en esa partida
    int        intentos;                  ///< Veces que destap&oacute; dos cartas
    float      tiempo;                    ///< Duraci&oacute;n de la partida, en segundos
    bool       gano;                      ///< Si result&oacute; ganador
};

/**
 * \brief Compara dos registros por puntos, de mayor a menor.
 *
 * Con los mismos puntos gana el de mejor racha, y si tambi&eacute;n empatan, el m&aacute;s
 * r&aacute;pido. Sin desempates la tabla cambiar&iacute;a de orden cada vez que se abre.
 *
 * \param a Primer registro.
 * \param b Segundo registro.
 * \return Verdadero si \p a va antes que \p b.
 */
bool mejorPorPuntos(const Puntaje& a, const Puntaje& b);

/**
 * \brief Compara dos registros por tiempo, del m&aacute;s r&aacute;pido al m&aacute;s lento.
 *
 * \param a Primer registro.
 * \param b Segundo registro.
 * \return Verdadero si \p a va antes que \p b.
 */
bool mejorPorTiempo(const Puntaje& a, const Puntaje& b);

/**
 * \brief Llena un registro vac&iacute;o, para no dejar basura en la memoria.
 * \return Un registro en ceros.
 */
Puntaje puntajeVacio();

#endif // PUNTAJE_HPP_INCLUDED
