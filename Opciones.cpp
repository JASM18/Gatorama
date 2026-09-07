/**
 * \file Opciones.cpp
 * \brief Implementaci&oacute;n de la pantalla de opciones.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Opciones.hpp"
#include "Audio.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO
//***********************************************

static Rectangle panelOpciones()
{
    return rectangulo((GetScreenWidth() - 660.0f) / 2.0f, 150.0f, 660.0f, 300.0f);
}

// Las dos categorias de sonido, en el orden en que se ven y se recorren.
const int SONIDO_MUSICA  = 0;
const int SONIDO_EFECTOS = 1;
const int NUM_SONIDOS    = 2;

static const char* NOMBRES_SONIDO[NUM_SONIDOS] = { "Musica", "SFX" };

/**
 * \brief La barra de una de las dos categor&iacute;as de sonido.
 * \param cual SONIDO_MUSICA o SONIDO_EFECTOS.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle barraVolumen(int cual)
{
    Rectangle panel = panelOpciones();

    return rectangulo(panel.x + 50.0f, panel.y + 88.0f + cual * 90.0f,
                      panel.width - 160.0f, 18.0f);
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

static Rectangle botonVolver()
{
    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 130.0f, 220.0f, 52.0f);
}

// Tres controles: las dos barras y el boton de volver.
const int CTRL_VOLVER   = NUM_SONIDOS;
const int NUM_CONTROLES = NUM_SONIDOS + 1;

static int enfoque = SONIDO_MUSICA;

//***********************************************
// PANTALLA
//***********************************************

Escena_Estado ActualizarOpciones()
{
    for(int i = 0; i < NUM_SONIDOS; i++){
        if(ratonEncima(barraVolumen(i))) enfoque = i;
    }

    if(ratonEncima(botonVolver())) enfoque = CTRL_VOLVER;

    bool enBarra = (enfoque < NUM_SONIDOS);

    // Sobre una barra, izquierda y derecha ajustan en vez de cambiar de control.
    enfoque = moverEnfoque(enfoque, NUM_CONTROLES, !enBarra);

    if(enBarra){
        // Por segundo y no por fotograma, para que suba igual de rapido en
        // cualquier maquina.
        float paso = 0.6f * GetFrameTime();

        if(IsKeyDown(KEY_RIGHT)) fijarVolumenDe(enfoque, volumenDe(enfoque) + paso);
        if(IsKeyDown(KEY_LEFT))  fijarVolumenDe(enfoque, volumenDe(enfoque) - paso);
    }

    // Las barras se atienden antes que el boton. Si se hiciera al reves, soltar el
    // raton sobre una tras arrastrarla podria contar como clic en lo que quedara
    // debajo.
    for(int i = 0; i < NUM_SONIDOS; i++){
        fijarVolumenDe(i, valorDeslizador(barraVolumen(i), volumenDe(i)));
    }

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonVolver())) return Escena_menu;

    if(enfoqueActivado() && enfoque == CTRL_VOLVER) return Escena_menu;

    return Escena_opciones;
}

void DibujarOpciones()
{
    dibujarTextoCentrado("OPCIONES", 60, 42, COLOR_TITULO);

    Rectangle panel = panelOpciones();

    DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);

    for(int i = 0; i < NUM_SONIDOS; i++){

        Rectangle barra = barraVolumen(i);

        DrawText(NOMBRES_SONIDO[i], (int)barra.x, (int)(barra.y - 34.0f), 24, COLOR_TEXTO);

        dibujarDeslizador(barra, volumenDe(i));

        if(enfoque == i) dibujarAnilloEnfoque(barra);

        dibujarDato(TextFormat("%d%%", (int)(volumenDe(i) * 100.0f + 0.5f)),
                    (int)(barra.x + barra.width + 24.0f), (int)(barra.y - 6.0f),
                    24, COLOR_SELECCION);
    }

    DrawText("La musica acompana de fondo; los SFX avisan que algo paso",
             (int)panel.x + 50, (int)(panel.y + panel.height - 46.0f), 16, COLOR_TENUE);

    dibujarBoton(botonVolver(), "Volver al menu", false);

    if(enfoque == CTRL_VOLVER) dibujarAnilloEnfoque(botonVolver());
}
