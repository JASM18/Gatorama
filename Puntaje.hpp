/**
 * \file Puntaje.hpp
 * \brief El registro de una partida terminada y las formas de ordenarlo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Modelo puro: ni raylib ni pantalla. Es lo que se guarda en el archivo y lo que
 * se ordena para armar la tabla.
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
    float      tiempo;                    ///< Sus segundos, no los de la partida
    bool       gano;                      ///< Si result&oacute; ganador
};

/**
 * \brief Tipo de la funci&oacute;n que decide cu&aacute;l de dos registros va primero.
 *
 * Es un **puntero a funci&oacute;n**: una variable que en vez de guardar un n&uacute;mero guarda
 * *cu&aacute;l funci&oacute;n* usar. Gracias a eso el mont&iacute;culo y la tabla no necesitan una
 * versi&oacute;n distinta por cada forma de ordenar.
 */
typedef bool (*ComparadorPuntaje)(const Puntaje&, const Puntaje&);

/**
 * \brief Las columnas por las que se puede ordenar la tabla.
 */
enum ColumnaPuntaje {
    Columna_puntos,
    Columna_racha,
    Columna_pares,
    Columna_tiempo
};

/**
 * \brief Cu&aacute;ntas columnas ordenables hay.
 */
const int NUM_COLUMNAS_ORDEN = 4;

/**
 * \brief El t&iacute;tulo que se dibuja en el encabezado de una columna.
 * \param columna Columna a nombrar.
 * \return Su t&iacute;tulo.
 */
const char* tituloDeColumna(ColumnaPuntaje columna);

/**
 * \brief En qu&eacute; direcci&oacute;n conviene ordenar una columna la primera vez.
 *
 * Al hacer clic en una columna nueva, el orden que uno espera no siempre es el
 * mismo: en Puntos lo interesante es el m&aacute;s alto, pero en Tiempo es el m&aacute;s bajo.
 * Esta funci&oacute;n guarda esa expectativa para que la pantalla no tenga que decidirla.
 *
 * \param columna Columna elegida.
 * \return Verdadero si conviene de mayor a menor.
 */
bool ordenDescendentePorOmision(ColumnaPuntaje columna);

/**
 * \brief La funci&oacute;n que ordena por una columna en una direcci&oacute;n.
 *
 * Las ocho combinaciones -cuatro columnas por dos direcciones- viven en una tabla
 * en lugar de una cadena de if. Agregar una columna es agregar un rengl&oacute;n.
 *
 * \param columna     Columna por la que ordenar.
 * \param descendente Verdadero para de mayor a menor.
 * \return El comparador correspondiente.
 */
ComparadorPuntaje comparadorDe(ColumnaPuntaje columna, bool descendente);

/**
 * \brief Llena un registro vac&iacute;o, para no dejar basura en la memoria.
 * \return Un registro en ceros.
 */
Puntaje puntajeVacio();

#endif // PUNTAJE_HPP_INCLUDED
