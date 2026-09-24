/**
 * \file Opciones.hpp
 * \brief Ventana emergente de opciones: el volumen y nada m&aacute;s.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 24/09/2026
 *
 * Antes era una pantalla aparte a la que se llegaba desde el men&uacute;. Ahora es una
 * ventana que se abre **encima** de donde se est&eacute; -el men&uacute; o el tablero- con el
 * bot&oacute;n del engrane, igual que la pausa: lo de abajo se sigue viendo.
 *
 * Las barras son las mismas que las de la pausa, y de hecho el mismo valor: las dos
 * le hablan al m&oacute;dulo de Audio, as&iacute; que mover una mueve la otra.
 */

#ifndef OPCIONES_HPP_INCLUDED
#define OPCIONES_HPP_INCLUDED

/**
 * \brief Deja la ventana lista para abrirse, sin nada resaltado.
 */
void PrepararOpciones();

/**
 * \brief Procesa la entrada de la ventana (una llamada por fotograma).
 * \return Verdadero cuando el jugador pidi&oacute; cerrarla: ESC, la tacha o Regresar.
 */
bool ActualizarOpciones();

/**
 * \brief Dibuja la ventana encima de lo que haya. Va al final del dibujo.
 */
void DibujarOpciones();

#endif // OPCIONES_HPP_INCLUDED
