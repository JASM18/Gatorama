/**
 * \file Instrucciones.hpp
 * \brief La ventana de "c&oacute;mo se juega", encima del tablero.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Otra ventana virtual, igual que la pausa: se dibuja encima y el juego sigue
 * congelado detr&aacute;s. Se abre con el bot&oacute;n de ayuda del tablero o desde la pausa.
 *
 * El texto describe **solo lo que el juego ya hace**. Cuando el equipo cierre las
 * reglas de turnos, puntaje y racha, hay que volver aqu&iacute; a agregarlas: unas
 * instrucciones que prometan algo que no ocurre son peores que no tenerlas.
 */

#ifndef INSTRUCCIONES_HPP_INCLUDED
#define INSTRUCCIONES_HPP_INCLUDED

/**
 * \brief Procesa la entrada de la ventana de instrucciones.
 *
 * \return Verdadero cuando el jugador la cierra, con la tacha o con ESC.
 */
bool ActualizarInstrucciones();

/**
 * \brief Dibuja el velo y el panel de instrucciones.
 *
 * Muestra los dos modos a la vez, como el boceto. Antes el texto cambiaba seg&uacute;n el
 * modo de la partida, pero desde el men&uacute; todav&iacute;a no hay partida de d&oacute;nde sacarlo
 * &mdash; y ver los dos es justo lo que alguien quiere antes de elegir.
 */
void DibujarInstrucciones();

#endif // INSTRUCCIONES_HPP_INCLUDED
