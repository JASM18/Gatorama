/**
 * \file Partida.hpp
 * \brief Las reglas: turnos, parejas, racha y puntaje.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Sigue siendo **modelo**: ni una l&iacute;nea de raylib, ni un p&iacute;xel. Se puede jugar una
 * partida entera desde una prueba de consola, y de hecho as&iacute; se prueba.
 *
 * El reparto de responsabilidades es este: Tablero sabe **qu&eacute; cartas hay**, Partida
 * sabe **qu&eacute; significa voltearlas**, y la pantalla de juego solo traduce clics y
 * dibuja. Cambiar una regla se hace aqu&iacute; y nada m&aacute;s aqu&iacute;.
 */

#ifndef PARTIDA_HPP_INCLUDED
#define PARTIDA_HPP_INCLUDED

#include <exception>

#include "Tablero.hpp"
#include "Dificultad.hpp"

/**
 * \brief Cu&aacute;ntos jugadores caben como m&aacute;ximo.
 */
const int MAX_JUGADORES = 2;

/**
 * \brief Qu&eacute; pas&oacute; al voltear una carta.
 */
enum ResultadoVolteo {
    Volteo_ignorado,   ///< No se pudo: carta inv&aacute;lida, ya destapada, o hay un fallo sin resolver
    Volteo_primera,    ///< Se destap&oacute; la primera carta del par
    Volteo_acierto,    ///< Se destap&oacute; la segunda y son pareja
    Volteo_fallo       ///< Se destap&oacute; la segunda y no son pareja
};

/**
 * \brief Una partida en curso: el tablero m&aacute;s las reglas que lo gobiernan.
 *
 * **Las reglas, en corto:**
 * - Aciertas y sigues jugando. Fallas y pasa el turno.
 * - Cada pareja acertada alarga tu racha y te da **tantos puntos como el largo de
 *   esa racha**: la primera vale 1, la segunda seguida vale 2, la tercera vale 3.
 * - **Gana quien junte m&aacute;s parejas, no quien tenga m&aacute;s puntos.** El puntaje es una
 *   medalla para la tabla de mejores puntajes, no la condici&oacute;n de victoria.
 */
class Partida
{
public:

    /**
     * \brief Se pidi&oacute; una partida con un n&uacute;mero de jugadores imposible.
     */
    class JugadoresInvalidos : public std::exception
    {
    public:
        const char* what() const noexcept
        {
            return "El numero de jugadores debe ser 1 o 2";
        }
    };

    /**
     * \brief Arma una partida nueva, ya barajada.
     *
     * \param nivel        Dificultad, que decide el tama&ntilde;o del tablero.
     * \param numJugadores 1 o 2.
     * \throw JugadoresInvalidos Si no son 1 ni 2.
     */
    Partida(Dificultad nivel, int numJugadores);

    /**
     * \brief Vuelve a repartir y pone todos los marcadores en cero.
     *
     * Misma dificultad y mismos jugadores: es el bot&oacute;n de reiniciar.
     */
    void Reiniciar();

    /**
     * \brief El tablero, para consultarlo al dibujar.
     * \return Referencia constante al tablero.
     */
    const Tablero& ElTablero() const;

    /**
     * \brief Intenta destapar una carta.
     *
     * \param indice Posici&oacute;n corrida de la carta, de 0 a NumeroDeCartas() - 1.
     * \return Qu&eacute; ocurri&oacute;; ver ResultadoVolteo.
     */
    ResultadoVolteo Voltear(int indice);

    /**
     * \brief Si hay dos cartas destapadas que no hicieron pareja.
     *
     * Mientras esto sea verdadero no se acepta ning&uacute;n volteo m&aacute;s. Quien decide
     * **cu&aacute;ndo** taparlas es la pantalla, porque es una decisi&oacute;n de tiempo en
     * pantalla y no una regla del juego.
     *
     * \return Verdadero si falta resolver un fallo.
     */
    bool HayFalloPendiente() const;

    /**
     * \brief Tapa el par equivocado, rompe la racha y pasa el turno.
     *
     * La racha se rompe **aqu&iacute;** y no en el momento de fallar, para que el jugador
     * alcance a ver en pantalla la racha que ten&iacute;a justo cuando se le cae.
     */
    void ResolverFallo();

