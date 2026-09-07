/**
 * \file Pausa.cpp
 * \brief Implementaci&oacute;n de la ventana de pausa.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Pausa.hpp"
#include "Boton.hpp"
#include "Audio.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO DEL PANEL
//***********************************************

const int NUM_BOTONES_PAUSA = 4;

static const char* ETIQUETAS_PAUSA[NUM_BOTONES_PAUSA] = {
    "Continuar",
    "Reiniciar partida",
    "Como se juega",
    "Regresar al menu"
};

// Cada boton lleva a una accion. Mismo truco que el menu principal: dos arreglos
// en paralelo, para que agregar una opcion sea agregar dos renglones de datos.
static const AccionPausa ACCIONES_PAUSA[NUM_BOTONES_PAUSA] = {
    Pausa_continuar,
    Pausa_reiniciar,
    Pausa_instrucciones,
    Pausa_menu
};

// Las dos categorias de sonido, iguales que en la pantalla de Opciones.
const int SONIDO_MUSICA  = 0;
const int SONIDO_EFECTOS = 1;
const int NUM_SONIDOS    = 2;

static const char* NOMBRES_SONIDO[NUM_SONIDOS] = { "Musica", "SFX" };

// Controles que recorre el teclado: primero las dos barras, luego los botones. Se
// cuentan juntos porque las flechas los recorren como una sola lista.
const int NUM_ENFOQUES_PAUSA = NUM_SONIDOS + NUM_BOTONES_PAUSA;

static int enfoque = NUM_SONIDOS;   // arranca en Continuar

static const float PANEL_ANCHO = 440.0f;
static const float PANEL_ALTO  = 526.0f;

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
 * \brief La barra de una de las dos categor&iacute;as de sonido.
 * \param cual SONIDO_MUSICA o SONIDO_EFECTOS.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle barraVolumen(int cual)
{
    Rectangle panel = panelPausa();

    return rectangulo(panel.x + 40.0f, panel.y + 118.0f + cual * 62.0f,
                      panel.width - 130.0f, 16.0f);
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
 * \brief La tacha de cerrar, en la esquina del panel.
 *
 * Hace lo mismo que Continuar. Es un camino de m&aacute;s para el rat&oacute;n, no una acci&oacute;n
 * nueva, as&iacute; que **no entra en el recorrido del teclado**: ese ya tiene dos
 * salidas -ESC y el bot&oacute;n Continuar- y una tercera parada solo estorbar&iacute;a.
 *
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonCerrar()
{
    Rectangle panel = panelPausa();

    return rectangulo(panel.x + 18.0f, panel.y + 18.0f, 42.0f, 42.0f);
}

/**
 * \brief D&oacute;nde queda uno de los botones del panel.
 * \param indice Bot&oacute;n, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonPausa(int indice)
{
    Rectangle panel = panelPausa();

    const float MARGEN  = 40.0f;
    const float ALTO    = 50.0f;
    const float SEPARA  = 14.0f;
    const float PRIMERO = 234.0f;   // debajo de las dos barras

    return rectangulo(panel.x + MARGEN,
                      panel.y + PRIMERO + indice * (ALTO + SEPARA),
                      panel.width - MARGEN * 2.0f,
                      ALTO);
}

//***********************************************
// VENTANA DE PAUSA
//***********************************************

void PrepararPausa()
{
    enfoque = NUM_SONIDOS;   // el primer boton, que es Continuar
}

AccionPausa ActualizarPausa()
{
    // El raton manda sobre el teclado: si el puntero esta encima de un control,
    // ese toma el enfoque. Asi lo resaltado y lo que esta bajo el cursor no se
    // contradicen en pantalla.
    for(int i = 0; i < NUM_SONIDOS; i++){
        if(ratonEncima(barraVolumen(i))) enfoque = i;
    }

    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){
        if(ratonEncima(botonPausa(i))) enfoque = NUM_SONIDOS + i;
    }

    bool enBarra = (enfoque < NUM_SONIDOS);

    // Sobre una barra, izquierda y derecha ajustan en vez de cambiar de control.
    enfoque = moverEnfoque(enfoque, NUM_ENFOQUES_PAUSA, !enBarra);

    if(enBarra){
        // Se usa IsKeyDown y no IsKeyPressed para poder dejar la flecha apretada.
        // El paso va por segundo y no por fotograma, para que suba igual de rapido
        // en cualquier maquina: asi tarda poco menos de dos segundos de extremo a
        // extremo.
        float paso = 0.6f * GetFrameTime();

        if(IsKeyDown(KEY_RIGHT)) fijarVolumenDe(enfoque, volumenDe(enfoque) + paso);
        if(IsKeyDown(KEY_LEFT))  fijarVolumenDe(enfoque, volumenDe(enfoque) - paso);
    }

    // Las barras se atienden antes que los botones. Si se hiciera al reves, soltar
    // el raton sobre una despues de arrastrarla podria contar como clic en el
    // boton que quedo debajo.
    for(int i = 0; i < NUM_SONIDOS; i++){
        fijarVolumenDe(i, valorDeslizador(barraVolumen(i), volumenDe(i)));
    }

    // La misma tecla que abre la pausa la cierra. Si ESC hiciera otra cosa aqui
    // -por ejemplo salir al menu- seria facil perder una partida sin querer.
    if(IsKeyPressed(KEY_ESCAPE))      return Pausa_continuar;
    if(botonClicado(botonCerrar()))   return Pausa_continuar;

    if(enfoqueActivado() && enfoque >= NUM_SONIDOS){
        return ACCIONES_PAUSA[enfoque - NUM_SONIDOS];
    }

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

    dibujarBoton(botonCerrar(), "X", false);

    const char* titulo = "PAUSA";
    int tamano = 40;
    int ancho  = MeasureText(titulo, tamano);

    DrawText(titulo,
             (int)(panel.x + (panel.width - ancho) / 2.0f),
             (int)(panel.y + 34.0f),
             tamano, COLOR_TITULO);

    // ---- Volumen: musica y efectos ----
    for(int i = 0; i < NUM_SONIDOS; i++){

        Rectangle barra = barraVolumen(i);

        DrawText(NOMBRES_SONIDO[i], (int)barra.x, (int)(barra.y - 28.0f), 20, COLOR_TEXTO);

        dibujarDeslizador(barra, volumenDe(i));

        if(enfoque == i) dibujarAnilloEnfoque(barra);

        // El porcentaje va a la derecha de la barra, en el hueco que se le dejo.
        dibujarDato(TextFormat("%d%%", (int)(volumenDe(i) * 100.0f + 0.5f)),
                    (int)(barra.x + barra.width + 22.0f),
                    (int)(barra.y - 4.0f),
                    20, COLOR_TENUE);
    }

    // ---- Botones ----
    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){
        // Ninguno va marcado como seleccionado: son acciones, no opciones entre
        // las que se escoge una y se queda encendida.
        dibujarBoton(botonPausa(i), ETIQUETAS_PAUSA[i], false);

        if(enfoque == NUM_SONIDOS + i) dibujarAnilloEnfoque(botonPausa(i));
    }

    dibujarTextoCentrado("Flechas y Enter     ESC para seguir jugando",
                         (int)(panel.y + panel.height - 32.0f), 18, COLOR_TENUE);
}
