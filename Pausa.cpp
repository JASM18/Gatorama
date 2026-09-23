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

const int SIN_ENFOQUE = -1;

// Cual control esta resaltado. Mismo modelo que el cursor de cartas del tablero:
// empieza en nada, el raton lo mueve solo cuando de verdad se mueve (y lo apaga al
// salir de todo), y las flechas lo estrenan.
static int enfoque = SIN_ENFOQUE;

static const float PANEL_ANCHO = 440.0f;
static const float PANEL_ALTO  = 526.0f;

// El panel completo: marco, sol con "PAUSA", la tacha, los rotulos de Musica y
// Sonido, las cuatro placas de los botones y la linea de ayuda. Mide 440x526,
// igual que el panel, asi que un pixel de la imagen es un pixel del panel y todas
// las zonas de abajo se midieron sobre el PNG.
static const char* RUTA_PANEL = "recursos/pausa.png";

static Texture2D texturaPanel;
static bool      hayPanel = false;

void CargarTexturasPausa()
{
    hayPanel = cargarTexturaSiEsta(RUTA_PANEL, &texturaPanel);
}

void DescargarTexturasPausa()
{
    if(hayPanel) UnloadTexture(texturaPanel);

    hayPanel = false;
}

// Tinta cafe oscura, la del arte, para los porcentajes.
static const Color COLOR_TINTA = { 62, 46, 30, 255 };

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

    // A la derecha de los rotulos "Musica" (centro en y = 130) y "Sonido" (y = 189)
    // del arte. Deja lugar a la derecha para el porcentaje.
    return rectangulo(panel.x + 150.0f, panel.y + 122.0f + cual * 59.0f,
                      195.0f, 16.0f);
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

    // El cuadrito con la tacha, arriba a la izquierda del arte.
    return rectangulo(panel.x + 21.0f, panel.y + 28.0f, 47.0f, 39.0f);
}

/**
 * \brief D&oacute;nde queda uno de los botones del panel.
 * \param indice Bot&oacute;n, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonPausa(int indice)
{
    // Las placas del arte, medidas por su contorno. Estan dibujadas a mano: cada
    // una quedo de su tamano, por eso no salen de una formula.
    static const Rectangle PLACAS[NUM_BOTONES_PAUSA] = {
        { 47.0f, 240.0f, 326.0f, 48.0f },   // Continuar
        { 46.0f, 305.0f, 329.0f, 45.0f },   // Reiniciar partida
        { 47.0f, 360.0f, 330.0f, 46.0f },   // Como se juega
        { 44.0f, 415.0f, 330.0f, 43.0f }    // Regresar al menu
    };

    Rectangle panel = panelPausa();
    Rectangle placa = PLACAS[indice];

    return rectangulo(panel.x + placa.x, panel.y + placa.y, placa.width, placa.height);
}

/**
 * \brief Qu&eacute; control est&aacute; bajo el puntero, o SIN_ENFOQUE si ninguno.
 *
 * La tacha no cuenta: no est&aacute; en el recorrido del teclado.
 */
static int controlBajoElRaton()
{
    for(int i = 0; i < NUM_SONIDOS; i++){
        if(ratonEncima(barraVolumen(i))) return i;
    }

    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){
        if(ratonEncima(botonPausa(i))) return NUM_SONIDOS + i;
    }

    return SIN_ENFOQUE;
}

//***********************************************
// VENTANA DE PAUSA
//***********************************************

void PrepararPausa()
{
    enfoque = SIN_ENFOQUE;
}

AccionPausa ActualizarPausa()
{
    // El raton solo cuenta cuando de verdad se movio: si se leyera cada fotograma,
    // pisaria al instante lo que se acaba de elegir con las flechas.
    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        enfoque = controlBajoElRaton();
    }

    bool enBarra = (enfoque != SIN_ENFOQUE && enfoque < NUM_SONIDOS);

    if(enfoque == SIN_ENFOQUE){
        // La primera flecha estrena el enfoque: hacia abajo por la primera barra,
        // hacia arriba por el ultimo boton.
        if(IsKeyPressed(KEY_DOWN)) enfoque = 0;
        if(IsKeyPressed(KEY_UP))   enfoque = NUM_ENFOQUES_PAUSA - 1;
    } else {
        // Sobre una barra, izquierda y derecha ajustan en vez de cambiar de control.
        enfoque = moverEnfoque(enfoque, NUM_ENFOQUES_PAUSA, !enBarra);
    }

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

    if(hayPanel){
        // El arte trae el titulo, los rotulos, la tacha, las placas y la ayuda.
        DrawTexture(texturaPanel, (int)panel.x, (int)panel.y, WHITE);

        marcarPlaca(botonCerrar(), false, ratonEncima(botonCerrar()));
    } else {
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
    }

    // ---- Volumen: musica y efectos ----
    for(int i = 0; i < NUM_SONIDOS; i++){

        Rectangle barra = barraVolumen(i);

        if(!hayPanel){
            DrawText(NOMBRES_SONIDO[i], (int)panel.x + 40, (int)(barra.y - 2.0f), 20, COLOR_TEXTO);
        }

        // Con enfoque, un velo claro detras de la barra en vez de un anillo: sobre
        // el pergamino, un recuadro de color se ve pegado encima.
        if(enfoque == i){
            Rectangle velo = { barra.x - 10.0f, barra.y - 12.0f, barra.width + 70.0f, barra.height + 24.0f };

            if(hayPanel) DrawRectangleRounded(velo, 0.5f, 8, Fade(WHITE, 0.35f));
            else         dibujarAnilloEnfoque(barra);
        }

        dibujarDeslizador(barra, volumenDe(i));

        // El porcentaje va a la derecha de la barra, en el hueco que se le dejo.
        dibujarDato(TextFormat("%d%%", (int)(volumenDe(i) * 100.0f + 0.5f)),
                    (int)(barra.x + barra.width + 18.0f),
                    (int)(barra.y - 4.0f),
                    20, hayPanel ? COLOR_TINTA : COLOR_TENUE);
    }

    // ---- Botones ----
    for(int i = 0; i < NUM_BOTONES_PAUSA; i++){

        bool conEnfoque = (enfoque == NUM_SONIDOS + i);

        // Ninguno va marcado como elegido: son acciones, no opciones entre las que
        // se escoge una y se queda encendida.
        if(hayPanel){
            marcarPlaca(botonPausa(i), false, conEnfoque);
        } else {
            dibujarBoton(botonPausa(i), ETIQUETAS_PAUSA[i], false);

            if(conEnfoque) dibujarAnilloEnfoque(botonPausa(i));
        }
    }

    if(!hayPanel){
        dibujarTextoCentrado("Flechas y Enter     ESC para seguir jugando",
                             (int)(panel.y + panel.height - 32.0f), 18, COLOR_TENUE);
    }
}
