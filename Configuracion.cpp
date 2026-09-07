/**
 * \file Configuracion.cpp
 * \brief Implementaci&oacute;n de la pantalla de configuraci&oacute;n de partida.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstring>

#include "raylib.h"

#include "Configuracion.hpp"
#include "Dificultad.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO DE LA PANTALLA
//***********************************************

// Que campo de texto tiene el cursor. 0 es ninguno, 1 el primer nombre y 2 el
// segundo. Se guarda aqui porque es estado de la pantalla, no de la partida: al
// jugador no le importa y a ConfigPartida tampoco.
static int campoConFoco = 0;

static Rectangle botonModo(int indice)
{
    return rectangulo(80.0f + indice * 250.0f, 145.0f, 230.0f, 52.0f);
}

static Rectangle botonDificultad(int indice)
{
    return rectangulo(80.0f + indice * 165.0f, 262.0f, 150.0f, 52.0f);
}

static Rectangle campoNombre(int numJugador)
{
    return rectangulo(80.0f, 375.0f + (numJugador - 1) * 62.0f, 330.0f, 46.0f);
}

static Rectangle panelResumen()
{
    return rectangulo(640.0f, 120.0f, 560.0f, 400.0f);
}

static Rectangle botonIniciar()
{
    Rectangle panel = panelResumen();

    return rectangulo(panel.x + panel.width - 220.0f,
                      panel.y + panel.height - 84.0f,
                      180.0f, 56.0f);
}

//***********************************************
// ESCRITURA DE NOMBRES
//***********************************************

/**
 * \brief Mete en un campo de texto lo que el jugador vaya tecleando.
 *
 * \param destino Arreglo de LARGO_NOMBRE + 1 caracteres.
 */
static void escribirEn(char* destino)
{
    // GetCharPressed devuelve las letras que se juntaron desde el fotograma
    // pasado, una por llamada, hasta que se acaban y regresa cero. Se usa esta y
    // no IsKeyPressed porque esta ya viene resuelta: respeta mayusculas, acentos
    // y la distribucion del teclado, cosas que a mano serian un desastre.
    int letra = GetCharPressed();

    while(letra > 0){

        int largo = (int)strlen(destino);

        // Se filtra a ASCII imprimible porque la fuente que trae raylib de fabrica
        // no cubre mas que eso: una letra acentuada se veria como un simbolo raro.
        // Y se revisa el largo ANTES de escribir: ese es el limite que evita
        // pasarse del arreglo.
        if(letra >= 32 && letra <= 125 && largo < LARGO_NOMBRE){
            destino[largo]     = (char)letra;
            destino[largo + 1] = '\0';
        }

        letra = GetCharPressed();
    }

    // El borrado se pregunta aparte: el retroceso no es un caracter que se
    // escriba, es una tecla que quita.
    if(IsKeyPressed(KEY_BACKSPACE)){
        int largo = (int)strlen(destino);

        if(largo > 0) destino[largo - 1] = '\0';
    }
}

//***********************************************
// ACTUALIZAR
//***********************************************

void PrepararConfiguracion(ConfigPartida& config)
{
    // Se rehace entera, no solo los nombres: si el nino anterior dejo Dificil y
    // multijugador puestos, el siguiente arranca en un juego que no pidio.
    config = configPorDefecto();

    campoConFoco = 0;
}

Escena_Estado ActualizarConfiguracion(ConfigPartida& config)
{
    if(IsKeyPressed(KEY_ESCAPE)) return Escena_menu;

    // ---- Modo ----
    if(botonClicado(botonModo(0))) config.modo = Modo_solitario;
    if(botonClicado(botonModo(1))) config.modo = Modo_multijugador;

    // ---- Dificultad ----
    for(int i = 0; i < NUM_DIFICULTADES; i++){
        if(botonClicado(botonDificultad(i))) config.dificultad = (Dificultad)i;
    }

    // ---- Foco de los campos de texto ----
    // Un clic en cualquier otro lado quita el foco. Asi el cursor parpadeando
    // siempre esta donde el jugador acaba de picar, y no en un campo olvidado.
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){

        if(ratonEncima(campoNombre(1))){
            campoConFoco = 1;
        } else if(config.modo == Modo_multijugador && ratonEncima(campoNombre(2))){
            campoConFoco = 2;
        } else {
            campoConFoco = 0;
        }
    }

    // Al cambiar a solitario el segundo campo desaparece; si tenia el cursor hay
    // que quitarselo o se seguiria escribiendo en un cuadro invisible.
    if(config.modo == Modo_solitario && campoConFoco == 2) campoConFoco = 0;

    if(campoConFoco == 1) escribirEn(config.nombre1);
    if(campoConFoco == 2) escribirEn(config.nombre2);

    // ---- Iniciar ----
    // Enter tambien arranca, como pedia el boceto. Funciona incluso escribiendo un
    // nombre: terminar de escribir y darle Enter es lo que uno espera.
    if(botonClicado(botonIniciar()) || IsKeyPressed(KEY_ENTER)) return Escena_juego;

    return Escena_configuracion;
}

