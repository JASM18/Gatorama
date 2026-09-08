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

// Los controles que recorre el teclado, en el orden en que se recorren.
const int CTRL_SOLITARIO = 0;
const int CTRL_MULTI     = 1;
const int CTRL_FACIL     = 2;   // los tres niveles ocupan el 2, el 3 y el 4
const int CTRL_NOMBRE1   = 5;
const int CTRL_NOMBRE2   = 6;
const int CTRL_INICIAR   = 7;
const int NUM_CONTROLES  = 8;

// Cual control tiene el enfoque. Tambien decide en que campo se escribe: si el
// enfoque esta sobre un nombre, las letras van ahi. Un solo dato para las dos
// cosas es lo que evita que el cursor parpadee en un campo y se escriba en otro.
//
// Arranca en Iniciar para que Enter siga arrancando la partida de inmediato, como
// antes de que existieran las flechas.
static int enfoque = CTRL_INICIAR;

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
// ARTE DE LA PANTALLA
//***********************************************

// El panel del resumen. Mide 560x400, lo mismo que panelResumen().
static const char* RUTA_RESUMEN = "recursos/configResumen.png";

static Texture2D texturaResumen;
static bool      hayResumen = false;

void CargarTexturasConfiguracion()
{
    hayResumen = cargarTexturaSiEsta(RUTA_RESUMEN, &texturaResumen);
}

void DescargarTexturasConfiguracion()
{
    if(hayResumen){
        UnloadTexture(texturaResumen);
        hayResumen = false;
    }
}

//***********************************************
// ESCRITURA DE NOMBRES
//***********************************************

// Cuanto hay que tener apretado el retroceso antes de que empiece a borrar solo, y
// cada cuanto quita una letra a partir de ahi. La espera larga es para que un toque
// rapido quite una sola letra: sin ella, corregir una vocal borraria medio nombre.
// Ya que arranco, va rapido, porque a esas alturas lo que se quiere es vaciar el
// campo. 0.05 vacia un nombre de 12 letras en poco mas de medio segundo.
static const float ESPERA_BORRADO = 0.4f;
static const float RITMO_BORRADO  = 0.05f;

// Cuanto lleva apretado el retroceso, y cuanto falta para la siguiente letra. Un
// solo par de contadores para los dos campos: como solo se escribe en el que tiene
// el enfoque, nunca hay dos borrados corriendo al mismo tiempo.
static float tiempoRetroceso = 0.0f;
static float proximoBorrado  = 0.0f;

/**
 * \brief Quita la ultima letra de un campo, si queda alguna.
 *
 * \param destino Cadena terminada en cero.
 */
static void borrarUltima(char* destino)
{
    int largo = (int)strlen(destino);

    if(largo <= 0) return;

    // No se borra un byte, se borra una LETRA. En UTF-8 una acentuada ocupa dos
    // bytes, y el segundo siempre empieza con los bits 10: quitar solo ese dejaria
    // media letra y el nombre se veria como un simbolo roto. Se retrocede hasta el
    // byte donde de verdad empieza el caracter.
    int i = largo - 1;

    while(i > 0 && ((unsigned char)destino[i] & 0xC0) == 0x80) i--;

    destino[i] = '\0';
}

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

        // Se le pregunta a la fuente en vez de comparar contra un rango escrito
        // aqui. Asi la lista de letras validas esta en un solo lugar: si manana se
        // hornea un signo nuevo, este campo lo acepta sin tocar esta linea.
        if(fuenteTieneCodigo(letra)){

            // Una letra acentuada NO ocupa un byte. Se guarda en UTF-8, que gasta
            // uno para el ASCII y dos para nuestros acentos, asi que el limite se
            // mide en bytes y no en letras: "Andres" cabe en seis y "Andres" con
            // acento en siete. CodepointToUTF8 hace la conversion y de paso dice
            // cuantos bytes salieron.
            int         cuantos = 0;
            const char* enUTF8  = CodepointToUTF8(letra, &cuantos);

            // Se revisa ANTES de escribir: ese es el limite que evita pasarse del
            // arreglo. Antes bastaba con mirar el largo; ahora una sola letra
            // puede no caber aunque todavia quede un hueco de un byte.
            if(largo + cuantos <= LARGO_NOMBRE){

                for(int i = 0; i < cuantos; i++) destino[largo + i] = enUTF8[i];

                destino[largo + cuantos] = '\0';
            }
        }

        letra = GetCharPressed();
    }

    // El borrado se pregunta aparte: el retroceso no es un caracter que se
    // escriba, es una tecla que quita.
    //
    // Se lleva a mano y no con IsKeyPressedRepeat porque esa usa el ritmo que tenga
    // configurado Windows, que en una computadora prestada el dia del rally puede
    // ser cualquiera. Aqui la espera y el ritmo son los mismos en toda maquina.
    if(IsKeyPressed(KEY_BACKSPACE)){

        // El primer toque siempre quita una letra, sin esperar nada.
        borrarUltima(destino);

        tiempoRetroceso = 0.0f;
        proximoBorrado  = 0.0f;

    } else if(IsKeyDown(KEY_BACKSPACE)){

        tiempoRetroceso += GetFrameTime();

        if(tiempoRetroceso >= ESPERA_BORRADO){

            proximoBorrado -= GetFrameTime();

            if(proximoBorrado <= 0.0f){
                borrarUltima(destino);

                proximoBorrado = RITMO_BORRADO;
            }
        }

    } else {
        // Soltar la tecla reinicia la cuenta: la espera se mide desde que se
        // aprieta, no desde que se abrio la pantalla.
        tiempoRetroceso = 0.0f;
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

    enfoque = CTRL_INICIAR;

    // Por si se sale de la pantalla con el retroceso apretado: al volver, la cuenta
    // arranca de cero y no borra de golpe el nombre que se acaba de poner.
    tiempoRetroceso = 0.0f;
    proximoBorrado  = 0.0f;
}

