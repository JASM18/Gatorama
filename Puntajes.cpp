/**
 * \file Puntajes.cpp
 * \brief Implementaci&oacute;n de la pantalla de mejores puntajes.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstring>

#include "raylib.h"

#include "Puntajes.hpp"
#include "TablaPuntajes.hpp"
#include "Dificultad.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ESTADO DE LA PANTALLA
//***********************************************

// Cuantos renglones se muestran. Ocho caben sin apretar y son los que a alguien
// le interesa ver: nadie va al noveno lugar.
const int RENGLONES = 8;

// Las dos pestanas.
const int PESTANA_PUNTOS  = 0;
const int PESTANA_TIEMPOS = 1;

static TablaPuntajes tabla;
static bool          cargada = false;

static int     pestana = PESTANA_PUNTOS;
static Puntaje mejores[RENGLONES];
static int     numMejores = 0;

/**
 * \brief Se asegura de que la tabla est&eacute; le&iacute;da del archivo.
 *
 * Se lee una sola vez por ejecuci&oacute;n. Volver a leerla en cada dibujo ser&iacute;a abrir el
 * archivo sesenta veces por segundo para nada.
 */
static void asegurarCargada()
{
    if(cargada) return;

    tabla.Cargar(RUTA_PUNTAJES);
    cargada = true;
}

/**
 * \brief Rellena el arreglo de los mejores seg&uacute;n la pesta&ntilde;a elegida.
 *
 * Se hace al cambiar de pesta&ntilde;a y al entrar, no al dibujar: armar un mont&iacute;culo por
 * fotograma ser&iacute;a trabajo tirado a la basura sesenta veces por segundo.
 */
static void recalcularMejores()
{
    if(pestana == PESTANA_TIEMPOS){
        // Solo partidas de un jugador: en multijugador el tiempo es de la partida
        // y no de una persona, asi que el mismo tiempo aparecería dos veces con
        // dos nombres distintos, como si fueran dos logros.
        numMejores = tabla.Mejores(mejores, RENGLONES, mejorPorTiempo, true);
    } else {
        numMejores = tabla.Mejores(mejores, RENGLONES, mejorPorPuntos, false);
    }
}

//***********************************************
// REGISTRO DE PARTIDAS
//***********************************************

void GuardarResultado(const ConfigPartida& config, const Partida& partida)
{
    asegurarCargada();

    for(int j = 0; j < partida.NumJugadores(); j++){

        Puntaje p = puntajeVacio();

        strncpy(p.nombre, nombreDeJugador(config, j + 1), LARGO_NOMBRE);
        p.nombre[LARGO_NOMBRE] = '\0';

        p.modo       = config.modo;
        p.dificultad = config.dificultad;
        p.pares      = partida.ParesDe(j);
        p.puntos     = partida.PuntajeDe(j);
        p.racha      = partida.RachaMaximaDe(j);   // la mejor de la partida, no la de ahorita
        p.intentos   = partida.Intentos();
        p.tiempo     = partida.Tiempo();
        p.gano       = (partida.Ganador() == j);

        tabla.Agregar(p);
    }

    // Se escribe de inmediato y no al cerrar el juego. En un stand alguien va a
    // cerrar la ventana con la tacha, y todo lo que no se haya guardado se pierde.
    tabla.Guardar(RUTA_PUNTAJES);
}

//***********************************************
// PANTALLA
//***********************************************

static Rectangle botonPestana(int cual)
{
    return rectangulo(340.0f + cual * 280.0f, 92.0f, 260.0f, 46.0f);
}

static Rectangle botonVolver()
{
    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 78.0f, 220.0f, 48.0f);
}

void PrepararPuntajes()
{
    // Se relee por si otra cosa toco el archivo, y porque entrar a la pantalla es
    // exactamente el momento en que importa estar al dia.
    tabla.Cargar(RUTA_PUNTAJES);
    cargada = true;

    pestana = PESTANA_PUNTOS;
    recalcularMejores();
}

Escena_Estado ActualizarPuntajes()
{
    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonVolver())) return Escena_menu;

    for(int i = 0; i < 2; i++){
        if(botonClicado(botonPestana(i)) && pestana != i){
            pestana = i;
            recalcularMejores();
        }
    }

    return Escena_puntajes;
}

/**
 * \brief Escribe un tiempo en segundos como minutos:segundos.
 * \param segundos Tiempo de la partida.
 * \return Cadena lista para dibujar, v&aacute;lida hasta la siguiente llamada.
 */
static const char* comoReloj(float segundos)
{
    int total = (int)segundos;

    return TextFormat("%02d:%02d", total / 60, total % 60);
}

/**
 * \brief Color del rengl&oacute;n seg&uacute;n el lugar.
 * \param lugar Posici&oacute;n empezando en cero.
 * \return Color del texto.
 */