//***********************************************
// DIBUJAR
//***********************************************

/**
 * \brief Dibuja un campo de texto con su contenido y, si toca, el cursor.
 *
 * \param rec       D&oacute;nde va.
 * \param texto     Contenido actual.
 * \param conFoco   Verdadero si es el campo donde se est&aacute; escribiendo.
 */
static void dibujarCampo(Rectangle rec, const char* texto, bool conFoco)
{
    DrawRectangleRounded(rec, 0.2f, 8, COLOR_BOTON);
    DrawRectangleRoundedLinesEx(rec, 0.2f, 8, conFoco ? 3.0f : 1.0f,
                                conFoco ? COLOR_SELECCION : COLOR_TENUE);

    const int TAMANO = 24;
    int x = (int)rec.x + 14;
    int y = (int)(rec.y + (rec.height - TAMANO) / 2.0f);

    DrawText(texto, x, y, TAMANO, COLOR_TEXTO);

    if(conFoco){
        // El cursor prende y apaga cada medio segundo. GetTime da los segundos
        // desde que abrio el juego; el residuo entre 1 parte ese segundo en dos
        // mitades, y en una se dibuja y en la otra no.
        bool visible = (GetTime() - (int)GetTime()) < 0.5;

        if(visible){
            int desplazado = x + MeasureText(texto, TAMANO) + 2;

            DrawRectangle(desplazado, y, 2, TAMANO, COLOR_SELECCION);
        }
    }
}

void DibujarConfiguracion(const ConfigPartida& config)
{
    dibujarTextoCentrado("CONFIGURACION DE LA PARTIDA", 32, 34, COLOR_TITULO);

    // ---- 1. Modo ----
    DrawText("1.  Modo", 80, 108, 22, COLOR_TEXTO);
    dibujarBoton(botonModo(0), "Solitario",    config.modo == Modo_solitario);
    dibujarBoton(botonModo(1), "Multijugador", config.modo == Modo_multijugador);

    // ---- 2. Dificultad ----
    DrawText("2.  Dificultad", 80, 226, 22, COLOR_TEXTO);

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        const InfoDificultad& nivel = DIFICULTADES[i];
        Rectangle             rec   = botonDificultad(i);

        dibujarBoton(rec, nivel.nombre, config.dificultad == (Dificultad)i);

        // Debajo de cada boton, cuantas cartas trae. Es el dato que de verdad le
        // dice al jugador que tan larga va a ser la partida.
        const char* cuantas = TextFormat("%d cartas", nivel.filas * nivel.columnas);
        int         anchoSub = MeasureText(cuantas, 16);

        DrawText(cuantas,
                 (int)(rec.x + (rec.width - anchoSub) / 2.0f),
                 (int)(rec.y + rec.height + 8.0f),
                 16, COLOR_TENUE);
    }

    // ---- 3. Jugadores ----
    DrawText(config.modo == Modo_multijugador ? "3.  Jugadores" : "3.  Jugador",
             80, 342, 22, COLOR_TEXTO);

    dibujarCampo(campoNombre(1), config.nombre1, campoConFoco == 1);

    if(config.modo == Modo_multijugador){
        dibujarCampo(campoNombre(2), config.nombre2, campoConFoco == 2);
    }

    // ---- Resumen ----
    Rectangle panel = panelResumen();

    DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);

    int x = (int)panel.x + 36;
    int y = (int)panel.y + 40;

    DrawText("Asi va a quedar", x, y, 24, COLOR_TITULO);

    const InfoDificultad& nivel = DIFICULTADES[config.dificultad];

    DrawText(TextFormat("Modo:        %s",
                        config.modo == Modo_solitario ? "Solitario" : "Multijugador"),
             x, y + 60, 22, COLOR_TEXTO);

    DrawText(TextFormat("Dificultad:  %s   %dx%d   (%d pares)",
                        nivel.nombre, nivel.filas, nivel.columnas,
                        (nivel.filas * nivel.columnas) / 2),
             x, y + 96, 22, COLOR_TEXTO);

    DrawText(TextFormat("Jugador:     %s", nombreDeJugador(config, 1)),
             x, y + 132, 22, COLOR_TEXTO);

    if(config.modo == Modo_multijugador){
        DrawText(TextFormat("Jugador:     %s", nombreDeJugador(config, 2)),
                 x, y + 168, 22, COLOR_TEXTO);
    }

    dibujarBoton(botonIniciar(), "Iniciar", true);

    dibujarTextoCentrado("ESC para volver al menu     ENTER para iniciar",
                         GetScreenHeight() - 40, 18, COLOR_TENUE);
}
