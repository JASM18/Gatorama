/**
 * \file Instrucciones.cpp
 * \brief Implementaci&oacute;n de la ventana de instrucciones.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Instrucciones.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// TEXTO
//***********************************************

// Los pasos van como datos y no como una ristra de DrawText con las Y escritas a
// mano. Asi, agregar o quitar un paso no obliga a recorrer todos los de abajo.
//
// Sin acentos: la fuente que trae raylib de fabrica solo cubre ASCII y una letra
// acentuada saldria como un simbolo raro. El dia que se cargue una fuente propia
// con LoadFontEx, esto se puede escribir bien.
const int NUM_PASOS = 5;

static const char* PASOS[NUM_PASOS] = {
    "1.  Hazle clic a una carta y se voltea.",
    "2.  Voltea una segunda carta.",
    "3.  Si las dos traen el mismo gato, se quedan destapadas.",
    "4.  Si no, se tapan solas y lo vuelves a intentar.",
    "5.  Ganas cuando encuentras todas las parejas."
};

static const float PANEL_ANCHO = 760.0f;
static const float PANEL_ALTO  = 420.0f;

static Rectangle panelInstrucciones()
{
    return rectangulo((GetScreenWidth()  - PANEL_ANCHO) / 2.0f,
                      (GetScreenHeight() - PANEL_ALTO ) / 2.0f,
                      PANEL_ANCHO, PANEL_ALTO);
}

/**
 * \brief La tacha de cerrar, en la esquina del panel.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonCerrar()
{
    Rectangle panel = panelInstrucciones();

    return rectangulo(panel.x + panel.width - 60.0f, panel.y + 18.0f, 42.0f, 42.0f);
}

//***********************************************
// VENTANA DE INSTRUCCIONES
//***********************************************

bool ActualizarInstrucciones()
{
    if(IsKeyPressed(KEY_ESCAPE))      return true;
    if(botonClicado(botonCerrar()))   return true;

    return false;
}

void DibujarInstrucciones(ModoJuego modo)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), COLOR_VELO);

    Rectangle panel = panelInstrucciones();

    DrawRectangleRounded(panel, 0.05f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.05f, 10, 2.0f, COLOR_SELECCION);

    DrawText("COMO SE JUEGA", (int)panel.x + 40, (int)panel.y + 28, 32, COLOR_TITULO);

    dibujarBoton(botonCerrar(), "X", false);

    int y = (int)panel.y + 100;

    for(int i = 0; i < NUM_PASOS; i++){
        DrawText(PASOS[i], (int)panel.x + 44, y, 22, COLOR_TEXTO);
        y += 40;
    }

    // El ultimo renglon depende del modo. En solitario se habla del reloj; en dos
    // jugadores todavia no se promete nada de turnos ni puntaje, porque el juego
    // aun no los hace.
    const char* nota = (modo == Modo_solitario)
                     ? "El reloj corre mientras juegas. En pausa se detiene."
                     : "Los dos jugadores comparten el tablero.";

    DrawText(nota, (int)panel.x + 44, y + 12, 20, COLOR_SELECCION);

    dibujarTextoCentrado("ESC o la tacha para cerrar",
                         (int)(panel.y + panel.height - 34.0f), 18, COLOR_TENUE);
}