/**
 * \brief Mueve el enfoque con las flechas, saltandose lo que no esta en pantalla.
 *
 * En solitario no hay segundo nombre, as&iacute; que ese control se brinca en la
 * direcci&oacute;n en la que se iba: parar el cursor en un campo invisible ser&iacute;a como
 * escribir a ciegas.
 *
 * \param config Configuraci&oacute;n actual, para saber si el segundo nombre existe.
 */
static void moverEnfoqueConfiguracion(const ConfigPartida& config)
{
    int paso = 0;

    if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) paso =  1;
    if(IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_LEFT))  paso = -1;

    if(paso == 0) return;

    do {
        enfoque = (enfoque + paso + NUM_CONTROLES) % NUM_CONTROLES;
    } while(config.modo == Modo_solitario && enfoque == CTRL_NOMBRE2);
}

Escena_Estado ActualizarConfiguracion(ConfigPartida& config)
{
    if(IsKeyPressed(KEY_ESCAPE)) return Escena_menu;

    moverEnfoqueConfiguracion(config);

    // ---- Clics: eligen y ademas se llevan el enfoque ----
    // Que el clic mueva el enfoque evita el salto raro de picarle a Dificil con el
    // raton y que la siguiente flecha continue desde donde estaba el teclado.
    if(botonClicado(botonModo(0))){ config.modo = Modo_solitario;    enfoque = CTRL_SOLITARIO; }
    if(botonClicado(botonModo(1))){ config.modo = Modo_multijugador; enfoque = CTRL_MULTI; }

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        if(botonClicado(botonDificultad(i))){
            config.dificultad = (Dificultad)i;
            enfoque           = CTRL_FACIL + i;
        }
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){

        if(ratonEncima(campoNombre(1))){
            enfoque = CTRL_NOMBRE1;
        } else if(config.modo == Modo_multijugador && ratonEncima(campoNombre(2))){
            enfoque = CTRL_NOMBRE2;
        }
    }

    // Al cambiar a solitario el segundo campo desaparece; si tenia el enfoque hay
    // que moverlo o se seguiria escribiendo en un cuadro invisible.
    if(config.modo == Modo_solitario && enfoque == CTRL_NOMBRE2) enfoque = CTRL_NOMBRE1;

    // ---- Escribir ----
    if(enfoque == CTRL_NOMBRE1) escribirEn(config.nombre1);
    if(enfoque == CTRL_NOMBRE2) escribirEn(config.nombre2);

    // ---- Enter ----
    // Sobre una opcion, Enter la elige. Sobre un nombre o sobre Iniciar, arranca
    // la partida: escribir el nombre y darle Enter es lo que uno espera.
    if(IsKeyPressed(KEY_ENTER)){

        if(enfoque == CTRL_SOLITARIO)  config.modo = Modo_solitario;
        else if(enfoque == CTRL_MULTI) config.modo = Modo_multijugador;
        else if(enfoque >= CTRL_FACIL && enfoque < CTRL_FACIL + NUM_DIFICULTADES){
            config.dificultad = (Dificultad)(enfoque - CTRL_FACIL);
        }
        else return Escena_juego;
    }

    if(botonClicado(botonIniciar())) return Escena_juego;

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

    dibujarDato(texto, x, y, TAMANO, COLOR_TEXTO);

    if(conFoco){
        // El cursor prende y apaga cada medio segundo. GetTime da los segundos
        // desde que abrio el juego; el residuo entre 1 parte ese segundo en dos
        // mitades, y en una se dibuja y en la otra no.
        bool visible = (GetTime() - (int)GetTime()) < 0.5;

        if(visible){
            int desplazado = x + anchoDato(texto, TAMANO) + 2;

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

    if(enfoque == CTRL_SOLITARIO) dibujarAnilloEnfoque(botonModo(0));
    if(enfoque == CTRL_MULTI)     dibujarAnilloEnfoque(botonModo(1));

    // ---- 2. Dificultad ----
    DrawText("2.  Dificultad", 80, 226, 22, COLOR_TEXTO);

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        const InfoDificultad& nivel = DIFICULTADES[i];
        Rectangle             rec   = botonDificultad(i);

        dibujarBoton(rec, nivel.nombre, config.dificultad == (Dificultad)i);

        if(enfoque == CTRL_FACIL + i) dibujarAnilloEnfoque(rec);

        // Debajo de cada boton, cuantas cartas trae. Es el dato que de verdad le
        // dice al jugador que tan larga va a ser la partida.
        const char* cuantas  = TextFormat("%d cartas", nivel.filas * nivel.columnas);
        int         anchoSub = anchoDato(cuantas, 16);

        dibujarDato(cuantas,
                    (int)(rec.x + (rec.width - anchoSub) / 2.0f),
                    (int)(rec.y + rec.height + 8.0f),
                    16, COLOR_TENUE);
    }

    // ---- 3. Jugadores ----
    DrawText(config.modo == Modo_multijugador ? "3.  Jugadores" : "3.  Jugador",
             80, 342, 22, COLOR_TEXTO);

    dibujarCampo(campoNombre(1), config.nombre1, enfoque == CTRL_NOMBRE1);

    if(config.modo == Modo_multijugador){
        dibujarCampo(campoNombre(2), config.nombre2, enfoque == CTRL_NOMBRE2);
    }

    // ---- Resumen ----
    Rectangle panel = panelResumen();

    if(hayResumen){
        Rectangle origen  = { 0.0f, 0.0f,
                              (float)texturaResumen.width, (float)texturaResumen.height };
        Vector2   desfase = { 0.0f, 0.0f };

        DrawTexturePro(texturaResumen, origen, panel, desfase, 0.0f, WHITE);
    } else {
        // Sin imagen se dibuja el panel de siempre, para poder seguir trabajando
        // en la pantalla aunque el arte no este.
        DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
        DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);
    }

    int x = (int)panel.x + 36;
    int y = (int)panel.y + 40;

    DrawText("Asi va a quedar", x, y, 24, COLOR_TITULO);

    const InfoDificultad& nivel = DIFICULTADES[config.dificultad];

    // El rotulo fijo va con la fuente de fabrica y el valor con la del juego. Se
    // dibujan en dos columnas y no en una sola cadena con espacios: en una fuente
    // donde cada letra mide distinto, alinear con espacios no alinea nada.
    const int X_VALOR = x + 150;

    DrawText("Modo:", x, y + 60, 22, COLOR_TENUE);
    dibujarDato(config.modo == Modo_solitario ? "Solitario" : "Multijugador",
                X_VALOR, y + 60, 22, COLOR_TEXTO);

    DrawText("Dificultad:", x, y + 96, 22, COLOR_TENUE);
    dibujarDato(TextFormat("%s   %dx%d   (%d pares)",
                           nivel.nombre, nivel.filas, nivel.columnas,
                           (nivel.filas * nivel.columnas) / 2),
                X_VALOR, y + 96, 22, COLOR_TEXTO);

    DrawText("Jugador:", x, y + 132, 22, COLOR_TENUE);
    dibujarDato(nombreDeJugador(config, 1), X_VALOR, y + 132, 22, COLOR_TEXTO);

    if(config.modo == Modo_multijugador){
        DrawText("Jugador:", x, y + 168, 22, COLOR_TENUE);
        dibujarDato(nombreDeJugador(config, 2), X_VALOR, y + 168, 22, COLOR_TEXTO);
    }

    dibujarBoton(botonIniciar(), "Iniciar", true);

    if(enfoque == CTRL_INICIAR) dibujarAnilloEnfoque(botonIniciar());

    dibujarTextoCentrado("Flechas para moverte     ESC para volver     ENTER para iniciar",
                         GetScreenHeight() - 40, 18, COLOR_TENUE);
}
