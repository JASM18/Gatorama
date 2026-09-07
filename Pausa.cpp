/**
 * \file Pausa.cpp
 * \brief Implementaci&oacute;n de la ventana de pausa.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Pausa.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO DEL PANEL
//***********************************************

const int NUM_BOTONES_PAUSA = 3;

static const char* ETIQUETAS_PAUSA[NUM_BOTONES_PAUSA] = {
    "Continuar",
    "Reiniciar partida",
    "Regresar al menu"
};

// Cada boton lleva a una accion. Mismo truco que el menu principal: dos arreglos
// en paralelo, para que agregar una opcion sea agregar dos renglones de datos.
static const AccionPausa ACCIONES_PAUSA[NUM_BOTONES_PAUSA] = {
    Pausa_continuar,
    Pausa_reiniciar,
    Pausa_menu
};

static const float PANEL_ANCHO = 420.0f;
static const float PANEL_ALTO  = 320.0f;

/**
 * \brief El rect&aacute;ngulo del panel, centrado en la ventana.
 *
 * Actualizar y dibujar lo calculan cada quien por su lado en vez de guardarlo. Es
 * una resta: sale m&aacute;s barato que arriesgarse a que el bot&oacute;n se dibuje en un lugar
 * y se detecte el clic en otro.
 */
static Rectangle panelPausa()
{
    return rectangulo((GetScreenWidth()  - PANEL_ANCHO) / 2.0f,
                      (GetScreenHeight() - PANEL_ALTO ) / 2.0f,
                      PANEL_ANCHO, PANEL_ALTO);
}

/**
 * \brief D&oacute;nde queda uno de los botones del panel.
 * \param indice Bot&oacute;n, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonPausa(int indice)
{
    Rectangle panel = panelPausa();

    const float MARGEN   = 40.0f;
    const float ALTO     = 52.0f;
    const float SEPARA   = 18.0f;
    const float PRIMERO  = 110.0f;   // debajo del titulo

    return rectangulo(panel.x + MARGEN,
                      panel.y + PRIMERO + indice * (ALTO + SEPARA),
                      panel.width - MARGEN * 2.0f,
                      ALTO);
}

//***********************************************
// VENTANA DE PAUSA
//***********************************************

AccionPausa ActualizarPausa()
{
    // La misma tecla que abre la pausa la cierra. Si ESC hiciera otra cosa aqui
    // -por ejemplo salir al menu- seria facil perder una partida sin querer.
    if(IsKeyPressed(KEY_ESCAPE)) return Pausa_continuar;

    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){
        if(botonClicado(botonPausa(i))) return ACCIONES_PAUSA[i];
    }

    return Pausa_ninguna;
}

void DibujarPausa()
{
    // El velo cubre toda la ventana. Es semitransparente a proposito: el tablero
    // se sigue viendo debajo, y eso es lo que hace que se lea como "el juego
    // sigue ahi, en pausa" y no como "me cambiaron de pantalla".
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), COLOR_VELO);

    Rectangle panel = panelPausa();

    DrawRectangleRounded(panel, 0.08f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 10, 2.0f, COLOR_SELECCION);

    const char* titulo = "PAUSA";
    int tamano = 40;
    int ancho  = MeasureText(titulo, tamano);

    DrawText(titulo,
             (int)(panel.x + (panel.width - ancho) / 2.0f),
             (int)(panel.y + 38.0f),
             tamano, COLOR_TITULO);

    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){
        // Ninguno va marcado como seleccionado: son acciones, no opciones entre
        // las que se escoge una y se queda encendida.
        dibujarBoton(botonPausa(i), ETIQUETAS_PAUSA[i], false);
    }

    dibujarTextoCentrado("ESC para seguir jugando",
                         (int)(panel.y + panel.height - 34.0f), 18, COLOR_TENUE);
}
