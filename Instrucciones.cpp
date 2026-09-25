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
// ESCALADO
//***********************************************

struct Escala {
    float x; // para anchos, gaps horizontales, offsets en X
    float y; // para altos, offsets en Y
    float u; // uniforme: fuentes, radios de icono, grosor de borde
};

static inline float E(float valor, float factor)
{
    return valor * factor;
}

//***********************************************
// ICONOS (Temporales)
//***********************************************

// Los iconos no necesitan saber nada de escala: ya reciben su radio como
// parametro (siempre en base a escala.u, para no salir ovalados), y
// adentro solo usan fracciones de ese radio.

typedef void (*DibujarIconoFn)(Vector2 centro, float radio, Color color);

static void iconoToque(Vector2 c, float r, Color color)
{
    DrawCircleV(c, r, Fade(color, 0.18f));
    DrawCircleV(c, r * 0.42f, color);
}

static void iconoSegundaCarta(Vector2 c, float r, Color color)
{
    Rectangle atras  = rectangulo(c.x - r * 0.75f, c.y - r * 0.55f, r * 0.9f, r * 1.3f);
    Rectangle frente = rectangulo(c.x - r * 0.15f, c.y - r * 0.40f, r * 0.9f, r * 1.3f);

    DrawRectangleRounded(atras,  0.18f, 6, Fade(color, 0.40f));
    DrawRectangleRounded(frente, 0.18f, 6, color);
}

static void iconoCoinciden(Vector2 c, float r, Color color)
{
    Rectangle izq = rectangulo(c.x - r * 0.90f, c.y - r * 0.5f, r * 0.75f, r * 1.1f);
    Rectangle der = rectangulo(c.x + r * 0.15f, c.y - r * 0.5f, r * 0.75f, r * 1.1f);

    DrawRectangleRounded(izq, 0.2f, 6, color);
    DrawRectangleRounded(der, 0.2f, 6, color);

    DrawLineEx((Vector2){ c.x - r * 0.35f, c.y + r * 0.05f }, (Vector2){ c.x - r * 0.10f, c.y + r * 0.30f }, 4.0f, DARKGREEN);
    DrawLineEx((Vector2){ c.x - r * 0.10f, c.y + r * 0.30f }, (Vector2){ c.x + r * 0.40f, c.y - r * 0.25f }, 4.0f, DARKGREEN);
}

static void iconoReintentar(Vector2 c, float r, Color color)
{
    DrawRing(c, r * 0.50f, r * 0.68f, 30.0f, 300.0f, 24, color);

    Vector2 punta[3] = {
        { c.x + r * 0.68f, c.y - r * 0.35f },
        { c.x + r * 0.98f, c.y - r * 0.55f },
        { c.x + r * 0.78f, c.y - r * 0.05f }
    };
    DrawTriangle(punta[0], punta[1], punta[2], color);
}

static void iconoTrofeo(Vector2 c, float r, Color color)
{
    DrawRectangleRounded(rectangulo(c.x - r * 0.35f, c.y - r * 0.50f, r * 0.70f, r * 0.70f), 0.3f, 8, color);
    DrawRing((Vector2){ c.x - r * 0.50f, c.y - r * 0.25f }, r * 0.18f, r * 0.26f, -90.0f, 90.0f,  16, color);
    DrawRing((Vector2){ c.x + r * 0.50f, c.y - r * 0.25f }, r * 0.18f, r * 0.26f,  90.0f, 270.0f, 16, color);
    DrawRectangle((int)(c.x - r * 0.12f), (int)(c.y + r * 0.20f), (int)(r * 0.24f), (int)(r * 0.28f), color);
    DrawRectangleRounded(rectangulo(c.x - r * 0.32f, c.y + r * 0.42f, r * 0.64f, r * 0.14f), 0.3f, 6, color);
}

