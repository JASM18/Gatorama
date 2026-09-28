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

// Dos botones: 0 es "Jugar otra vez" y 1 es "Volver al menu". Mismo modelo que las
// demas pantallas: arranca sin nada resaltado, las flechas lo estrenan y el raton
// lo mueve solo cuando de verdad se mueve. La primera flecha cae en "Jugar otra
// vez", que es lo que casi siempre quiere el que acaba de jugar.
const int NUM_CONTROLES = 2;
const int SIN_ENFOQUE   = -1;

static int enfoque = SIN_ENFOQUE;

static int        numJugadores = 1;
static int        ganador      = 0;
static int        intentos     = 0;
static float      tiempo       = 0.0f;
static Dificultad dificultad   = Dificultad_facil;

void PrepararResultados(const ConfigPartida& config, const Partida& partida)
{
    enfoque = SIN_ENFOQUE;

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
// ARTE
//***********************************************

// Una imagen por tarjeta, en el hueco que queda a la derecha del numero de parejas.
// Son tres: la del que gano en 1 vs 1 (tambien la de un empate), la del que perdio y
// la de solitario. Si falta la de solitario se usa la del ganador; si falta
// cualquiera de las otras, el hueco se queda vacio y todo lo demas funciona.
static const char* RUTA_GANADOR   = "recursos/resultadoGanador.png";
static const char* RUTA_PERDEDOR  = "recursos/resultadoPerdedor.png";
static const char* RUTA_SOLITARIO = "recursos/resultadoSolitario.png";

static Texture2D texturaGanador;
static Texture2D texturaPerdedor;
static Texture2D texturaSolitario;

static bool hayGanador   = false;
static bool hayPerdedor  = false;
static bool haySolitario = false;

void CargarTexturasResultados()
{
    hayGanador   = cargarTexturaSiEsta(RUTA_GANADOR,   &texturaGanador);
    hayPerdedor  = cargarTexturaSiEsta(RUTA_PERDEDOR,  &texturaPerdedor);
    haySolitario = cargarTexturaSiEsta(RUTA_SOLITARIO, &texturaSolitario);
}

void DescargarTexturasResultados()
{
    if(hayGanador)   UnloadTexture(texturaGanador);
    if(hayPerdedor)  UnloadTexture(texturaPerdedor);
    if(haySolitario) UnloadTexture(texturaSolitario);

    hayGanador = hayPerdedor = haySolitario = false;
}

/**
 * \brief Dibuja una textura centrada dentro de una zona, sin deformarla.
 *
 * Si la imagen es m&aacute;s grande que la zona se reduce conservando su proporci&oacute;n;
 * si es m&aacute;s chica se deja de su tama&ntilde;o para que no se vea borrosa.
 *
 * \param textura Imagen a dibujar.
 * \param zona    Rect&aacute;ngulo donde debe caber.
 */
static void dibujarTexturaAjustada(const Texture2D& textura, Rectangle zona)
{
    float escala = zona.width / (float)textura.width;
    float alto   = zona.height / (float)textura.height;

    if(alto < escala) escala = alto;
    if(escala > 1.0f) escala = 1.0f;

    float w = textura.width  * escala;
    float h = textura.height * escala;

    Rectangle origen  = { 0.0f, 0.0f, (float)textura.width, (float)textura.height };
    Rectangle destino = { zona.x + (zona.width - w) / 2.0f,
                          zona.y + (zona.height - h) / 2.0f, w, h };

    DrawTexturePro(textura, origen, destino, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
}

//***********************************************
// ACOMODO
//***********************************************

// Cada jugador tiene su tarjeta. En 1 vs 1 van una junto a la otra, en el mismo lado
// que sus marcadores de la partida (jugador 1 a la izquierda, jugador 2 a la
// derecha). En solitario hay una sola, centrada y un poco mas ancha.
static const float TARJETA_Y = 140.0f;

// En 1 vs 1 la tarjeta es mas alta: lleva un renglon mas, el tiempo de cada quien.
static float altoTarjeta()
{
    return (numJugadores > 1) ? 316.0f : 260.0f;
}

static Rectangle tarjetaJugador(int jugador)
{
    if(numJugadores == 1){
        return rectangulo((GetScreenWidth() - 440.0f) / 2.0f, TARJETA_Y, 440.0f, altoTarjeta());
    }

    // Dos de 370 con 40 de hueco: 780 en total, centrados.
    float inicio = (GetScreenWidth() - 780.0f) / 2.0f;

    return rectangulo(inicio + jugador * (370.0f + 40.0f), TARJETA_Y, 370.0f, altoTarjeta());
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
    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        enfoque = SIN_ENFOQUE;

        if(ratonEncima(botonOtraVez())) enfoque = 0;
        if(ratonEncima(botonAlMenu()))  enfoque = 1;
    }

    if(enfoque == SIN_ENFOQUE){
        if(IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_DOWN) ||
           IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)){
            enfoque = 0;
        }
    } else {
        enfoque = moverEnfoque(enfoque, NUM_CONTROLES, true);
    }

    if(botonClicado(botonOtraVez())) return Escena_configuracion;

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonAlMenu())) return Escena_menu;

    if(enfoqueActivado() && enfoque != SIN_ENFOQUE){
        return (enfoque == 0) ? Escena_configuracion : Escena_menu;
    }

    return Escena_resultados;
}

