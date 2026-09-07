/**
 * \file Boton.hpp
 * \brief Botones con rat&oacute;n, compartidos por todas las pantallas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Un bot&oacute;n aqu&iacute; no es un objeto que se guarde en ning&uacute;n lado: es un rect&aacute;ngulo
 * al que se le pregunta si lo clicaron y al que se le pide que se dibuje. Ese es
 * el estilo de raylib, y significa que agregar un bot&oacute;n a una pantalla es agregar
 * dos llamadas, sin registrar nada ni acordarse de liberarlo.
 */

#ifndef BOTON_HPP_INCLUDED
#define BOTON_HPP_INCLUDED

#include "raylib.h"

/**
 * \brief Si el puntero est&aacute; encima de un rect&aacute;ngulo.
 * \param rec Zona a revisar.
 * \return Verdadero si el rat&oacute;n cae dentro.
 */
bool ratonEncima(Rectangle rec);

/**
 * \brief Si acaban de hacer clic sobre un rect&aacute;ngulo.
 *
 * Va en la parte de **actualizar**, no en la de dibujar: cambia lo que va a pasar.
 *
 * \param rec Zona del bot&oacute;n.
 * \return Verdadero solo en el fotograma en que se presion&oacute; el bot&oacute;n izquierdo
 *         estando encima.
 */
bool botonClicado(Rectangle rec);

/**
 * \brief Dibuja un bot&oacute;n con su etiqueta centrada.
 *
 * \param rec          D&oacute;nde va.
 * \param etiqueta     Texto del bot&oacute;n.
 * \param seleccionado Verdadero si representa una opci&oacute;n ya elegida (por ejemplo la
 *                     dificultad actual). Es distinto de tener el rat&oacute;n encima: eso
 *                     se detecta solo.
 */
void dibujarBoton(Rectangle rec, const char* etiqueta, bool seleccionado);

/**
 * \brief Arma un rect&aacute;ngulo, para no repetir la construcci&oacute;n a cada rato.
 * \param x      Esquina izquierda.
 * \param y      Esquina superior.
 * \param ancho  Ancho.
 * \param alto   Alto.
 * \return El rect&aacute;ngulo.
 */
Rectangle rectangulo(float x, float y, float ancho, float alto);

/**
 * \brief Lee la nueva posici&oacute;n de un deslizador.
 *
 * Va en la parte de **actualizar**. Mientras se mantenga apretado el bot&oacute;n
 * izquierdo, el valor sigue al rat&oacute;n; se puede arrastrar de largo sin salirse,
 * porque el valor se recorta a los extremos.
 *
 * \param rec         Zona del deslizador.
 * \param valorActual Valor que tiene ahora, de 0.0 a 1.0.
 * \return El valor nuevo, o el mismo si nadie lo est&aacute; arrastrando.
 */
float valorDeslizador(Rectangle rec, float valorActual);

/**
 * \brief Dibuja un deslizador con su parte llena y su perilla.
 * \param rec   Zona del deslizador.
 * \param valor Valor de 0.0 a 1.0.
 */
void dibujarDeslizador(Rectangle rec, float valor);

/**
 * \brief Mueve un &iacute;ndice de enfoque con las flechas del teclado.
 *
 * El enfoque da la vuelta: del &uacute;ltimo control se pasa al primero. Se suma
 * @p cuantos antes del residuo porque en C++ el residuo de un negativo es negativo.
 *
 * \param enfoque      &Iacute;ndice del control enfocado ahora.
 * \param cuantos      Cu&aacute;ntos controles hay.
 * \param conLaterales Si las flechas izquierda y derecha tambi&eacute;n mueven. Se pone en
 *                     falso cuando esas dos hacen otra cosa, como ajustar el volumen.
 * \return El &iacute;ndice nuevo, o el mismo si no se presion&oacute; ninguna flecha.
 */
int moverEnfoque(int enfoque, int cuantos, bool conLaterales);

/**
 * \brief Si se acaba de pedir activar el control enfocado.
 *
 * Solo Enter, a prop&oacute;sito: la barra espaciadora escribe un espacio en los campos de
 * nombre, y una tecla que a veces activa y a veces escribe es una trampa.
 *
 * \return Verdadero en el fotograma en que se presion&oacute; Enter.
 */
bool enfoqueActivado();

/**
 * \brief Dibuja un anillo alrededor del control enfocado por teclado.
 *
 * Va por fuera del control y no encima, para que sirva igual sobre un bot&oacute;n, un
 * campo de texto o una imagen con transparencia.
 *
 * \param rec Control enfocado.
 */
void dibujarAnilloEnfoque(Rectangle rec);

#endif // BOTON_HPP_INCLUDED