static void iconoPersona(Vector2 c, float r, Color color)
{
    DrawCircleV((Vector2){ c.x, c.y - r * 0.35f }, r * 0.28f, color);
    DrawRing((Vector2){ c.x, c.y + r * 0.55f }, r * 0.40f, r * 0.62f, 180.0f, 360.0f, 16, color);
}

static void iconoDosPersonas(Vector2 c, float r, Color color)
{
    iconoPersona((Vector2){ c.x - r * 0.42f, c.y }, r * 0.72f, color);
    iconoPersona((Vector2){ c.x + r * 0.42f, c.y }, r * 0.72f, color);
}

static void iconoEstrella(Vector2 c, float r, Color color)
{
    DrawPoly(c, 4, r * 0.85f, 0.0f,  color);
    DrawPoly(c, 4, r * 0.85f, 45.0f, color);
}

static void iconoFuego(Vector2 c, float r, Color color)
{
    Vector2 punta[3] = {
        { c.x,            c.y - r * 0.90f },
        { c.x - r * 0.55f, c.y + r * 0.60f },
        { c.x + r * 0.55f, c.y + r * 0.60f }
    };
    DrawTriangle(punta[2], punta[1], punta[0], color);
    DrawCircleV((Vector2){ c.x, c.y + r * 0.35f }, r * 0.30f, Fade(color, 0.7f));
}

static void iconoReloj(Vector2 c, float r, Color color)
{
    DrawRing(c, r * 0.72f, r * 0.85f, 0.0f, 360.0f, 32, color);
    DrawLineEx(c, (Vector2){ c.x,           c.y - r * 0.50f }, 3.0f, color);
    DrawLineEx(c, (Vector2){ c.x + r * 0.35f, c.y }, 3.0f, color);
}

//***********************************************
// COLORES LOCALES (Temporal)
//***********************************************

static const Color COLOR_MODO_SOLO = { 255, 138, 101, 255 }; // coral suave
static const Color COLOR_MODO_VS   = {  77, 182, 172, 255 }; // verde azulado

//***********************************************
// TEXTO Y DATOS
//***********************************************

struct Paso {
    const char* linea1;
    const char* linea2;
    DibujarIconoFn icono;
};

static const int NUM_PASOS = 4;
static const Paso PASOS[NUM_PASOS] = {
    { "Toca una",    "carta",             iconoToque },
    { "Toca otra",   "carta",             iconoSegundaCarta },
    { "Son el mismo","gato? Se quedan!",  iconoCoinciden },
    { "No? Se tapan","y otra vez",        iconoReintentar }
};

static const char* TEXTO_GANAR = "Ganas cuando encuentras todos los gatos!";

struct Badge {
    DibujarIconoFn icono;
    const char* titulo;
    const char* descripcion;
};

static const int NUM_BADGES = 3;
static const Badge BADGES[NUM_BADGES] = {
    { iconoEstrella, "Puntos", "Suman por pareja" },
    { iconoFuego,    "Racha",  "Seguidas dan mas" },
    { iconoReloj,    "Tiempo", "Solo va sumando" }
};

//***********************************************
// TAMANO DEL PANEL (Para el ajuste de prueba)
//***********************************************

// PANEL_DISENO_* es la regla contra la que estan escritos todos los
// numeros del archivo (108, 34, 20, tamanos de fuente...). No se toca al
// cambiar el tamano del panel: es solo la vara de medir.
static const float PANEL_DISENO_ANCHO = 760.0f;
static const float PANEL_DISENO_ALTO  = 470.0f;

// PANEL_OBJETIVO_* es el tamano que de verdad queremos ver en pantalla.
// Para cambiar la resolucion del panel (ej. a 960x600), esto es lo unico
// que hay que tocar: Escala.x/y/u en calcularEscala() hacen que icono,
// texto y tarjetas crezcan juntos para llenar el rectangulo nuevo.
static const float PANEL_OBJETIVO_ANCHO = 960.0f;
static const float PANEL_OBJETIVO_ALTO  = 600.0f;

