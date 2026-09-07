/**
 * \file Tablero.hpp
 * \brief La matriz de cartas: reparto, barajado y consultas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Tambi&eacute;n es **modelo**: ni una l&iacute;nea de raylib. El tablero sabe cu&aacute;ntas cartas
 * hay, qui&eacute;n es pareja de qui&eacute;n y cu&aacute;les ya se encontraron. **No** sabe de turnos,
 * puntaje ni reloj: eso vivir&aacute; en una clase Partida cuando el equipo cierre las
 * reglas.
 */

#ifndef TABLERO_HPP_INCLUDED
#define TABLERO_HPP_INCLUDED

#include <exception>

#include "Carta.hpp"

/**
 * \brief Matriz de cartas boca abajo, ya barajada.
 *
 * Las cartas se guardan en **un solo bloque de memoria por renglones**: la carta
 * de la fila f y la columna c est&aacute; en la posici&oacute;n `f * columnas + c`. Se eligi&oacute; as&iacute;
 * y no con un arreglo de punteros a arreglos (`Carta**`) porque es una sola
 * reservaci&oacute;n en vez de una por rengl&oacute;n: el destructor es un `delete[]`, la copia
 * es un ciclo, y no hay forma de dejar medio tablero reservado si algo falla.
 *
 * Como s&iacute; maneja memoria din&aacute;mica, esta clase lleva la **regla de tres completa**:
 * destructor, constructor de copias y operador de asignaci&oacute;n. Sin ellos, copiar un
 * tablero copiar&iacute;a el puntero y dos tableros terminar&iacute;an liberando la misma memoria.
 */
class Tablero
{
public:

    /**
     * \brief Se pidi&oacute; un tablero que no se puede armar.
     *
     * Filas o columnas no positivas, o un total de cartas impar: con un n&uacute;mero
     * impar de casillas siempre sobrar&iacute;a una carta sin pareja.
     */
    class TableroInvalido : public std::exception
    {
    public:
        const char* what() const noexcept
        {
            return "Tablero invalido: filas y columnas deben ser positivas y el total de cartas par";
        }
    };

    /**
     * \brief Se pidi&oacute; una carta que no existe en este tablero.
     */
    class FueraDeRango : public std::exception
    {
    public:
        const char* what() const noexcept
        {
            return "Posicion fuera del tablero";
        }
    };

    /**
     * \brief Arma un tablero y reparte las cartas ya barajadas.
     *
     * \param filas    Renglones de cartas, mayor que cero.
     * \param columnas Cartas por rengl&oacute;n, mayor que cero.
     * \throw TableroInvalido Si alguna medida no es positiva o el total es impar.
     */
    Tablero(int filas, int columnas);

    /**
     * \brief Constructor de copias (regla de tres).
     * \param otro Tablero a copiar.
     */
    Tablero(const Tablero& otro);

    /**
     * \brief Operador de asignaci&oacute;n (regla de tres).
     * \param otro Tablero a copiar.
     * \return Referencia a este tablero, para poder encadenar asignaciones.
     */
    Tablero& operator=(const Tablero& otro);

    /**
     * \brief Destructor (regla de tres): libera el bloque de cartas.
     */
    ~Tablero();

    /**
     * \brief Renglones del tablero.
     * \return N&uacute;mero de filas.
     */
    int Filas() const;

    /**
     * \brief Cartas por rengl&oacute;n.
     * \return N&uacute;mero de columnas.
     */
    int Columnas() const;

    /**
     * \brief Total de casillas.
     * \return filas por columnas.
     */
    int NumeroDeCartas() const;

    /**
     * \brief Cu&aacute;ntas parejas distintas hay repartidas.
     *
     * Es tambi&eacute;n cu&aacute;ntas ilustraciones distintas necesita este tablero.
     *
     * \return La mitad del total de cartas.
     */
    int NumeroDePares() const;

    /**
     * \brief Vuelve a repartir: todas boca abajo, sin emparejar y barajadas.
     *
     * Sirve para el bot&oacute;n de reiniciar sin tener que destruir y volver a crear el
     * tablero. El barajado es Fisher-Yates.
     */
    void Repartir();

    /**
     * \brief La carta de una posici&oacute;n, para consultarla o modificarla.
     * \param fila    Rengl&oacute;n, desde cero.
     * \param columna Columna, desde cero.
     * \return Referencia a la carta.
     * \throw FueraDeRango Si la posici&oacute;n no existe.
     */
    Carta& En(int fila, int columna);

    /**
     * \brief La carta de una posici&oacute;n, solo para consultarla.
     * \param fila    Rengl&oacute;n, desde cero.
     * \param columna Columna, desde cero.
     * \return Referencia constante a la carta.
     * \throw FueraDeRango Si la posici&oacute;n no existe.
     */
    const Carta& En(int fila, int columna) const;

    /**
     * \brief La carta de un &iacute;ndice corrido, para consultarla o modificarla.
     *
     * La vista ya numera las cartas de corrido al detectar el rat&oacute;n, as&iacute; que esta
     * versi&oacute;n le ahorra convertir a fila y columna para volver a lo mismo.
     *
     * \param indice Posici&oacute;n corrida, de 0 a NumeroDeCartas() - 1.
     * \return Referencia a la carta.
     * \throw FueraDeRango Si el &iacute;ndice no existe.
     */
    Carta& EnIndice(int indice);

    /**
     * \brief La carta de un &iacute;ndice corrido, solo para consultarla.
     * \param indice Posici&oacute;n corrida, de 0 a NumeroDeCartas() - 1.
     * \return Referencia constante a la carta.
     * \throw FueraDeRango Si el &iacute;ndice no existe.
     */
    const Carta& EnIndice(int indice) const;

    /**
     * \brief Cu&aacute;ntas parejas se han encontrado.
     * \return N&uacute;mero de parejas ya emparejadas.
     */
    int ParesEncontrados() const;

    /**
     * \brief Si ya no queda ninguna carta por encontrar.
     * \return Verdadero cuando todas las cartas est&aacute;n emparejadas.
     */
    bool EstaResuelto() const;

private:

    int    filas;      ///< Renglones
    int    columnas;   ///< Cartas por rengl&oacute;n
    Carta* cartas;     ///< Bloque de filas*columnas cartas, guardado por renglones

    /**
     * \brief Traduce fila y columna al &iacute;ndice del bloque, validando de paso.
     * \param fila    Rengl&oacute;n.
     * \param columna Columna.
     * \return &Iacute;ndice dentro de `cartas`.
     * \throw FueraDeRango Si la posici&oacute;n no existe.
     */
    int Indice(int fila, int columna) const;

    /**
     * \brief Reserva el bloque y copia el contenido de otro tablero.
     *
     * La usan el constructor de copias y el operador de asignaci&oacute;n, que hacen lo
     * mismo salvo por lo que ocurre antes.
     *
     * \param otro Tablero de donde copiar.
     */
    void Copiar(const Tablero& otro);
};

#endif // TABLERO_HPP_INCLUDED
