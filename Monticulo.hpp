/**
 * \file Monticulo.hpp
 * \brief Mont&iacute;culo de registros, para sacar los mejores sin ordenar todo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Modelo puro, sin raylib.
 *
 * **Por qu&eacute; un mont&iacute;culo y no ordenar el arreglo.** Para la tabla se necesitan los
 * cinco mejores de un mont&oacute;n de partidas, no la lista completa en orden. Ordenar
 * todo cuesta n log n; meter todo al mont&iacute;culo cuesta n y sacar cinco cuesta
 * 5 log n. Con las partidas de un rally la diferencia no se siente, pero la
 * estructura es la correcta para la pregunta que se est&aacute; haciendo, y es la que el
 * equipo ya implement&oacute; en Estructura de Datos.
 */

#ifndef MONTICULO_HPP_INCLUDED
#define MONTICULO_HPP_INCLUDED

#include <exception>

#include "Puntaje.hpp"

/**
 * \brief Mont&iacute;culo binario de registros de puntaje.
 *
 * En la ra&iacute;z siempre est&aacute; el "mejor" seg&uacute;n la funci&oacute;n de comparaci&oacute;n que se le d&eacute;
 * al construirlo. El mismo mont&iacute;culo sirve para la tabla de puntos y para la de
 * tiempos: lo &uacute;nico que cambia es esa funci&oacute;n.
 *
 * Maneja memoria din&aacute;mica, as&iacute; que lleva la **regla de tres completa**.
 */
class Monticulo
{
public:

    /**
     * \brief Tipo de la funci&oacute;n que decide qui&eacute;n va primero.
     *
     * Es el mismo tipo que define Puntaje.hpp. Gracias a &eacute;l no hacen falta ocho
     * mont&iacute;culos casi id&eacute;nticos, uno por cada columna y direcci&oacute;n de la tabla.
     */
    typedef ComparadorPuntaje Comparador;

    /**
     * \brief Se quiso insertar en un mont&iacute;culo que ya no tiene lugar.
     */
    class Lleno : public std::exception
    {
    public:
        const char* what() const noexcept { return "El monticulo esta lleno"; }
    };

    /**
     * \brief Se quiso sacar de un mont&iacute;culo vac&iacute;o.
     */
    class Vacio : public std::exception
    {
    public:
        const char* what() const noexcept { return "El monticulo esta vacio"; }
    };

    /**
     * \brief Arma un mont&iacute;culo vac&iacute;o.
     *
     * \param capacidad Cu&aacute;ntos registros caben como m&aacute;ximo; debe ser mayor que cero.
     * \param esMejor   Funci&oacute;n que decide cu&aacute;l de dos registros va primero.
     * \throw Lleno Si la capacidad no es positiva.
     */
    Monticulo(int capacidad, Comparador esMejor);

    /**
     * \brief Constructor de copias (regla de tres).
     * \param otro Mont&iacute;culo a copiar.
     */
    Monticulo(const Monticulo& otro);

    /**
     * \brief Operador de asignaci&oacute;n (regla de tres).
     * \param otro Mont&iacute;culo a copiar.
     * \return Referencia a este mont&iacute;culo.
     */
    Monticulo& operator=(const Monticulo& otro);

    /**
     * \brief Destructor (regla de tres).
     */
    ~Monticulo();

    /**
     * \brief Mete un registro y lo acomoda en su lugar.
     * \param p Registro a insertar.
     * \throw Lleno Si ya no cabe.
     */
    void Insertar(const Puntaje& p);

    /**
     * \brief Saca el mejor registro y reacomoda el resto.
     * \return El registro que estaba en la ra&iacute;z.
     * \throw Vacio Si no hay nada que sacar.
     */
    Puntaje Sacar();

    /**
     * \brief Si no queda nada dentro.
     * \return Verdadero si est&aacute; vac&iacute;o.
     */
    bool EstaVacio() const;

    /**
     * \brief Cu&aacute;ntos registros tiene.
     * \return N&uacute;mero de elementos guardados.
     */
    int Cantidad() const;

private:

    Puntaje*   datos;       ///< Arreglo donde vive el &aacute;rbol
    int        capacidad;   ///< Tama&ntilde;o del arreglo
    int        cantidad;    ///< Cu&aacute;ntos lugares est&aacute;n ocupados
    Comparador esMejor;     ///< Qui&eacute;n va primero

    /**
     * \brief Sube un elemento hasta que su padre sea mejor que &eacute;l.
     * \param posicion De d&oacute;nde empezar a subir.
     */
    void Subir(int posicion);

    /**
     * \brief Baja un elemento hasta que sea mejor que sus dos hijos.
     * \param posicion De d&oacute;nde empezar a bajar.
     */
    void Bajar(int posicion);

    /**
     * \brief Reserva y copia el contenido de otro mont&iacute;culo.
     * \param otro Mont&iacute;culo de donde copiar.
     */
    void Copiar(const Monticulo& otro);
};

#endif // MONTICULO_HPP_INCLUDED
