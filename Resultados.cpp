/**
 * \file Resultados.cpp
 * \brief Implementaci&oacute;n de la pantalla de fin de partida.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstring>

#include "raylib.h"

#include "Resultados.hpp"
#include "Dificultad.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// LA COPIA DE LOS NUMEROS
//***********************************************

static char       nombres[MAX_JUGADORES][LARGO_NOMBRE + 1];
static int        pares[MAX_JUGADORES];
static int        puntos[MAX_JUGADORES];
static int        rachas[MAX_JUGADORES];
static float      tiempos[MAX_JUGADORES];

// Dos botones. El enfoque arranca en "Jugar otra vez" porque es lo que casi
// siempre quiere el que acaba de jugar.
const int NUM_CONTROLES = 2;

static int enfoque = 0;

static int        numJugadores = 1;
static int        ganador      = 0;
static int        intentos     = 0;
static float      tiempo       = 0.0f;
static Dificultad dificultad   = Dificultad_facil;

void PrepararResultados(const ConfigPartida& config, const Partida& partida)
{
    enfoque = 0;

    numJugadores = partida.NumJugadores();
    ganador      = partida.Ganador();
    intentos     = partida.Intentos();
    tiempo       = partida.Tiempo();
    dificultad   = config.dificultad;

    for(int j = 0; j < MAX_JUGADORES; j++){

        // Se copian los dos casilleros aunque juegue uno solo: dejar basura en el
        // del jugador que no existe es como se cuelan datos raros en pantalla.
        strncpy(nombres[j], nombreDeJugador(config, j + 1), LARGO_NOMBRE);
        nombres[j][LARGO_NOMBRE] = '\0';

        pares[j]  = partida.ParesDe(j);
        puntos[j] = partida.PuntajeDe(j);
        rachas[j]  = partida.RachaMaximaDe(j);
        tiempos[j] = partida.TiempoDe(j);
    }
}

//***********************************************
// ACOMODO
//***********************************************

static Rectangle panelResultados()
{
    return rectangulo((GetScreenWidth() - 780.0f) / 2.0f, 150.0f, 780.0f, 330.0f);
}

static Rectangle botonOtraVez()
{
    return rectangulo(GetScreenWidth() / 2.0f - 250.0f, GetScreenHeight() - 150.0f,
                      230.0f, 56.0f);
}

static Rectangle botonAlMenu()
{
    return rectangulo(GetScreenWidth() / 2.0f + 20.0f, GetScreenHeight() - 150.0f,
                      230.0f, 56.0f);
}

//***********************************************
// PANTALLA
//***********************************************

Escena_Estado ActualizarResultados()
{
    if(ratonEncima(botonOtraVez())) enfoque = 0;
    if(ratonEncima(botonAlMenu()))  enfoque = 1;

    enfoque = moverEnfoque(enfoque, NUM_CONTROLES, true);

    if(botonClicado(botonOtraVez())) return Escena_configuracion;

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonAlMenu())) return Escena_menu;

    if(enfoqueActivado()){
        return (enfoque == 0) ? Escena_configuracion : Escena_menu;
    }

    return Escena_resultados;
}

/**
 * \brief Escribe un tiempo en segundos como minutos:segundos.
 * \param segundos Duraci&oacute;n de la partida.
 * \return Cadena lista para dibujar, v&aacute;lida hasta la siguiente llamada.
 */
static const char* comoReloj(float segundos)
{
    int total = (int)segundos;

    return TextFormat("%02d:%02d", total / 60, total % 60);
}

/**
 * \brief Dibuja el rengl&oacute;n de un jugador dentro del panel.
 * \param jugador 0 o 1.
 * \param y       Altura donde va el rengl&oacute;n.
 */
static void dibujarRenglonJugador(int jugador, int y)
{
    Rectangle panel   = panelResultados();
    bool      esElQue = (numJugadores > 1 && jugador == ganador);

    int x = (int)panel.x + 40;

    // Al ganador se le pinta un recuadro detras. Es la senal que se lee de lejos,
    // desde atras de la mesa, sin tener que comparar dos numeros.
    if(esElQue){
        DrawRectangleRounded(rectangulo(panel.x + 24.0f, (float)y - 8.0f,
                                        panel.width - 48.0f, 44.0f),
                             0.3f, 8, COLOR_BOTON);
    }

    Color color = esElQue ? COLOR_BOTON_ACTIVO : COLOR_TEXTO;

    DrawText(nombres[jugador], x, y, 24, color);

    DrawText(TextFormat("%d parejas", pares[jugador]),  x + 220, y, 22, color);
    DrawText(TextFormat("%d puntos",  puntos[jugador]), x + 370, y, 22, COLOR_TEXTO);
    DrawText(TextFormat("racha %d",   rachas[jugador]), x + 500, y, 22, COLOR_TENUE);

    // El reloj de cada quien, no el de la partida. En 1 vs 1 es el unico numero
    // que permite decir quien se tardo mas: el de la partida los mezcla.
    DrawText(comoReloj(tiempos[jugador]), x + 620, y, 22, COLOR_TENUE);
}

void DibujarResultados()
{
    // ---- Titulo ----
    if(numJugadores == 1){
        dibujarTextoCentrado("PARTIDA TERMINADA", 60, 40, COLOR_TITULO);
    }
    else if(ganador >= 0){
        dibujarTextoCentrado(TextFormat("GANO %s", nombres[ganador]), 60, 44, COLOR_BOTON_ACTIVO);
    }
    else {
        dibujarTextoCentrado("EMPATE", 60, 44, COLOR_TITULO);
    }

    // ---- Panel ----
    Rectangle panel = panelResultados();

    DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);

    const InfoDificultad& nivel = DIFICULTADES[dificultad];

    DrawText(TextFormat("%s   %dx%d      Tiempo  %s      Intentos  %d",
                        nivel.nombre, nivel.filas, nivel.columnas,
                        comoReloj(tiempo), intentos),
             (int)panel.x + 40, (int)panel.y + 34, 22, COLOR_TENUE);

    for(int j = 0; j < numJugadores; j++){
        dibujarRenglonJugador(j, (int)panel.y + 110 + j * 64);
    }

    // Se recuerda la regla justo donde alguien podria discutirla: viendo a un
    // jugador con mas puntos que el ganador.
    if(numJugadores > 1){
        DrawText("Gana quien junta mas parejas. Los puntos son la medalla por las rachas.",
                 (int)panel.x + 40, (int)(panel.y + panel.height - 46.0f), 16, COLOR_TENUE);
    }

    dibujarBoton(botonOtraVez(), "Jugar otra vez", true);
    dibujarBoton(botonAlMenu(),  "Volver al menu", false);

    dibujarAnilloEnfoque((enfoque == 0) ? botonOtraVez() : botonAlMenu());
}
