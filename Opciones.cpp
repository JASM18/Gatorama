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

/**
 * \brief La barra de volumen.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle barraVolumen()
{
    Rectangle panel = panelOpciones();

    return rectangulo(panel.x + 50.0f, panel.y + 130.0f, panel.width - 160.0f, 18.0f);
}

static Rectangle botonVolver()
{
    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 130.0f, 220.0f, 52.0f);
}

// Dos controles: el volumen y el boton de volver.
const int ENFOQUE_VOLUMEN = 0;
const int NUM_CONTROLES   = 2;

static int enfoque = ENFOQUE_VOLUMEN;

//***********************************************
// PANTALLA
//***********************************************

Escena_Estado ActualizarOpciones()
{
    if(ratonEncima(barraVolumen())) enfoque = ENFOQUE_VOLUMEN;
    if(ratonEncima(botonVolver()))  enfoque = 1;

    bool enVolumen = (enfoque == ENFOQUE_VOLUMEN);

    // Sobre el volumen, izquierda y derecha ajustan en vez de cambiar de control.
    enfoque = moverEnfoque(enfoque, NUM_CONTROLES, !enVolumen);

    if(enVolumen){
        // Por segundo y no por fotograma, para que suba igual de rapido en
        // cualquier maquina.
        float paso = 0.6f * GetFrameTime();

        if(IsKeyDown(KEY_RIGHT)) FijarVolumenMusica(VolumenMusica() + paso);
        if(IsKeyDown(KEY_LEFT))  FijarVolumenMusica(VolumenMusica() - paso);
    }

    // El volumen del raton se atiende antes que el boton. Si se hiciera al reves,
    // soltar el raton sobre la barra tras arrastrarla podria contar como clic en
    // lo que quedara debajo.
    FijarVolumenMusica(valorDeslizador(barraVolumen(), VolumenMusica()));

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonVolver())) return Escena_menu;

    if(enfoqueActivado() && enfoque == 1) return Escena_menu;

    return Escena_opciones;
}

void DibujarOpciones()
{
    dibujarTextoCentrado("OPCIONES", 60, 42, COLOR_TITULO);

    Rectangle panel = panelOpciones();

    DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);

    DrawText("Musica", (int)panel.x + 50, (int)panel.y + 60, 26, COLOR_TEXTO);

    DrawText("El mismo volumen que se ajusta desde la pausa",
             (int)panel.x + 50, (int)panel.y + 96, 16, COLOR_TENUE);

    Rectangle barra = barraVolumen();

    dibujarDeslizador(barra, VolumenMusica());

    if(enfoque == ENFOQUE_VOLUMEN) dibujarAnilloEnfoque(barra);

    dibujarDato(TextFormat("%d%%", (int)(VolumenMusica() * 100.0f + 0.5f)),
                (int)(barra.x + barra.width + 24.0f), (int)(barra.y - 6.0f),
                24, COLOR_SELECCION);

    dibujarTextoCentrado("Mas opciones cuando haya efectos de sonido",
                         (int)(panel.y + panel.height - 50.0f), 16, COLOR_TENUE);

    dibujarBoton(botonVolver(), "Volver al menu", false);

    if(enfoque == 1) dibujarAnilloEnfoque(botonVolver());
}