static Rectangle panelInstrucciones()
{
    float anchoDisponible = GetScreenWidth()  * 0.92f;
    float altoDisponible  = GetScreenHeight() * 0.92f;

    // Cada eje se achica por su lado si la ventana no alcanza; ya no hace
    // falta conservar una proporcion fija porque Escala.x/y no deforman
    // texto ni iconos aunque el panel deje de ser 8:5.
    float ancho = (PANEL_OBJETIVO_ANCHO < anchoDisponible) ? PANEL_OBJETIVO_ANCHO : anchoDisponible;
    float alto  = (PANEL_OBJETIVO_ALTO  < altoDisponible)  ? PANEL_OBJETIVO_ALTO  : altoDisponible;

    return rectangulo((GetScreenWidth()  - ancho) / 2.0f,
                      (GetScreenHeight() - alto ) / 2.0f,
                      ancho, alto);
}

/**
 * \brief Calcula los tres factores de escala a partir del panel real.
 *        u es el menor de x/y para que texto e iconos nunca se deformen,
 *        aunque el panel ya no guarde la proporcion 760:470 original.
 */
static Escala calcularEscala(Rectangle panel)
{
    Escala e;
    e.x = panel.width  / PANEL_DISENO_ANCHO;
    e.y = panel.height / PANEL_DISENO_ALTO;
    e.u = (e.x < e.y) ? e.x : e.y;
    return e;
}

/**
 * \brief La tacha de cerrar, en la esquina del panel.
 * \return Su rectangulo en pantalla.
 */
static Rectangle botonCerrar()
{
    Rectangle panel  = panelInstrucciones();
    Escala    esc    = calcularEscala(panel);

    return rectangulo(panel.x + panel.width - E(60.0f, esc.x),
                      panel.y + E(18.0f, esc.y),
                      E(42.0f, esc.u), E(42.0f, esc.u)); // boton cuadrado: usa u, no x/y
}

static void dibujarTextoCentradoEn(const char* texto, float x, float ancho, int y, int tamano, Color color)
{
    int anchoTexto = MeasureText(texto, tamano);
    DrawText(texto, (int)(x + (ancho - anchoTexto) / 2.0f), y, tamano, color);
}

//***********************************************
// SECCIONES DEL PANEL
//***********************************************

static void dibujarPasos(Rectangle panel, float y, Escala esc)
{
    float contentX  = panel.x + E(40.0f, esc.x);
    float contentW  = panel.width - 2.0f * E(40.0f, esc.x);
    float gap       = E(12.0f, esc.x);
    float tarjetaW  = (contentW - 3.0f * gap) / (float)NUM_PASOS;
    float tarjetaH  = E(108.0f, esc.y);

    for(int i = 0; i < NUM_PASOS; i++){
        Rectangle tarjeta = rectangulo(contentX + i * (tarjetaW + gap), y, tarjetaW, tarjetaH);

        DrawRectangleRounded(tarjeta, 0.14f, 8, COLOR_PANEL);
        DrawRectangleRoundedLinesEx(tarjeta, 0.14f, 8, E(2.0f, esc.u), Fade(COLOR_SELECCION, 0.5f));

        Vector2 numCentro = { tarjeta.x + E(16.0f, esc.x), tarjeta.y - E(2.0f, esc.y) };
        DrawCircleV(numCentro, E(13.0f, esc.u), COLOR_SELECCION);
        char numTexto[2] = { (char)('1' + i), '\0' };
        int  numTam       = (int)E(16.0f, esc.u);
        int  numAncho     = MeasureText(numTexto, numTam);
        DrawText(numTexto, (int)(numCentro.x - numAncho / 2.0f), (int)(numCentro.y - numTam / 2.0f), numTam, WHITE);

        Vector2 iconoCentro = { tarjeta.x + tarjeta.width / 2.0f, tarjeta.y + E(34.0f, esc.y) };
        PASOS[i].icono(iconoCentro, E(20.0f, esc.u), COLOR_SELECCION);

        int tamTexto = (int)E(14.0f, esc.u);
        dibujarTextoCentradoEn(PASOS[i].linea1, tarjeta.x, tarjeta.width, (int)(tarjeta.y + E(62.0f, esc.y)), tamTexto, COLOR_TEXTO);
        dibujarTextoCentradoEn(PASOS[i].linea2, tarjeta.x, tarjeta.width, (int)(tarjeta.y + E(82.0f, esc.y)), tamTexto, COLOR_TEXTO);
    }
}