/**
 * \brief Escribe una duraci&oacute;n como se la dir&iacute;a uno a un ni&ntilde;o: "14 seg" o "1 min 05 seg".
 *
 * El reloj "00:14" es dif&iacute;cil de leer a los seis a&ntilde;os; con la palabra al lado se
 * entiende sin explicar.
 *
 * \param segundos Duraci&oacute;n.
 * \return Cadena lista para dibujar, v&aacute;lida hasta la siguiente llamada.
 */
static const char* comoDuracion(float segundos)
{
    int total = (int)segundos;

    if(total < 60) return TextFormat("%d seg", total);

    return TextFormat("%d min %02d seg", total / 60, total % 60);
}

/**
 * \brief Dibuja la tarjeta de un jugador: nombre, parejas en grande y dos datos chicos.
 *
 * Todo lo que decide qui&eacute;n gan&oacute; (las parejas) va grande y arriba; lo dem&aacute;s va
 * abajo y chico. As&iacute; se compara de un vistazo sin leer.
 *
 * \param jugador 0 o 1.
 */
static void dibujarTarjetaJugador(int jugador)
{
    Rectangle rec     = tarjetaJugador(jugador);
    bool      esElQue = (numJugadores > 1 && jugador == ganador);

    // En solitario la unica tarjeta va resaltada: quien llega aqui gano.
    bool resaltada = esElQue || numJugadores == 1;

    int x = (int)rec.x + 24;
    int y = (int)rec.y;

    if(resaltada){
        DrawRectangleRounded(rec, 0.08f, 10, COLOR_BOTON);
        DrawRectangleRoundedLinesEx(rec, 0.08f, 10, 4.0f, COLOR_BOTON_ACTIVO);
    } else {
        DrawRectangleRounded(rec, 0.08f, 10, COLOR_PANEL);
        DrawRectangleRoundedLinesEx(rec, 0.08f, 10, 2.0f, COLOR_TENUE);
    }

    // La etiqueta se lee de lejos y no depende de comparar numeros. Sin acentos ni
    // signos de apertura, igual que el resto de los textos del juego.
    if(esElQue){
        Rectangle etiqueta = rectangulo(rec.x + 24.0f, rec.y - 18.0f, 150.0f, 36.0f);

        DrawRectangleRounded(etiqueta, 0.5f, 8, COLOR_BOTON_ACTIVO);
        dibujarDato("GANADOR", (int)etiqueta.x + 18, (int)etiqueta.y + 7, 22, WHITE);
    }

    dibujarDato(nombres[jugador], x, y + 26, 30, COLOR_TEXTO);

    // El numero grande y, debajo, la palabra. Debajo y no al lado: al lado dependia
    // de cuantas cifras tuviera el numero y con una sola quedaba un hueco enorme.
    dibujarDato(TextFormat("%d", pares[jugador]), x, y + 58, 72, COLOR_TEXTO);
    dibujarDato((pares[jugador] == 1) ? "pareja" : "parejas", x, y + 134, 24, COLOR_TEXTO);

    // La imagen ocupa el hueco a la derecha del numero, entre el nombre y la linea.
    // Alegre para quien gano (y en solitario, y si hubiera empate); triste o de
    // animo para quien perdio.
    bool alegre = (numJugadores == 1 || ganador < 0 || jugador == ganador);

    // En 1 vs 1 el hueco es bajo (104) para dejar sitio al nombre y a la etiqueta de
    // GANADOR. En solitario no hay etiqueta y la tarjeta es mas ancha, asi que el
    // hueco sube y crece a 140: una imagen cuadrada sale de 140x140.
    Rectangle huecoArte = (numJugadores == 1)
        ? rectangulo(rec.x + 140.0f, rec.y + 22.0f, rec.width - 164.0f, 140.0f)
        : rectangulo(rec.x + 140.0f, rec.y + 58.0f, rec.width - 164.0f, 104.0f);

    // Cual imagen toca: en solitario la suya (o la del ganador si no existe); en
    // 1 vs 1, la del ganador o la del perdedor.
    const Texture2D* arte = 0;

    if(numJugadores == 1){
        if(haySolitario)     arte = &texturaSolitario;
        else if(hayGanador)  arte = &texturaGanador;
    }
    else if(alegre){
        if(hayGanador)       arte = &texturaGanador;
    }
    else if(hayPerdedor){
        arte = &texturaPerdedor;
    }

    if(arte != 0) dibujarTexturaAjustada(*arte, huecoArte);

    DrawLine(x, y + 170, (int)(rec.x + rec.width) - 24, y + 170, COLOR_TENUE);

    // Dos datos chicos, iguales en solitario y en 1 vs 1.
    int xSegundo = x + 110;

    dibujarDato("PUNTOS", x, y + 184, 16, COLOR_TENUE);
    dibujarDato(TextFormat("%d", puntos[jugador]), x, y + 206, 28, COLOR_TEXTO);

    dibujarDato("SEGUIDAS", xSegundo, y + 184, 16, COLOR_TENUE);
    dibujarDato(TextFormat("%d", rachas[jugador]), xSegundo, y + 206, 28, COLOR_TEXTO);

    // Solo en 1 vs 1: lo que tardo cada quien en sus turnos. En solitario seria el
    // mismo numero que el tiempo total de abajo, asi que no se repite. Va en su
    // propio renglon porque "2 min 30 seg" no cabe en una tercera columna.
    if(numJugadores > 1){
        dibujarDato("SU TIEMPO", x, y + 244, 16, COLOR_TENUE);
        dibujarDato(comoDuracion(tiempos[jugador]), x, y + 266, 28, COLOR_TEXTO);
    }
}

