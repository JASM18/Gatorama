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

// El fondo de la pantalla -madera con las placas de "Juega otra vez" y "Volver al
// menu"- y el liston de "Se acabo", que va encima de todo con sus cortinas. Los dos
// miden 1280x720; el liston es transparente fuera del dibujo.
static const char* RUTA_FONDO  = "recursos/resultados.png";
static const char* RUTA_LISTON = "recursos/resultadosListon.png";

static Texture2D texturaFondo;
static Texture2D texturaListon;

static bool hayFondo  = false;
static bool hayListon = false;

// Las tarjetas dibujadas: la de quien gano -tambien la del solitario y la de un
// empate- y la de quien perdio. Traen el dibujo, los rotulos (Parejas, Puntos,
// Seguidas, Tiempo total, Nivel) y el letrero de VICTORIA o DERROTA; el codigo
// escribe los valores. No miden lo mismo entre si, asi que se dibujan las dos al
// mismo alto y cada valor se acomoda con las medidas de su propia imagen.
static const char* RUTA_VICTORIA = "recursos/resultadosVictoria.png";
static const char* RUTA_DERROTA  = "recursos/resultadosDerrota.png";

static Texture2D texturaVictoria;
static Texture2D texturaDerrota;

static bool hayVictoria = false;
static bool hayDerrota  = false;

/**
 * \brief D&oacute;nde va cada valor en una tarjeta, en p&iacute;xeles de SU imagen.
 *
 * Cada punto es donde termina el r&oacute;tulo impreso -el valor se escribe justo
 * despu&eacute;s- y la altura del centro de ese rengl&oacute;n. Para Parejas es al rev&eacute;s: el
 * n&uacute;mero va a la izquierda de la palabra, as&iacute; que es donde empieza.
 */
struct CamposTarjeta {
    Vector2 parejas;
    Vector2 puntos;
    Vector2 seguidas;
    Vector2 tiempo;
    Vector2 nivel;
};

// Medidas sobre resultadosVictoria.png (456x425) y resultadosDerrota.png (385x363).
static const CamposTarjeta CAMPOS_VICTORIA = {
    { 150.0f, 275.0f }, { 120.0f, 309.0f }, { 373.0f, 305.0f }, { 153.0f, 339.0f }, { 338.0f, 335.0f }
};
static const CamposTarjeta CAMPOS_DERROTA = {
    { 125.0f, 232.0f }, { 101.0f, 262.0f }, { 314.0f, 257.0f }, { 139.0f, 289.0f }, { 283.0f, 287.0f }
};

// La pantalla completa de solitario, 1280x720: madera, liston "Lo lograste", la
// tarjeta y las placas de los botones, todo ya en su lugar. En solitario se usa en
// vez del fondo, el liston de "Se acabo" y las tarjetas sueltas.
static const char* RUTA_PANTALLA_SOLITARIO = "recursos/resultadosSolitario.png";

static Texture2D texturaPantallaSolitario;
static bool      hayPantallaSolitario = false;

// Donde va cada valor en la tarjeta de solitario, en pixeles de pantalla: donde
// termina cada rotulo (o donde empieza "PAREJAS") y el centro de su renglon.
static const CamposTarjeta CAMPOS_SOLITARIO = {
    { 572.0f, 394.0f }, { 535.0f, 432.0f }, { 796.0f, 424.0f }, { 579.0f, 478.0f }, { 754.0f, 469.0f }
};

// La franja de abajo de la tarjeta, que trae impreso "EL JUGADOR": se tapa con su
// mismo color y ahi se escribe el nombre de quien jugo.
static const Rectangle FRANJA_NOMBRE_SOLITARIO = { 426.0f, 504.0f, 436.0f, 44.0f };
static const Color     COLOR_FRANJA_SOLITARIO  = { 196, 174, 133, 255 };

void CargarTexturasResultados()
{
    hayGanador   = cargarTexturaSiEsta(RUTA_GANADOR,   &texturaGanador);
    hayPerdedor  = cargarTexturaSiEsta(RUTA_PERDEDOR,  &texturaPerdedor);
    haySolitario = cargarTexturaSiEsta(RUTA_SOLITARIO, &texturaSolitario);
    hayFondo     = cargarTexturaSiEsta(RUTA_FONDO,     &texturaFondo);
    hayListon    = cargarTexturaSiEsta(RUTA_LISTON,    &texturaListon);
    hayVictoria  = cargarTexturaSiEsta(RUTA_VICTORIA,  &texturaVictoria);
    hayDerrota   = cargarTexturaSiEsta(RUTA_DERROTA,   &texturaDerrota);

    hayPantallaSolitario = cargarTexturaSiEsta(RUTA_PANTALLA_SOLITARIO, &texturaPantallaSolitario);

    // Se dibujan un poco mas chicas que el archivo: con mipmaps no se ven dentadas.
    if(hayVictoria){
        GenTextureMipmaps(&texturaVictoria);
        SetTextureFilter(texturaVictoria, TEXTURE_FILTER_TRILINEAR);
    }

    if(hayDerrota){
        GenTextureMipmaps(&texturaDerrota);
        SetTextureFilter(texturaDerrota, TEXTURE_FILTER_TRILINEAR);
    }
}

