/**
 * \file Resultados.hpp
 * \brief Pantalla de fin de partida: qui&eacute;n gan&oacute; y c&oacute;mo qued&oacute; todo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Esta pantalla **se queda con una copia** de los n&uacute;meros, no con la partida. As&iacute; el
 * juego puede liberar el tablero, o armar otro, sin que los resultados que se
 * est&aacute;n mostrando se le desaparezcan de abajo.
 */

#ifndef RESULTADOS_HPP_INCLUDED
#define RESULTADOS_HPP_INCLUDED

#include "Escena.hpp"
#include "ConfigPartida.hpp"
#include "Partida.hpp"

/**
 * \brief Copia los n&uacute;meros de una partida terminada.
 *
 * Se llama justo antes de cambiar a esta escena.
 *
 * \param config  Modo, dificultad y nombres con los que se jug&oacute;.
 * \param partida La partida ya terminada.
 */
void PrepararResultados(const ConfigPartida& config, const Partida& partida);

/**
 * \brief Procesa la entrada de la pantalla.
 * \return Escena_configuracion si quiere jugar otra, Escena_menu si quiere salir,
 *         o Escena_resultados si sigue viendo.
 */
Escena_Estado ActualizarResultados();

/**
 * \brief Dibuja el resultado de la partida.
 */
void DibujarResultados();

#endif // RESULTADOS_HPP_INCLUDED
