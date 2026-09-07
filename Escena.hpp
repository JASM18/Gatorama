/**
 * \file Escena.hpp
 * \brief Las pantallas del juego, en un solo lugar.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Este enum viv&iacute;a dentro de Menu.hpp, que ten&iacute;a sentido cuando el men&uacute; era la
 * &uacute;nica pantalla. Ahora la configuraci&oacute;n, el juego y la pausa tambi&eacute;n lo necesitan,
 * y que todas incluyeran el men&uacute; para llegar a &eacute;l era la se&ntilde;al de que estaba en el
 * lugar equivocado.
 */

#ifndef ESCENA_HPP_INCLUDED
#define ESCENA_HPP_INCLUDED

/**
 * \brief Pantallas del juego.
 *
 * El bucle principal guarda uno de estos valores y decide con &eacute;l qu&eacute; actualizar
 * y qu&eacute; dibujar en cada vuelta. No hay ventanas ni pantallas de verdad: solo
 * esta variable.
 *
 * La **pausa no est&aacute; en esta lista** a prop&oacute;sito. No es otra pantalla: es un modo
 * del juego, una ventana que se dibuja **encima** del tablero mientras el tablero
 * se sigue viendo. Por eso vive como una bandera dentro de la pantalla de juego y
 * no como un valor de aqu&iacute;.
 */
enum Escena_Estado {
    Escena_menu,
    Escena_configuracion,
    Escena_puntajes,
    Escena_opciones,
    Escena_creditos,
    Escena_juego,
    Escena_resultados,
    Escena_salir        ///< No dibuja nada; le avisa al bucle que termine
};

#endif // ESCENA_HPP_INCLUDED