void DescargarTexturasResultados()
{
    if(hayGanador)   UnloadTexture(texturaGanador);
    if(hayPerdedor)  UnloadTexture(texturaPerdedor);
    if(haySolitario) UnloadTexture(texturaSolitario);
    if(hayFondo)     UnloadTexture(texturaFondo);
    if(hayListon)    UnloadTexture(texturaListon);

    if(hayVictoria)  UnloadTexture(texturaVictoria);
    if(hayDerrota)   UnloadTexture(texturaDerrota);
    if(hayPantallaSolitario) UnloadTexture(texturaPantallaSolitario);

    hayPantallaSolitario = false;

    hayFondo = hayListon = hayVictoria = hayDerrota = false;

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
//
// Con el liston de "Se acabo" bajan un poco, para no quedar pegadas a su curva.
static float tarjetaY()
{
    return hayListon ? 160.0f : 140.0f;
}

// En 1 vs 1 la tarjeta es mas alta: lleva un renglon mas, el tiempo de cada quien.
static float altoTarjeta()
{
    return (numJugadores > 1) ? 316.0f : 260.0f;
}

static Rectangle tarjetaJugador(int jugador)
{
    if(numJugadores == 1){
        return rectangulo((GetScreenWidth() - 440.0f) / 2.0f, tarjetaY(), 440.0f, altoTarjeta());
    }

    // Dos de 370 con 40 de hueco: 780 en total, centrados.
    float inicio = (GetScreenWidth() - 780.0f) / 2.0f;

    return rectangulo(inicio + jugador * (370.0f + 40.0f), tarjetaY(), 370.0f, altoTarjeta());
}

static Rectangle botonOtraVez()
{
    // Con el arte, las placas medidas sobre resultados.png por su contorno.
    if(hayFondo) return rectangulo(390.0f, 604.0f, 236.0f, 60.0f);

    return rectangulo(GetScreenWidth() / 2.0f - 250.0f, GetScreenHeight() - 150.0f,
                      230.0f, 56.0f);
}

static Rectangle botonAlMenu()
{
    if(hayFondo) return rectangulo(658.0f, 604.0f, 237.0f, 60.0f);

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
        if(teclaArriba()   || teclaAbajo() ||
           teclaIzquierda() || teclaDerecha()){
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

/** \brief Si hay arte para las tarjetas (las dos versiones). */
static bool tarjetasConArte()
{
    return hayVictoria && hayDerrota;
}

/**
 * \brief Si a un jugador le toca la tarjeta de victoria: el ganador, el solitario y
 *        los dos en un empate.
 */
static bool leTocaVictoria(int jugador)
{
    return numJugadores == 1 || ganador < 0 || jugador == ganador;
}

/**
 * \brief El alto de la tarjeta de un jugador en 1 vs 1.
 *
 * Como en el arte conceptual: la de quien gan&oacute; va grande y la de quien perdi&oacute;,
 * chica. Son los tama&ntilde;os de las dos im&aacute;genes tal como se dibujaron. En un empate
 * ninguna destaca, y van las dos chicas.
 */
static float altoTarjetaConArte(int jugador)
{
    const float GRANDE = 425.0f;
    const float CHICA  = 363.0f;

    if(ganador < 0) return CHICA;

    return (jugador == ganador) ? GRANDE : CHICA;
}

/**
 * \brief D&oacute;nde va la tarjeta dibujada de un jugador en 1 vs 1.
 *
 * El jugador 1 a la izquierda y el 2 a la derecha, como en sus marcadores de la
 * partida, sin importar qui&eacute;n gan&oacute;. Las dos juntas, con 15 de hueco, centradas a
 * lo ancho y a la misma altura de centro: as&iacute; la chica queda a media altura de
 * la grande, como en el arte conceptual.
 */
static Rectangle zonaTarjetaConArte(int jugador)
{
    const float HUECO    = 15.0f;
    const float CENTRO_X = 641.0f;
    const float CENTRO_Y = 314.0f;

    float alto[2];
    float ancho[2];

    for(int j = 0; j < 2; j++){
        const Texture2D& tex = leTocaVictoria(j) ? texturaVictoria : texturaDerrota;

        alto[j]  = altoTarjetaConArte(j);
        ancho[j] = alto[j] * tex.width / (float)tex.height;
    }

    float x0 = CENTRO_X - (ancho[0] + HUECO + ancho[1]) / 2.0f;
    float x  = (jugador == 0) ? x0 : x0 + ancho[0] + HUECO;

    return rectangulo(x, CENTRO_Y - alto[jugador] / 2.0f, ancho[jugador], alto[jugador]);
}

/**
 * \brief Escribe un valor en una tarjeta dibujada, a partir de un punto de su imagen.
 *
 * \param tarjeta  D&oacute;nde est&aacute; la tarjeta en pantalla.
 * \param escala   Pantalla entre imagen.
 * \param punto    El punto del r&oacute;tulo en p&iacute;xeles de la imagen.
 * \param texto    El valor.
 * \param tamano   Alto de la letra en pantalla.
 * \param alFinal  Si el texto termina en el punto (alineado a la derecha) en vez de
 *                 empezar ah&iacute;.
 */
static void escribirEnTarjeta(Rectangle tarjeta, float escala, Vector2 punto,
                              const char* texto, int tamano, bool alFinal)
{
    const Color TINTA = { 50, 36, 24, 255 };

    float x = tarjeta.x + punto.x * escala;
    float y = tarjeta.y + punto.y * escala - tamano / 2.0f;

    if(alFinal) x -= anchoDato(texto, tamano) + 8.0f;
    else        x += 6.0f;

    dibujarDato(texto, (int)x, (int)y, tamano, TINTA);
}

static void escribirValoresTarjeta(Rectangle zona, float escala, const CamposTarjeta& campos,
                                   int jugador, float letra);

/**
 * \brief La tarjeta dibujada de un jugador, con su nombre debajo y sus valores.
 */
static void dibujarTarjetaConArte(int jugador)
{
    bool              victoria = leTocaVictoria(jugador);
    const Texture2D&  tex      = victoria ? texturaVictoria : texturaDerrota;
    const CamposTarjeta& campos = victoria ? CAMPOS_VICTORIA : CAMPOS_DERROTA;

    Rectangle zona   = zonaTarjetaConArte(jugador);
    float     escala = zona.height / tex.height;

    Rectangle origen  = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Vector2   desfase = { 0.0f, 0.0f };

    DrawTexturePro(tex, origen, zona, desfase, 0.0f, WHITE);

    // El nombre debajo de la tarjeta, sobre la madera: la tarjeta no tiene donde, y
    // arriba lo taparia el liston.
    const int TAM_NOMBRE = 30;
    int       anchoNombre = anchoDato(nombres[jugador], TAM_NOMBRE);

    dibujarDatoSobreFondo(nombres[jugador],
                          (int)(zona.x + (zona.width - anchoNombre) / 2.0f),
                          (int)(zona.y + zona.height) + 8, TAM_NOMBRE, COLOR_FONDO_TEXTO);

    escribirValoresTarjeta(zona, escala, campos, jugador, 1.0f);
}

/**
 * \brief Los cinco valores de una tarjeta: parejas, puntos, seguidas, tiempo y nivel.
 *
 * \param zona    D&oacute;nde est&aacute; la tarjeta (o la pantalla, si los campos son de pantalla).
 * \param escala  Pantalla entre imagen.
 * \param campos  D&oacute;nde va cada valor, en p&iacute;xeles de la imagen.
 * \param jugador De qui&eacute;n son los valores.
 * \param letra   Cu&aacute;nto agrandar la letra: 1 para las tarjetas de 1 vs 1; m&aacute;s en
 *                la de solitario, cuyos r&oacute;tulos son m&aacute;s grandes.
 */
static void escribirValoresTarjeta(Rectangle zona, float escala, const CamposTarjeta& campos,
                                   int jugador, float letra)
{
    const InfoDificultad& nivel = DIFICULTADES[dificultad];

    // El numero de parejas, grande y a la izquierda de la palabra PAREJAS.
    int grande = (int)(36 * letra);
    int chica  = (int)(20 * letra);

    escribirEnTarjeta(zona, escala, campos.parejas, TextFormat("%d", pares[jugador]), grande, true);

    escribirEnTarjeta(zona, escala, campos.puntos,   TextFormat("%d", puntos[jugador]), chica, false);
    escribirEnTarjeta(zona, escala, campos.seguidas, TextFormat("%d", rachas[jugador]), chica, false);

    // "Tiempo total": el de la partida completa, igual para los dos. Como reloj
    // -"1:15"- y no "1 min 15 seg": en la tarjeta no cabe antes de "Nivel".
    int total = (int)tiempo;

    escribirEnTarjeta(zona, escala, campos.tiempo, TextFormat("%d:%02d", total / 60, total % 60), chica, false);
    escribirEnTarjeta(zona, escala, campos.nivel,  nivel.nombre, chica, false);
}

/**
 * \brief La pantalla de solitario con su arte: la imagen completa y, encima, los
 *        valores y el nombre del jugador en la franja de abajo de la tarjeta.
 */
static void dibujarSolitarioConArte()
{
    DrawTexture(texturaPantallaSolitario, 0, 0, WHITE);

    Rectangle pantalla = rectangulo(0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight());

    escribirValoresTarjeta(pantalla, 1.0f, CAMPOS_SOLITARIO, 0, 1.35f);

    // El nombre va en la franja que dice "EL JUGADOR": se tapa con su color y se
    // escribe encima, centrado.
    const int TAMANO = 34;
    Rectangle franja = FRANJA_NOMBRE_SOLITARIO;
    int       ancho  = anchoDato(nombres[0], TAMANO);

    DrawRectangleRec(franja, COLOR_FRANJA_SOLITARIO);
    dibujarDato(nombres[0], (int)(franja.x + (franja.width - ancho) / 2.0f),
                (int)(franja.y + (franja.height - TAMANO) / 2.0f), TAMANO, Color{ 50, 36, 24, 255 });
}

void DibujarResultados()
{
    // En solitario, con su arte, la pantalla completa sale de una sola imagen.
    if(numJugadores == 1 && hayPantallaSolitario){
        dibujarSolitarioConArte();

        marcarPlaca(botonOtraVez(), false, enfoque == 0);
        marcarPlaca(botonAlMenu(),  false, enfoque == 1);
        return;
    }

    if(hayFondo) DrawTexture(texturaFondo, 0, 0, WHITE);

    // ---- Titulo ----
    // Con el liston, el titulo es su "Se acabo" -se dibuja hasta el final, encima
    // de las tarjetas-, y quien gano ya lo dice la etiqueta GANADOR de su tarjeta.
    if(!hayListon){
        if(numJugadores == 1){
            dibujarTextoCentradoSobreFondo("LO LOGRASTE", 60, 44, COLOR_FONDO_TITULO);
        }
        else if(ganador >= 0){
            dibujarDatoCentradoSobreFondo(TextFormat("GANO %s", nombres[ganador]), 60, 44, COLOR_FONDO_RESALTE);
        }
        else {
            dibujarTextoCentradoSobreFondo("EMPATE", 60, 44, COLOR_FONDO_TITULO);
        }
    }

    // ---- Tarjetas ----
    // Con el arte, las tarjetas ya traen el tiempo total y el nivel, y su letrero
    // dice quien gano: el pie de abajo sobra.
    if(tarjetasConArte()){
        for(int j = 0; j < numJugadores; j++){
            dibujarTarjetaConArte(j);
        }
    } else {
        for(int j = 0; j < numJugadores; j++){
            dibujarTarjetaJugador(j);
        }
    }

    // ---- Pie, sobre la madera: la regla (solo en 1 vs 1) y lo que tardo la partida ----
    const InfoDificultad& nivel = DIFICULTADES[dificultad];

    if(!tarjetasConArte()){

        // Debajo de la tarjeta, que en 1 vs 1 es mas alta.
        int yPie = (int)(tarjetaY() + altoTarjeta()) + 24;

        if(numJugadores > 1){
            dibujarDatoCentradoSobreFondo("Gana quien encuentra mas parejas", yPie, 26, COLOR_FONDO_TENUE);
            yPie += 40;
        }

        // El tiempo es el de toda la partida, en completar el tablero: el mismo dato en
        // solitario y en 1 vs 1.
        dibujarDatoCentradoSobreFondo(TextFormat("Tiempo total  %s      Nivel  %s",
                                                 comoDuracion(tiempo), nivel.nombre),
                                      yPie, 26, COLOR_FONDO_TEXTO);
    }

    // Ninguno va pintado como elegido: son acciones, no opciones.
    if(hayFondo){
        marcarPlaca(botonOtraVez(), false, enfoque == 0);
        marcarPlaca(botonAlMenu(),  false, enfoque == 1);
    } else {
        dibujarBoton(botonOtraVez(), "Jugar otra vez", false);
        dibujarBoton(botonAlMenu(),  "Volver al menu", false);

        if(enfoque == 0) dibujarAnilloEnfoque(botonOtraVez());
        if(enfoque == 1) dibujarAnilloEnfoque(botonAlMenu());
    }

    // El liston va al final: sus cortinas caen por encima de las orillas de las
    // tarjetas, como un telon.
    if(hayListon) DrawTexture(texturaListon, 0, 0, WHITE);
}