    /**
     * \brief Adelanta el reloj de la partida.
     *
     * Recibe los segundos en vez de medirlos para no depender de raylib: as&iacute; una
     * prueba de consola puede simular una partida de tres minutos al instante.
     *
     * \param segundos Segundos transcurridos desde la llamada anterior.
     */
    void CorrerReloj(float segundos);

    /**
     * \brief Cu&aacute;ntos jugadores hay.
     * \return 1 o 2.
     */
    int NumJugadores() const;

    /**
     * \brief De qui&eacute;n es el turno.
     * \return 0 para el primer jugador, 1 para el segundo.
     */
    int TurnoActual() const;

    /**
     * \brief Parejas que lleva un jugador.
     * \param jugador 0 o 1.
     * \return N&uacute;mero de parejas; 0 si el jugador no existe.
     */
    int ParesDe(int jugador) const;

    /**
     * \brief Puntos que lleva un jugador.
     * \param jugador 0 o 1.
     * \return Puntos acumulados; 0 si el jugador no existe.
     */
    int PuntajeDe(int jugador) const;

    /**
     * \brief Racha actual de un jugador: parejas seguidas sin fallar.
     * \param jugador 0 o 1.
     * \return Largo de la racha; 0 si el jugador no existe.
     */
    int RachaDe(int jugador) const;

    /**
     * \brief La racha m&aacute;s larga que logr&oacute; un jugador en toda la partida.
     *
     * Es el dato bonito para la tabla de mejores puntajes.
     *
     * \param jugador 0 o 1.
     * \return Largo de su mejor racha; 0 si el jugador no existe.
     */
    int RachaMaximaDe(int jugador) const;

    /**
     * \brief Cu&aacute;ntas veces se han destapado dos cartas.
     * \return N&uacute;mero de intentos de toda la partida.
     */
    int Intentos() const;

    /**
     * \brief Tiempo jugado.
     * \return Segundos transcurridos.
     */
    float Tiempo() const;

    /**
     * \brief Si ya se encontraron todas las parejas.
     * \return Verdadero cuando el tablero est&aacute; resuelto.
     */
    bool Terminada() const;

    /**
     * \brief Qui&eacute;n gan&oacute;.
     *
     * Gana quien junte **m&aacute;s parejas**; el puntaje no cuenta para esto. Los tres
     * tableros tienen un n&uacute;mero impar de parejas -5, 9 y 15-, as&iacute; que en dos
     * jugadores el empate es imposible y no hace falta desempatar.
     *
     * \return 0 o 1 seg&uacute;n el ganador, o -1 si la partida no ha terminado (o si
     *         alguien cambia los tableros a un n&uacute;mero par y s&iacute; se empata).
     */
    int Ganador() const;

private:

    Tablero tablero;                      ///< Las cartas; Partida no las guarda aparte
    int     numJugadores;                 ///< 1 o 2
    int     turno;                        ///< Jugador al que le toca

    int     pares[MAX_JUGADORES];         ///< Parejas juntadas por cada quien
    int     puntos[MAX_JUGADORES];        ///< Puntos acumulados por cada quien
    int     racha[MAX_JUGADORES];         ///< Parejas seguidas sin fallar
    int     rachaMaxima[MAX_JUGADORES];   ///< La mejor racha de la partida

    int     primera;                      ///< Carta destapada esperando pareja; -1 si ninguna
    int     segunda;                      ///< Segunda carta destapada; -1 si ninguna
    int     intentos;                     ///< Veces que se destaparon dos cartas
    float   tiempo;                       ///< Segundos jugados

    /**
     * \brief Deja los marcadores en cero, sin tocar el tablero.
     */
    void LimpiarMarcadores();

    /**
     * \brief Si un n&uacute;mero de jugador existe en esta partida.
     * \param jugador N&uacute;mero a revisar.
     * \return Verdadero si es v&aacute;lido.
     */
    bool JugadorValido(int jugador) const;
};

#endif // PARTIDA_HPP_INCLUDED
