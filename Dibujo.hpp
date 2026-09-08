/**
 * \file Dibujo.hpp
 * \brief Utilidades de dibujo que comparten todas las pantallas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#ifndef DIBUJO_HPP_INCLUDED
#define DIBUJO_HPP_INCLUDED

#include "raylib.h"

/**
 * \brief Dibuja un texto centrado horizontalmente en la ventana.
 *
 * raylib solo sabe dibujar texto a partir de una esquina, as&iacute; que centrar es
 * medir el texto y restarle la mitad al centro de la ventana. Se usa en todas
 * las pantallas, por eso vive aqu&iacute; y no dentro de una en particular.
 *
 * \param texto  Cadena a dibujar.
 * \param y      Coordenada vertical de la esquina superior del texto.
 * \param tamano Tama&ntilde;o de la fuente en p&iacute;xeles.
 * \param color  Color del texto.
 */
void dibujarTextoCentrado(const char* texto, int y, int tamano, Color color);

/**
 * \brief Dibuja una pantalla provisional con su nombre y la ayuda para volver.
 *
 * Sirve de relleno para las escenas que todav&iacute;a no existen, para poder navegar
 * el men&uacute; completo desde el primer d&iacute;a. Se ir&aacute; borrando conforme cada pantalla
 * se implemente de verdad.
 *
 * \param nombre Nombre de la pantalla que ir&aacute; en su lugar.
 */
void dibujarPantallaPendiente(const char* nombre);

/**
 * \brief Carga el fondo que se ve en todas las pantallas.
 *
 * **Despu&eacute;s de InitWindow()**, como toda textura.
 */
void cargarFondoGeneral();

/**
 * \brief Libera el fondo general. **Antes de CloseWindow()**.
 */
void descargarFondoGeneral();

/**
 * \brief Pinta el fondo general, cubriendo la ventana entera.
 *
 * Va **al principio de cada fotograma**, justo despu&eacute;s de limpiar la pantalla y
 * antes que cualquier otra cosa. No hay que llamarlo desde cada pantalla: lo hace
 * el bucle principal una sola vez, y por eso todas lo tienen sin pedirlo.
 *
 * Si el archivo no est&aacute; no dibuja nada y queda el color liso de Tema.hpp.
 */
void dibujarFondoGeneral();

/**
 * \brief Carga una imagen si el archivo existe, y avisa si se pudo.
 *
 * Vive aqu&iacute; porque ya la usan el men&uacute;, el tablero y la configuraci&oacute;n: era la misma
 * media docena de l&iacute;neas copiada en tres archivos.
 *
 * Que devuelva un bool en vez de tronar es lo que permite que el juego corra con el
 * arte a medio hacer: quien la llama guarda la respuesta y decide si dibuja la
 * imagen o lo que hab&iacute;a antes.
 *
 * \param ruta    Archivo a cargar.
 * \param destino D&oacute;nde dejar la textura.
 * \return Verdadero si qued&oacute; cargada.
 */
bool cargarTexturaSiEsta(const char* ruta, Texture2D* destino);

//***********************************************
// TEXTO GENERADO
//***********************************************
// Las funciones de abajo dibujan con la fuente del juego, **Coconut Island**, y
// est&aacute;n hechas para el texto que sale de los datos: el nombre que tecle&oacute; el
// jugador, el reloj, los puntos, la tabla de puntajes.
//
// Los r&oacute;tulos fijos -"CREDITOS", "PAUSA", las etiquetas de los botones- siguen
// con DrawText y la fuente de f&aacute;brica **a prop&oacute;sito**: esos se van a reemplazar por
// im&aacute;genes, y no tiene caso vestirlos ahora.

/**
 * \brief Carga la fuente del juego.
 *
 * **Despu&eacute;s de InitWindow()**, como toda textura: una fuente cargada es al final
 * una imagen en la memoria de video. Si el archivo no est&aacute;, el juego sigue
 * corriendo con la fuente de f&aacute;brica.
 */
void cargarFuenteDatos();

/**
 * \brief Dice si la fuente de datos puede dibujar ese car&aacute;cter.
 *
 * Sirve para filtrar lo que se teclea: de nada vale aceptar una letra que luego
 * saldr&iacute;a como un cuadrito. Se pregunta aqu&iacute; en vez de comparar contra un rango en
 * cada pantalla para que la lista de caracteres v&aacute;lidos viva en un solo lugar: el
 * mismo que se le pasa a LoadFontEx.
 *
 * Si la fuente no carg&oacute;, solo acepta ASCII, porque el respaldo es la letra de
 * f&aacute;brica y esa no sabe de acentos.
 *
 * \param codigo Punto de c&oacute;digo Unicode, tal como lo devuelve GetCharPressed().
 * \return Verdadero si esa letra se puede dibujar.
 */
bool fuenteTieneCodigo(int codigo);

/**
 * \brief Libera la fuente del juego. **Antes de CloseWindow()**.
 */
void descargarFuenteDatos();

/**
 * \brief Dibuja texto generado a partir de una esquina.
 *
 * \param texto  Cadena a dibujar.
 * \param x      Coordenada horizontal de la esquina izquierda.
 * \param y      Coordenada vertical de la esquina superior.
 * \param tamano Alto de la letra en p&iacute;xeles.
 * \param color  Color del texto.
 */
void dibujarDato(const char* texto, int x, int y, int tamano, Color color);

/**
 * \brief Dibuja texto generado, centrado horizontalmente en la ventana.
 *
 * \param texto  Cadena a dibujar.
 * \param y      Coordenada vertical de la esquina superior.
 * \param tamano Alto de la letra en p&iacute;xeles.
 * \param color  Color del texto.
 */
void dibujarDatoCentrado(const char* texto, int y, int tamano, Color color);

/**
 * \brief Cu&aacute;ntos p&iacute;xeles de ancho ocupa un texto generado.
 *
 * Hace falta para centrar, alinear a la derecha o poner el cursor de escritura
 * despu&eacute;s de la &uacute;ltima letra. **No** sirve MeasureText: esa mide con la fuente de
 * f&aacute;brica y dar&iacute;a un ancho distinto al que se va a dibujar.
 *
 * \param texto  Cadena a medir.
 * \param tamano Alto de la letra en p&iacute;xeles.
 * \return Ancho en p&iacute;xeles.
 */
int anchoDato(const char* texto, int tamano);

#endif // DIBUJO_HPP_INCLUDED