static Color colorDeLugar(int lugar)
{
    if(lugar == 0) return COLOR_BOTON_ACTIVO;   // el primero resalta
    if(lugar < 3)  return COLOR_SELECCION;      // segundo y tercero, un poco

    return COLOR_TEXTO;
}

/**
 * \brief Dibuja el encabezado y los renglones de la tabla de puntos.
 */
static void dibujarTablaPuntos()
{
    const int X_LUGAR = 130, X_NOMBRE = 190, X_PUNTOS = 430;
    const int X_RACHA = 560, X_PARES  = 680, X_NIVEL  = 800, X_MODO = 960;

    DrawText("#",          X_LUGAR,  178, 18, COLOR_TENUE);
    DrawText("Jugador",    X_NOMBRE, 178, 18, COLOR_TENUE);
    DrawText("Puntos",     X_PUNTOS, 178, 18, COLOR_TENUE);
    DrawText("Racha",      X_RACHA,  178, 18, COLOR_TENUE);
    DrawText("Pares",      X_PARES,  178, 18, COLOR_TENUE);
    DrawText("Dificultad", X_NIVEL,  178, 18, COLOR_TENUE);
    DrawText("Modo",       X_MODO,   178, 18, COLOR_TENUE);

    for(int i = 0; i < numMejores; i++){
        const Puntaje& p = mejores[i];

        int   y     = 216 + i * 40;
        Color color = colorDeLugar(i);

        DrawText(TextFormat("%d", i + 1), X_LUGAR,  y, 22, color);
        DrawText(p.nombre,                X_NOMBRE, y, 22, color);
        DrawText(TextFormat("%d", p.puntos),   X_PUNTOS, y, 22, color);
        DrawText(TextFormat("%d", p.racha),    X_RACHA,  y, 22, color);
        DrawText(TextFormat("%d", p.pares),    X_PARES,  y, 22, color);
        DrawText(DIFICULTADES[p.dificultad].nombre, X_NIVEL, y, 22, COLOR_TEXTO);
        DrawText(p.modo == Modo_solitario ? "Solo" : "1 vs 1", X_MODO, y, 22, COLOR_TEXTO);
    }
}

/**
 * \brief Dibuja el encabezado y los renglones de la tabla de tiempos.
 */
static void dibujarTablaTiempos()
{
    const int X_LUGAR = 200, X_NOMBRE = 260, X_TIEMPO = 520;
    const int X_PUNTOS = 680, X_NIVEL = 820;

    DrawText("#",          X_LUGAR,  178, 18, COLOR_TENUE);
    DrawText("Jugador",    X_NOMBRE, 178, 18, COLOR_TENUE);
    DrawText("Tiempo",     X_TIEMPO, 178, 18, COLOR_TENUE);
    DrawText("Puntos",     X_PUNTOS, 178, 18, COLOR_TENUE);
    DrawText("Dificultad", X_NIVEL,  178, 18, COLOR_TENUE);

    for(int i = 0; i < numMejores; i++){
        const Puntaje& p = mejores[i];

        int   y     = 216 + i * 40;
        Color color = colorDeLugar(i);

        DrawText(TextFormat("%d", i + 1), X_LUGAR,  y, 22, color);
        DrawText(p.nombre,                X_NOMBRE, y, 22, color);
        DrawText(comoReloj(p.tiempo),     X_TIEMPO, y, 22, color);
        DrawText(TextFormat("%d", p.puntos), X_PUNTOS, y, 22, COLOR_TEXTO);
        DrawText(DIFICULTADES[p.dificultad].nombre, X_NIVEL, y, 22, COLOR_TEXTO);
    }

    dibujarTextoCentrado("Solo cuentan las partidas de un jugador: en 1 vs 1 el tiempo es de los dos",
                         GetScreenHeight() - 118, 16, COLOR_TENUE);
}

void DibujarPuntajes()
{
    dibujarTextoCentrado("MEJORES PUNTAJES", 34, 38, COLOR_TITULO);

    dibujarBoton(botonPestana(0), "Puntos",         pestana == PESTANA_PUNTOS);
    dibujarBoton(botonPestana(1), "Mejores tiempos", pestana == PESTANA_TIEMPOS);

    // Raya bajo el encabezado, para separar los titulos de columna de los datos.
    DrawLine(120, 206, GetScreenWidth() - 120, 206, COLOR_TENUE);

    if(numMejores == 0){
        dibujarTextoCentrado("Todavia no hay partidas registradas", 280, 24, COLOR_TEXTO);
        dibujarTextoCentrado("Juega una y aparece aqui", 320, 20, COLOR_TENUE);
    }
    else if(pestana == PESTANA_TIEMPOS){
        dibujarTablaTiempos();
    } else {
        dibujarTablaPuntos();
    }

    dibujarBoton(botonVolver(), "Volver al menu", false);
}