void DibujarResultados()
{
    // ---- Titulo ----
    if(numJugadores == 1){
        dibujarTextoCentradoSobreFondo("LO LOGRASTE", 60, 44, COLOR_FONDO_TITULO);
    }
    else if(ganador >= 0){
        dibujarDatoCentradoSobreFondo(TextFormat("GANO %s", nombres[ganador]), 60, 44, COLOR_FONDO_RESALTE);
    }
    else {
        dibujarTextoCentradoSobreFondo("EMPATE", 60, 44, COLOR_FONDO_TITULO);
    }

    // ---- Tarjetas ----
    for(int j = 0; j < numJugadores; j++){
        dibujarTarjetaJugador(j);
    }

    // ---- Pie, sobre la madera: la regla (solo en 1 vs 1) y lo que tardo la partida ----
    const InfoDificultad& nivel = DIFICULTADES[dificultad];

    // Debajo de la tarjeta, que en 1 vs 1 es mas alta.
    int yPie = (int)(TARJETA_Y + altoTarjeta()) + 24;

    if(numJugadores > 1){
        dibujarDatoCentradoSobreFondo("Gana quien encuentra mas parejas", yPie, 26, COLOR_FONDO_TENUE);
        yPie += 40;
    }

    // El tiempo es el de toda la partida, en completar el tablero: el mismo dato en
    // solitario y en 1 vs 1.
    dibujarDatoCentradoSobreFondo(TextFormat("Tiempo total  %s      Nivel  %s",
                                             comoDuracion(tiempo), nivel.nombre),
                                  yPie, 26, COLOR_FONDO_TEXTO);

    // Ninguno va pintado como elegido: son acciones, no opciones.
    dibujarBoton(botonOtraVez(), "Jugar otra vez", false);
    dibujarBoton(botonAlMenu(),  "Volver al menu", false);

    if(enfoque == 0) dibujarAnilloEnfoque(botonOtraVez());
    if(enfoque == 1) dibujarAnilloEnfoque(botonAlMenu());
}