static void dibujarBannerGanar(Rectangle panel, float y, Escala esc)
{
    float contentX = panel.x + E(40.0f, esc.x);
    float contentW = panel.width - 2.0f * E(40.0f, esc.x);
    Rectangle banner = rectangulo(contentX, y, contentW, E(34.0f, esc.y));

    DrawRectangleRounded(banner, 0.4f, 8, Fade(GOLD, 0.35f));

    Vector2 iconoCentro = { banner.x + E(26.0f, esc.x), banner.y + banner.height / 2.0f };
    iconoTrofeo(iconoCentro, E(13.0f, esc.u), Fade(BLACK, 0.55f));

    int tamTexto   = (int)E(16.0f, esc.u);
    int anchoTexto = MeasureText(TEXTO_GANAR, tamTexto);
    float inicioTexto = banner.x + E(50.0f, esc.x);
    DrawText(TEXTO_GANAR, (int)(inicioTexto + (contentX + contentW - inicioTexto - anchoTexto) / 2.0f),
             (int)(banner.y + E(9.0f, esc.y)), tamTexto, Fade(BLACK, 0.75f));
}

static void dibujarModos(Rectangle panel, float y, Escala esc)
{
    float contentX = panel.x + E(40.0f, esc.x);
    float contentW = panel.width - 2.0f * E(40.0f, esc.x);
    float gap      = E(16.0f, esc.x);
    float panelW   = (contentW - gap) / 2.0f;
    float panelH   = E(118.0f, esc.y);

    // --- Solitario ---
    Rectangle solo = rectangulo(contentX, y, panelW, panelH);
    DrawRectangleRounded(solo, 0.1f, 8, Fade(COLOR_MODO_SOLO, 0.25f));

    iconoPersona((Vector2){ solo.x + E(30.0f, esc.x), solo.y + E(30.0f, esc.y) }, E(20.0f, esc.u), COLOR_MODO_SOLO);
    DrawText("Tu solo", (int)(solo.x + E(56.0f, esc.x)), (int)(solo.y + E(18.0f, esc.y)), (int)E(20.0f, esc.u), COLOR_MODO_SOLO);

    iconoReloj((Vector2){ solo.x + E(24.0f, esc.x), solo.y + E(76.0f, esc.y) }, E(14.0f, esc.u), COLOR_TEXTO);
    DrawText("El reloj solo cuenta,", (int)(solo.x + E(46.0f, esc.x)), (int)(solo.y + E(60.0f, esc.y)), (int)E(15.0f, esc.u), COLOR_TEXTO);
    DrawText("no se detiene solo.",   (int)(solo.x + E(46.0f, esc.x)), (int)(solo.y + E(80.0f, esc.y)), (int)E(15.0f, esc.u), COLOR_TEXTO);
    DrawText("Es para ver que tan rapido fuiste.", (int)(solo.x + E(20.0f, esc.x)), (int)(solo.y + E(100.0f, esc.y)), (int)E(13.0f, esc.u), Fade(COLOR_TEXTO, 0.8f));

    // --- 1 vs 1 ---
    Rectangle vs = rectangulo(contentX + panelW + gap, y, panelW, panelH);
    DrawRectangleRounded(vs, 0.1f, 8, Fade(COLOR_MODO_VS, 0.25f));

    iconoDosPersonas((Vector2){ vs.x + E(34.0f, esc.x), vs.y + E(30.0f, esc.y) }, E(20.0f, esc.u), COLOR_MODO_VS);
    DrawText("1 vs 1", (int)(vs.x + E(62.0f, esc.x)), (int)(vs.y + E(18.0f, esc.y)), (int)E(20.0f, esc.u), COLOR_MODO_VS);

    DrawText("Aciertas y sigues.",        (int)(vs.x + E(20.0f, esc.x)), (int)(vs.y + E(58.0f, esc.y)),  (int)E(15.0f, esc.u), COLOR_TEXTO);
    DrawText("Fallas y le toca al otro.", (int)(vs.x + E(20.0f, esc.x)), (int)(vs.y + E(78.0f, esc.y)),  (int)E(15.0f, esc.u), COLOR_TEXTO);
    DrawText("Gana quien junte mas parejas.", (int)(vs.x + E(20.0f, esc.x)), (int)(vs.y + E(100.0f, esc.y)), (int)E(13.0f, esc.u), Fade(COLOR_TEXTO, 0.8f));
}

