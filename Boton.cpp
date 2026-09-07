/**
 * \file Boton.cpp
 * \brief Implementaci&oacute;n de los botones con rat&oacute;n.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Boton.hpp"
#include "Tema.hpp"

Rectangle rectangulo(float x, float y, float ancho, float alto)
{
    Rectangle rec = { x, y, ancho, alto };
    return rec;
}

bool ratonEncima(Rectangle rec)
{
    return CheckCollisionPointRec(GetMousePosition(), rec);
}

bool botonClicado(Rectangle rec)
{
    // Se usa IsMouseButtonPressed y no IsMouseButtonReleased. La diferencia se
    // siente: con Pressed el boton responde en el instante en que se aprieta, y
    // con Released hasta que se suelta. Para un stand con ninos apurados, que
    // responda de inmediato vale mas que poder arrepentirse arrastrando el raton
    // fuera del boton antes de soltar.
    return ratonEncima(rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void dibujarBoton(Rectangle rec, const char* etiqueta, bool seleccionado)
{
    // Preguntar por la posicion del raton al dibujar no rompe la separacion entre
    // actualizar y dibujar: leer donde esta el puntero no cambia nada. Lo que no
    // se vale aqui es decidir cosas, y de eso se encarga botonClicado.
    bool encima = ratonEncima(rec);

    Color fondo;
    Color textoColor;

    if(seleccionado){
        fondo      = COLOR_BOTON_ACTIVO;
        textoColor = COLOR_FONDO;          // texto oscuro sobre el boton claro
    } else if(encima){
        fondo      = COLOR_BOTON_HOVER;
        textoColor = COLOR_TEXTO;
    } else {
        fondo      = COLOR_BOTON;
        textoColor = COLOR_TEXTO;
    }

    const float REDONDEZ  = 0.25f;
    const int   SEGMENTOS = 8;

    DrawRectangleRounded(rec, REDONDEZ, SEGMENTOS, fondo);

    // El contorno solo aparece con el raton encima: marca cual se va a activar sin
    // llenar la pantalla de lineas cuando no hace falta.
    if(encima && !seleccionado){
        DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, 2.0f, COLOR_SELECCION);
    }

    // La letra se escala con el alto del boton para que un boton chico no se vea
    // con el texto encimado ni uno grande con el texto perdido.
    int tamano = (int)(rec.height * 0.42f);
    if(tamano < 12) tamano = 12;

    int ancho = MeasureText(etiqueta, tamano);

    DrawText(etiqueta,
             (int)(rec.x + (rec.width  - ancho ) / 2.0f),
             (int)(rec.y + (rec.height - tamano) / 2.0f),
             tamano, textoColor);
}

//***********************************************
// DESLIZADOR
//***********************************************

float valorDeslizador(Rectangle rec, float valorActual)
{
    if(!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) return valorActual;

    Vector2 raton = GetMousePosition();

    // Se acepta el arrastre si el raton esta a la altura del deslizador, aunque se
    // haya salido por los lados. Es lo que uno espera de una barra de volumen:
    // arrastras de largo y se queda pegada en el maximo, no se suelta a medias.
    const float TOLERANCIA = 14.0f;

    if(raton.y < rec.y - TOLERANCIA)              return valorActual;
    if(raton.y > rec.y + rec.height + TOLERANCIA) return valorActual;

    float nuevo = (raton.x - rec.x) / rec.width;

    if(nuevo < 0.0f) nuevo = 0.0f;
    if(nuevo > 1.0f) nuevo = 1.0f;

    return nuevo;
}

void dibujarDeslizador(Rectangle rec, float valor)
{
    if(valor < 0.0f) valor = 0.0f;
    if(valor > 1.0f) valor = 1.0f;

    // La barra de atras: todo el recorrido posible.
    DrawRectangleRounded(rec, 1.0f, 8, COLOR_BOTON);

    // La parte llena. Se dibuja encima y del mismo alto, asi el borde redondeado
    // de la izquierda coincide con el de la barra y no se ve un escalon.
    Rectangle lleno = rec;
    lleno.width = rec.width * valor;

    if(lleno.width > rec.height){
        DrawRectangleRounded(lleno, 1.0f, 8, COLOR_BOTON_ACTIVO);
    }

    // La perilla marca donde va. Se mantiene dentro de la barra restandole su
    // propio radio en los extremos, para que no se asome por fuera.
    float radio  = rec.height * 0.85f;
    float centro = rec.x + radio + (rec.width - radio * 2.0f) * valor;

    Vector2 posicion = { centro, rec.y + rec.height / 2.0f };

    DrawCircleV(posicion, radio, COLOR_SELECCION);
}
