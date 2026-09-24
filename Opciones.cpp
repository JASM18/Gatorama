/**
 * \file Opciones.cpp
 * \brief Implementaci&oacute;n de la ventana de opciones.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 24/09/2026
 */

#include "raylib.h"

#include "Opciones.hpp"
#include "Audio.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO DEL PANEL
//***********************************************

// Las dos categorias de sonido, en el orden en que se ven y se recorren. Mismos
// nombres que en la pausa, para que sean la misma cosa en las dos ventanas.
const int SONIDO_MUSICA  = 0;
const int SONIDO_EFECTOS = 1;
const int NUM_SONIDOS    = 2;

static const char* NOMBRES_SONIDO[NUM_SONIDOS] = { "Musica", "Sonido" };

// Tres controles: las dos barras y el boton de regresar. La tacha no cuenta, igual
// que en la pausa: es un camino de mas para el raton, no una parada del teclado.
const int CTRL_REGRESAR = NUM_SONIDOS;
const int NUM_CONTROLES = NUM_SONIDOS + 1;
const int SIN_ENFOQUE   = -1;

// Mismo modelo que el cursor de cartas: empieza en nada, el raton lo mueve solo
// cuando de verdad se mueve, y las flechas lo estrenan.
static int enfoque = SIN_ENFOQUE;

// Mismo ancho que la pausa y la mitad de alto: es la pausa con menos cosas.
static const float PANEL_ANCHO = 440.0f;
static const float PANEL_ALTO  = 300.0f;

static Rectangle panelOpciones()
{
    return rectangulo((GetScreenWidth()  - PANEL_ANCHO) / 2.0f,
                      (GetScreenHeight() - PANEL_ALTO ) / 2.0f,
                      PANEL_ANCHO, PANEL_ALTO);
}

static Rectangle botonCerrar()
{
    Rectangle panel = panelOpciones();

    return rectangulo(panel.x + 18.0f, panel.y + 18.0f, 42.0f, 42.0f);
}

/**
 * \brief La barra de una de las dos categor&iacute;as de sonido.
 * \param cual SONIDO_MUSICA o SONIDO_EFECTOS.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle barraVolumen(int cual)
{
    Rectangle panel = panelOpciones();

    return rectangulo(panel.x + 150.0f, panel.y + 112.0f + cual * 56.0f, 195.0f, 16.0f);
}

static Rectangle botonRegresar()
{
    Rectangle panel = panelOpciones();

    return rectangulo(panel.x + 70.0f, panel.y + 214.0f, panel.width - 140.0f, 50.0f);
}

/**
 * \brief El volumen de una categor&iacute;a.
 * \param cual SONIDO_MUSICA o SONIDO_EFECTOS.
 * \return Su volumen actual.
 */
static float volumenDe(int cual)
{
    return (cual == SONIDO_EFECTOS) ? VolumenEfectos() : VolumenMusica();
}

/**
 * \brief Cambia el volumen de una categor&iacute;a.
 * \param cual  SONIDO_MUSICA o SONIDO_EFECTOS.
 * \param valor Nuevo volumen.
 */
static void fijarVolumenDe(int cual, float valor)
{
    if(cual == SONIDO_EFECTOS) FijarVolumenEfectos(valor);
    else                       FijarVolumenMusica(valor);
}

/**
 * \brief Qu&eacute; control est&aacute; bajo el puntero, o SIN_ENFOQUE si ninguno.
 */
static int controlBajoElRaton()
{
    for(int i = 0; i < NUM_SONIDOS; i++){
        if(ratonEncima(barraVolumen(i))) return i;
    }

    if(ratonEncima(botonRegresar())) return CTRL_REGRESAR;

    return SIN_ENFOQUE;
}

//***********************************************
// VENTANA
//***********************************************

void PrepararOpciones()
{
    enfoque = SIN_ENFOQUE;
}

bool ActualizarOpciones()
{
    // El raton solo cuenta cuando de verdad se movio: si se leyera cada fotograma,
    // pisaria al instante lo que se acaba de elegir con las flechas.
    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        enfoque = controlBajoElRaton();
    }

    bool enBarra = (enfoque != SIN_ENFOQUE && enfoque < NUM_SONIDOS);

    if(enfoque == SIN_ENFOQUE){
        if(IsKeyPressed(KEY_DOWN)) enfoque = 0;
        if(IsKeyPressed(KEY_UP))   enfoque = CTRL_REGRESAR;
    } else {
        // Sobre una barra, izquierda y derecha ajustan en vez de cambiar de control.
        enfoque = moverEnfoque(enfoque, NUM_CONTROLES, !enBarra);
    }

    if(enBarra){
        // Por segundo y no por fotograma, para que suba igual de rapido en
        // cualquier maquina.
        float paso = 0.6f * GetFrameTime();

        if(IsKeyDown(KEY_RIGHT)) fijarVolumenDe(enfoque, volumenDe(enfoque) + paso);
        if(IsKeyDown(KEY_LEFT))  fijarVolumenDe(enfoque, volumenDe(enfoque) - paso);
    }

    // Las barras se atienden antes que los botones. Si se hiciera al reves, soltar
    // el raton sobre una tras arrastrarla podria contar como clic en lo de abajo.
    for(int i = 0; i < NUM_SONIDOS; i++){
        fijarVolumenDe(i, valorDeslizador(barraVolumen(i), volumenDe(i)));
    }

    if(IsKeyPressed(KEY_ESCAPE))      return true;
    if(botonClicado(botonCerrar()))   return true;
    if(botonClicado(botonRegresar())) return true;

    if(enfoqueActivado() && enfoque == CTRL_REGRESAR) return true;

    return false;
}

void DibujarOpciones()
{
    // Mismo velo que la pausa: lo de abajo se sigue viendo, asi se lee como una
    // ventana encima y no como otra pantalla.
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), COLOR_VELO);

    Rectangle panel = panelOpciones();

    DrawRectangleRounded(panel, 0.08f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 10, 2.0f, COLOR_SELECCION);

    dibujarBoton(botonCerrar(), "X", false);

    const char* titulo = "OPCIONES";
    int tamano = 36;
    int ancho  = MeasureText(titulo, tamano);

    DrawText(titulo, (int)(panel.x + (panel.width - ancho) / 2.0f), (int)(panel.y + 28.0f),
             tamano, COLOR_TITULO);

    // ---- Volumen: musica y efectos ----
    for(int i = 0; i < NUM_SONIDOS; i++){

        Rectangle barra = barraVolumen(i);

        DrawText(NOMBRES_SONIDO[i], (int)panel.x + 40, (int)(barra.y - 2.0f), 20, COLOR_TEXTO);

        dibujarDeslizador(barra, volumenDe(i));

        if(enfoque == i) dibujarAnilloEnfoque(barra);

        dibujarDato(TextFormat("%d%%", (int)(volumenDe(i) * 100.0f + 0.5f)),
                    (int)(barra.x + barra.width + 18.0f), (int)(barra.y - 4.0f),
                    20, COLOR_TENUE);
    }

    dibujarBoton(botonRegresar(), "Regresar", false);

    if(enfoque == CTRL_REGRESAR) dibujarAnilloEnfoque(botonRegresar());
}
