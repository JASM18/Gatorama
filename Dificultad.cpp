/**
 * \file Dificultad.cpp
 * \brief Implementaci&oacute;n de las consultas sobre los niveles de dificultad.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Dificultad.hpp"

int numeroDeCartas(Dificultad nivel)
{
    // Se sale del arreglo si alguien inventa un valor de enum que no existe.
    // Preferimos devolver el tablero mas chico antes que leer memoria ajena.
    if(nivel < 0 || nivel >= NUM_DIFICULTADES) nivel = Dificultad_facil;

    return DIFICULTADES[nivel].filas * DIFICULTADES[nivel].columnas;
}

int numeroDePares(Dificultad nivel)
{
    // Cada pareja son dos cartas. Los tres tableros tienen un numero par de
    // casillas justamente para que esta division siempre sea exacta.
    return numeroDeCartas(nivel) / 2;
}