static void dibujarBadges(Rectangle panel, float y, Escala esc)
{
    float contentX = panel.x + E(40.0f, esc.x);
    float contentW = panel.width - 2.0f * E(40.0f, esc.x);
    float gap      = E(12.0f, esc.x);
    float badgeW   = (contentW - 2.0f * gap) / (float)NUM_BADGES;
    float badgeH   = E(48.0f, esc.y);

    for(int i = 0; i < NUM_BADGES; i++){
        Rectangle badge = rectangulo(contentX + i * (badgeW + gap), y, badgeW, badgeH);

        DrawRectangleRounded(badge, 0.25f, 8, COLOR_PANEL);
        DrawRectangleRoundedLinesEx(badge, 0.25f, 8, E(1.5f, esc.u), Fade(COLOR_SELECCION, 0.4f));

        Vector2 iconoCentro = { badge.x + E(26.0f, esc.x), badge.y + badge.height / 2.0f };
        BADGES[i].icono(iconoCentro, E(15.0f, esc.u), COLOR_SELECCION);

        DrawText(BADGES[i].titulo,      (int)(badge.x + E(48.0f, esc.x)), (int)(badge.y + E(8.0f, esc.y)),  (int)E(15.0f, esc.u), COLOR_TITULO);
        DrawText(BADGES[i].descripcion, (int)(badge.x + E(48.0f, esc.x)), (int)(badge.y + E(27.0f, esc.y)), (int)E(12.0f, esc.u), Fade(COLOR_TEXTO, 0.85f));
    }
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

void DibujarInstrucciones()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), COLOR_VELO);

    Rectangle panel = panelInstrucciones();
    Escala    esc   = calcularEscala(panel);

    DrawRectangleRounded(panel, 0.05f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.05f, 10, E(2.0f, esc.u), COLOR_SELECCION);

    DrawText("COMO SE JUEGA", (int)(panel.x + E(40.0f, esc.x)), (int)(panel.y + E(28.0f, esc.y)), (int)E(32.0f, esc.u), COLOR_TITULO);
    dibujarBoton(botonCerrar(), "X", false);

    dibujarPasos(panel,       panel.y + E(86.0f,  esc.y), esc);
    dibujarBannerGanar(panel, panel.y + E(200.0f, esc.y), esc);
    dibujarModos(panel,       panel.y + E(242.0f, esc.y), esc);
    dibujarBadges(panel,      panel.y + E(372.0f, esc.y), esc);

    dibujarTextoCentrado("ESC o la tacha para cerrar",
                         (int)(panel.y + panel.height - E(24.0f, esc.y)), (int)E(16.0f, esc.u), COLOR_TENUE);
}
