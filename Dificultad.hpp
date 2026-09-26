/**
 * \file Dificultad.hpp
 * \brief Los tres niveles de dificultad de Gatorama y el tablero de cada uno.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 */

#ifndef DIFICULTAD_HPP_INCLUDED
#define DIFICULTAD_HPP_INCLUDED

const int NUM_DIFICULTADES = 3;

/**
 * \brief Nivel de dificultad de la partida.
 *
 * El valor del enum es tambi&eacute;n el &iacute;ndice dentro de DIFICULTADES, as&iacute; que
 * DIFICULTADES[Dificultad_facil] da los datos del nivel f&aacute;cil.
 */
enum Dificultad {
    Dificultad_facil,
    Dificultad_normal,
    Dificultad_dificil
};

/**
 * \brief Tama&ntilde;o de tablero que le corresponde a un nivel.
 */
struct InfoDificultad {
    const char* nombre;   ///< Nombre que se le muestra al jugador
    int filas;            ///< Renglones de cartas
    int columnas;         ///< Cartas por renglon
};

// Los tres niveles viven aqui como datos, no como una cadena de if. Cambiar un
// tablero es editar un renglon de esta tabla; agregar un cuarto nivel es agregar
// un renglon y subirle uno a NUM_DIFICULTADES. Ningun otro archivo se entera.
//
// Los nombres van sin acento a proposito: la fuente que trae raylib de fabrica
// solo cubre ASCII, y "Facil" con acento saldria con un simbolo raro en pantalla.
// Cuando se cargue una fuente propia con SetTextureFilter esto se puede corregir.
const InfoDificultad DIFICULTADES[NUM_DIFICULTADES] = {
    { "Facil",   2,  5 },   // 10 cartas -> 5 pares
    { "Normal",  3,  6 },   // 18 cartas -> 9 pares
    { "Dificil", 3, 10 }    // 30 cartas -> 15 pares
};

/**
 * \brief Cu&aacute;ntas cartas tiene el tablero de un nivel.
 * \param nivel Nivel de dificultad.
 * \return filas * columnas.
 */
int numeroDeCartas(Dificultad nivel);

/**
 * \brief Cu&aacute;ntas parejas distintas hacen falta para un nivel.
 *
 * Es el n&uacute;mero que le importa a la parte de arte: es cu&aacute;ntos gatos distintos
 * hay que dibujar.
 *
 * \param nivel Nivel de dificultad.
 * \return La mitad de las cartas del tablero.
 */
int numeroDePares(Dificultad nivel);

#endif // DIFICULTAD_HPP_INCLUDED
