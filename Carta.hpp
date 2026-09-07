/**
 * \file Carta.hpp
 * \brief Una carta del memorama: a qu&eacute; pareja pertenece y c&oacute;mo est&aacute;.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Esta clase es **modelo**: no sabe que raylib existe, no sabe d&oacute;nde se dibuja ni
 * de qu&eacute; tama&ntilde;o. Solo sabe qu&eacute; gato le toc&oacute; y si est&aacute; boca arriba, boca abajo o
 * ya emparejada. Por eso se puede probar en consola, sin abrir ventana.
 */

#ifndef CARTA_HPP_INCLUDED
#define CARTA_HPP_INCLUDED

/**
 * \brief Una carta del tablero.
 *
 * No maneja memoria din&aacute;mica -son tres datos simples-, as&iacute; que **no necesita la
 * regla de tres**: el destructor, el constructor de copias y el operador de
 * asignaci&oacute;n que genera el compilador hacen exactamente lo correcto. Escribirlos
 * a mano aqu&iacute; ser&iacute;a ruido.
 */
class Carta
{
public:

    /**
     * \brief Construye una carta sin pareja asignada, boca abajo.
     *
     * Hace falta porque el tablero reserva su arreglo antes de repartir.
     */
    Carta();

    /**
     * \brief Construye una carta de una pareja concreta, boca abajo.
     * \param idPareja N&uacute;mero de la pareja. Dos cartas con el mismo n&uacute;mero son iguales.
     */
    Carta(int idPareja);

    /**
     * \brief A qu&eacute; pareja pertenece.
     * \return El n&uacute;mero de pareja, o -1 si todav&iacute;a no se le asigna ninguna.
     */
    int IdPareja() const;

    /**
     * \brief Si en este momento se ve su ilustraci&oacute;n.
     * \return Verdadero si est&aacute; boca arriba o ya emparejada.
     */
    bool EstaVolteada() const;

    /**
     * \brief Si ya fue encontrada y se queda descubierta para siempre.
     * \return Verdadero si ya se emparej&oacute;.
     */
    bool EstaEmparejada() const;

    /**
     * \brief La pone boca arriba.
     *
     * Sobre una carta ya emparejada no hace nada: esas nunca se vuelven a tapar.
     */
    void Voltear();

    /**
     * \brief La regresa boca abajo.
     *
     * Sobre una carta ya emparejada no hace nada, por la misma raz&oacute;n.
     */
    void Ocultar();

    /**
     * \brief La marca como encontrada.
     *
     * A partir de aqu&iacute; se queda visible y deja de responder a Voltear y Ocultar.
     */
    void Emparejar();

    /**
     * \brief Si esta carta y otra forman pareja.
     *
     * Dos cartas hacen pareja si comparten n&uacute;mero de pareja. Una carta **no** hace
     * pareja consigo misma en el sentido del juego, pero eso lo decide quien lleve
     * la partida: aqu&iacute; solo se comparan los n&uacute;meros.
     *
     * \param otra La otra carta.
     * \return Verdadero si tienen el mismo n&uacute;mero de pareja.
     */
    bool HacePareja(const Carta& otra) const;

private:

    int  idPareja;         ///< Qu&eacute; ilustraci&oacute;n le toca; -1 si no tiene
    bool estaVolteada;     ///< Si se est&aacute; mostrando su cara
    bool estaEmparejada;   ///< Si ya fue encontrada
};

#endif // CARTA_HPP_INCLUDED
