/**
 * \file TablaPuntajes.hpp
 * \brief La lista de partidas jugadas y su archivo JSON.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Modelo puro: se puede cargar, agregar y guardar sin abrir ventana, y as&iacute; se
 * prueba.
 *
 * **Sobre el JSON.** Se escribe y se lee a mano, sin biblioteca externa. No es un
 * lector de JSON de verdad: es un lector del formato que este mismo archivo
 * escribe. Entiende objetos planos con cadenas, n&uacute;meros y booleanos, y respeta las
 * comillas y los escapes; no entiende objetos anidados ni arreglos dentro de un
 * registro, porque este formato no los tiene. Si alg&uacute;n d&iacute;a el formato crece, o se
 * crece tambi&eacute;n el lector o se mete una biblioteca.
 */

#ifndef TABLAPUNTAJES_HPP_INCLUDED
#define TABLAPUNTAJES_HPP_INCLUDED

#include "Puntaje.hpp"
#include "Monticulo.hpp"

/**
 * \brief Cu&aacute;ntas partidas se guardan como m&aacute;ximo.
 *
 * Al llenarse se tira la m&aacute;s vieja. Con las partidas de un rally no se llega ni
 * cerca, pero el archivo no puede crecer para siempre.
 */
const int MAX_PUNTAJES = 500;

/**
 * \brief Nombre del archivo donde viven los puntajes.
 *
 * Es relativo a la carpeta desde donde se ejecuta el juego, igual que las
 * im&aacute;genes.
 */
extern const char* const RUTA_PUNTAJES;

/**
 * \brief La colecci&oacute;n de partidas terminadas.
 *
 * Guarda los registros en un arreglo de tama&ntilde;o fijo, as&iacute; que **no maneja memoria
 * din&aacute;mica y no necesita la regla de tres**. El mont&iacute;culo, que s&iacute; la maneja, se
 * arma y se destruye dentro de Mejores().
 */
class TablaPuntajes
{
public:

    /**
     * \brief Arma una tabla vac&iacute;a.
     */
    TablaPuntajes();

    /**
     * \brief Lee el archivo de puntajes.
     *
     * Si el archivo no existe -la primera vez que se abre el juego- la tabla se
     * queda vac&iacute;a y no pasa nada. Un registro mal escrito se salta; el resto se
     * carga igual.
     *
     * \param ruta Archivo a leer.
     * \return Cu&aacute;ntos registros se cargaron.
     */
    int Cargar(const char* ruta);

    /**
     * \brief Escribe el archivo de puntajes.
     * \param ruta Archivo a escribir.
     * \return Verdadero si se pudo escribir.
     */
    bool Guardar(const char* ruta) const;

    /**
     * \brief Agrega el registro de una partida.
     *
     * Si la tabla est&aacute; llena se tira la m&aacute;s vieja para hacerle lugar.
     *
     * \param p Registro a agregar.
     */
    void Agregar(const Puntaje& p);

    /**
     * \brief Cu&aacute;ntos registros hay.
     * \return N&uacute;mero de partidas guardadas.
     */
    int Cantidad() const;

    /**
     * \brief Un registro por su posici&oacute;n.
     * \param indice De 0 a Cantidad() - 1.
     * \return Referencia constante al registro; el primero si el &iacute;ndice no existe.
     */
    const Puntaje& En(int indice) const;

    /**
     * \brief Saca los mejores registros seg&uacute;n el criterio que se le d&eacute;.
     *
     * Los mete todos a un mont&iacute;culo y saca los primeros. Ah&iacute; est&aacute; la ventaja: no
     * hace falta ordenar la lista completa para conocer los cinco de arriba.
     *
     * \param destino        Arreglo donde dejarlos; debe tener lugar para \p cuantos.
     * \param cuantos        Cu&aacute;ntos se quieren.
     * \param esMejor        Funci&oacute;n que decide qui&eacute;n va primero.
     * \param soloSolitario  Si es verdadero solo cuenta partidas de un jugador. Se
     *                       usa para la tabla de tiempos: en dos jugadores el tiempo
     *                       es de la partida y no de una persona, as&iacute; que meterlo
     *                       ah&iacute; repetir&iacute;a el mismo tiempo con dos nombres.
     * \return Cu&aacute;ntos se pudieron llenar; puede ser menos que \p cuantos.
     */
    int Mejores(Puntaje* destino, int cuantos,
                Monticulo::Comparador esMejor, bool soloSolitario) const;

private:

    Puntaje lista[MAX_PUNTAJES];   ///< Los registros
    int     cantidad;              ///< Cu&aacute;ntos lugares est&aacute;n ocupados
};

#endif // TABLAPUNTAJES_HPP_INCLUDED
