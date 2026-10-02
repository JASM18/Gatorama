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
#include "Opciones.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"
#include "IconosModo.hpp"

//***********************************************
// PASOS DEL ASISTENTE
//***********************************************

// La configuracion es un asistente de dos pasos. El primero pregunta el modo y
// el/los nombres juntos -en 1 vs 1, los dos campos de nombre viven en el mismo
// panel, uno junto al otro-, y ya no hay un resumen final: la partida arranca en
// cuanto se elige la dificultad.
//
//   Paso_modoYnombre  ->  Paso_dificultad  ->  (arranca la partida)
//
// ESC regresa un paso; desde el primero, regresa al menu. Todo vive en este
// archivo, con un enum interno, para no tener que dar de alta archivos nuevos en
// el proyecto.
enum PasoConfiguracion {
    Paso_modoYnombre,
    Paso_dificultad
};

static PasoConfiguracion pasoActual = Paso_modoYnombre;

//***********************************************
// ARTE DE LA PANTALLA
//***********************************************

// Una imagen completa de 1280x720 por cada vista del asistente. Traen dibujados
// el titulo, los escudos de modo, los pergaminos de los nombres, "Siguiente",
// "Atras" y las tres tarjetas de dificultad; el codigo solo escribe los nombres y
// marca lo elegido. Un pixel de la imagen es un pixel de la pantalla, asi que las
// zonas de abajo se midieron directo sobre los PNG.
//
// Las dos del primer paso comparten el acomodo -escudos, Siguiente, Atras- y solo
// cambian los pergaminos: uno ancho en solitario, dos en 1 vs 1.
static const char* RUTA_UN_JUGADOR   = "recursos/unJugadorConfig.png";
static const char* RUTA_DOS_JUGADORES = "recursos/dosJugadoresConfig.png";
static const char* RUTA_DIFICULTAD   = "recursos/dificultadConfig.png";

static Texture2D texturaUnJugador;
static Texture2D texturaDosJugadores;
static Texture2D texturaDificultad;

static bool hayUnJugador    = false;
static bool hayDosJugadores = false;
static bool hayDificultad   = false;

/** \brief Si el primer paso tiene su arte completo (las dos variantes). */
static bool artePasoModo()
{
    return hayUnJugador && hayDosJugadores;
}

// Tinta cafe oscura para los nombres sobre los pergaminos del arte.
static const Color COLOR_TINTA = { 62, 46, 30, 255 };

//***********************************************
// ACOMODO DE LA PANTALLA
//***********************************************

const int SIN_ENFOQUE  = -1;
const int NADA_ELEGIDO = -1;

// Cual opcion de dificultad esta resaltada (0, 1, 2...). Ya no hace falta para el
// modo: ese se lee directo de config.modo, ver mas abajo.
static int enfoque = SIN_ENFOQUE;

// Los controles del primer paso, acomodados en renglones como se ven en pantalla:
//
//   renglon 0:  Solitario   1 vs 1
//   renglon 1:  Nombre 1   (Nombre 2, solo en 1 vs 1)
//   renglon 2:  Siguiente
//   renglon 3:  Atras
//
// Arriba y abajo cambian de renglon; izquierda y derecha se mueven dentro de el.
const int CTRL_SOLO      = 0;
const int CTRL_VS        = 1;
const int CTRL_NOMBRE1   = 2;
const int CTRL_NOMBRE2   = 3;
const int CTRL_SIGUIENTE = 4;
const int CTRL_ATRAS     = 5;
const int NUM_RENGLONES  = 4;

// En el paso de dificultad, las tres tarjetas son 0, 1 y 2, y Atras va despues.
const int CTRL_ATRAS_DIFICULTAD = NUM_DIFICULTADES;

// La ultima tarjeta en la que estuvo el enfoque, para volver a ella al subir desde
// Atras en vez de caer siempre en Facil.
static int ultimaTarjeta = 0;

// En 1 vs 1 hay dos campos de nombre a la vista a la vez, asi que hace falta saber
// en cual se esta escribiendo: 0 es el jugador 1, 1 es el jugador 2. Con un solo
// campo (solitario) esto no se usa. Clic en un campo lo activa; TAB alterna.
static int campoActivo = 0;

// La ventana del engrane, abierta encima de la configuracion.
static bool enOpciones = false;

/**
 * \brief Las dos tarjetas de modo, chicas: comparten pantalla con el nombre.
 *
 * 260 de ancho y 40 de hueco dan exactamente 560 -el mismo ancho que el campo del
 * nombre de abajo-, para que las dos filas queden alineadas por los bordes.
 */
static Rectangle tarjetaModo(int indice)
{
    // Con el arte, los dos escudos: Solitario a la izquierda, 1 vs 1 a la derecha.
    if(artePasoModo()){
        return (indice == 0) ? rectangulo(348.0f, 120.0f, 287.0f, 178.0f)
                             : rectangulo(650.0f, 120.0f, 285.0f, 178.0f);
    }

    const float ANCHO = 260.0f;
    const float ALTO  = 150.0f;
    const float HUECO = 40.0f;
    const float Y     = 100.0f;
    float       x0    = (GetScreenWidth() - (2.0f * ANCHO + HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), Y, ANCHO, ALTO);
}

static Rectangle tarjetaDificultad(int indice)
{
    // Con el arte, las tres tarjetas con su marco oscuro.
    if(hayDificultad){
        static const Rectangle TARJETAS[3] = {
            { 316.0f, 186.0f, 211.0f, 252.0f },   // Facil
            { 546.0f, 188.0f, 210.0f, 250.0f },   // Normal
            { 770.0f, 185.0f, 209.0f, 254.0f }    // Dificil
        };

        return TARJETAS[indice];
    }

    // Tres de 240 con 30 de hueco, centradas. Estas si pueden ser grandes: en su
    // paso no comparten pantalla con nada mas.
    const float ANCHO = 240.0f;
    const float ALTO  = 280.0f;
    const float HUECO = 30.0f;
    const float Y     = 200.0f;
    float       x0    = (GetScreenWidth() - (3.0f * ANCHO + 2.0f * HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), Y, ANCHO, ALTO);
}

// Alto compartido por los campos de nombre de los dos modos: en solitario es uno
// solo y ancho completo, en 1 vs 1 son dos mas angostos, pero los tres deben verse
// del mismo grosor para que la pantalla se sienta consistente al cambiar de modo.
static const float ALTO_CAMPO_NOMBRE = 76.0f;

// El campo del nombre en solitario: uno solo, ancho completo.
static Rectangle campoNombreGrande()
{
    // Con el arte, la parte crema del pergamino ancho.
    if(artePasoModo()) return rectangulo(374.0f, 308.0f, 533.0f, 74.0f);

    const float ANCHO = 560.0f;

    return rectangulo((GetScreenWidth() - ANCHO) / 2.0f, 300.0f, ANCHO, ALTO_CAMPO_NOMBRE);
}

/**
 * \brief El campo de nombre en 1 vs 1: dos, uno junto al otro.
 *
 * 260 de ancho y 40 de hueco dan los mismos 560 que el campo solo y que las
 * tarjetas de modo de arriba, para que las tres filas queden alineadas por los
 * bordes. Comparte ALTO_CAMPO_NOMBRE con el campo solo para que los rectangulos
 * se vean del mismo tamano en los dos modos. Un poco mas abajo que el campo solo
 * (312 en vez de 300) para dejarle lugar al rotulo "Jugador 1"/"Jugador 2" encima
 * de cada uno.
 *
 * \param indice 0 para el jugador 1, 1 para el jugador 2.
 */
static Rectangle campoNombreDoble(int indice)
{
    // Con el arte, la parte crema de cada pergamino. Arriba a la izquierda traen
    // impreso "Player1:" / "Player2:"; el nombre va debajo.
    if(artePasoModo()){
        return (indice == 0) ? rectangulo(344.0f, 313.0f, 276.0f, 83.0f)
                             : rectangulo(659.0f, 315.0f, 271.0f, 82.0f);
    }

    const float ANCHO = 260.0f;
    const float HUECO = 40.0f;
    const float Y     = 312.0f;
    float       x0    = (GetScreenWidth() - (2.0f * ANCHO + HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), Y, ANCHO, ALTO_CAMPO_NOMBRE);
}

static Rectangle botonSiguiente()
{
    if(artePasoModo()) return rectangulo(520.0f, 428.0f, 240.0f, 72.0f);

    const float ANCHO = 240.0f;
    const float ALTO  = 60.0f;

    return rectangulo((GetScreenWidth() - ANCHO) / 2.0f, 430.0f, ANCHO, ALTO);
}

// Para quien juega solo con el raton: ESC ya regresa, pero un boton a la vista dice
// que se puede. En el primer paso regresa al menu.
static Rectangle botonAtras()
{
    // Las dos imagenes lo traen en el mismo lugar, abajo a la izquierda.
    bool conArte = (pasoActual == Paso_modoYnombre) ? artePasoModo() : hayDificultad;

    if(conArte) return rectangulo(37.0f, 622.0f, 148.0f, 56.0f);

    return rectangulo(40.0f, GetScreenHeight() - 96.0f, 140.0f, 46.0f);
}

//***********************************************
// ARTE DE LA PANTALLA
//***********************************************

void CargarTexturasConfiguracion()
{
    hayUnJugador    = cargarTexturaSiEsta(RUTA_UN_JUGADOR,    &texturaUnJugador);
    hayDosJugadores = cargarTexturaSiEsta(RUTA_DOS_JUGADORES, &texturaDosJugadores);
    hayDificultad   = cargarTexturaSiEsta(RUTA_DIFICULTAD,    &texturaDificultad);
}

void DescargarTexturasConfiguracion()
{
    if(hayUnJugador)    UnloadTexture(texturaUnJugador);
    if(hayDosJugadores) UnloadTexture(texturaDosJugadores);
    if(hayDificultad)   UnloadTexture(texturaDificultad);

    hayUnJugador    = false;
    hayDosJugadores = false;
    hayDificultad   = false;
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
// solo par de contadores para los dos campos: como solo se escribe en uno a la vez,
// nunca hay dos borrados corriendo al mismo tiempo.
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
// NAVEGACION ENTRE PASOS
//***********************************************

/**
 * \brief Cambia de paso y deja limpio lo que era del paso anterior.
 */
static void irAPaso(PasoConfiguracion nuevo)
{
    pasoActual  = nuevo;
    enfoque       = SIN_ENFOQUE;
    campoActivo   = 0;
    ultimaTarjeta = 0;

    // Si se cambia de paso con el retroceso apretado, la cuenta arranca de cero en
    // el siguiente campo y no le borra de golpe lo que ya llevaba.
    tiempoRetroceso = 0.0f;
    proximoBorrado  = 0.0f;
}

//***********************************************
// ACTUALIZAR
//***********************************************

void PrepararConfiguracion(ConfigPartida& config)
{
    // Se rehace entera, no solo los nombres: si el nino anterior dejo Dificil y
    // multijugador puestos, el siguiente arranca en un juego que no pidio. Y siempre
    // se empieza por el primer paso; la configuracion no se recuerda.
    config = configPorDefecto();

    // Los campos arrancan vacios: el nombre por omision ("Player 1") se ve tenue
    // como pista, y si se deja asi, nombreDeJugador lo pone al jugar. Si viniera ya
    // escrito, lo que teclea el nino se pegaria detras: "Player 1Ana".
    config.nombre1[0] = '\0';
    config.nombre2[0] = '\0';

    irAPaso(Paso_modoYnombre);
    enOpciones = false;
}

/**
 * \brief Atiende una lista de opciones (raton y teclado) y dice si se eligio una.
 *
 * \param cantidad Cuantas opciones tiene.
 * \param rectDe   Da el rectangulo de la opcion i.
 * \return El indice elegido, o NADA_ELEGIDO si en este fotograma no se eligio ninguna.
 */
static int actualizarLista(int cantidad, Rectangle (*rectDe)(int))
{
    // El raton solo cuenta cuando de verdad se movio: si se leyera cada fotograma,
    // pisaria al instante lo que se acaba de elegir con las flechas. Al moverse fuera
    // de todas las opciones, el enfoque se apaga. Es la misma regla que el cursor de
    // cartas del tablero.
    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        enfoque = SIN_ENFOQUE;

        for(int i = 0; i < cantidad; i++){
            if(ratonEncima(rectDe(i))) enfoque = i;
        }
    }

    // Las flechas las atiende quien llama (moverEnfoqueDificultad), porque ahi el
    // renglon de tarjetas tiene debajo el boton Atras.

    // Un clic elige, y ademas se lleva el enfoque.
    for(int i = 0; i < cantidad; i++){
        if(botonClicado(rectDe(i))){
            enfoque = i;
            return i;
        }
    }

    // Solo cuenta si el enfoque esta en una de las opciones: Atras se atiende aparte.
    if(IsKeyPressed(KEY_ENTER) && enfoque >= 0 && enfoque < cantidad) return enfoque;

    return NADA_ELEGIDO;
}

/**
 * \brief En qu&eacute; rengl&oacute;n est&aacute; un control del primer paso.
 */
static int renglonDe(int control)
{
    if(control == CTRL_SOLO    || control == CTRL_VS)      return 0;
    if(control == CTRL_NOMBRE1 || control == CTRL_NOMBRE2) return 1;
    if(control == CTRL_SIGUIENTE)                          return 2;

    return 3;
}

/**
 * \brief El control al que se llega al entrar a un rengl&oacute;n con las flechas.
 *
 * En los escudos, el del modo que est&aacute; elegido; en los nombres, el que estaba
 * recibiendo el teclado. As&iacute; subir y bajar nunca cambia nada por s&iacute; solo.
 */
static int entradaDeRenglon(int renglon, const ConfigPartida& config)
{
    if(renglon == 0) return (config.modo == Modo_multijugador) ? CTRL_VS : CTRL_SOLO;

    if(renglon == 1){
        bool dos = (config.modo == Modo_multijugador);
        return (dos && campoActivo == 1) ? CTRL_NOMBRE2 : CTRL_NOMBRE1;
    }

    return (renglon == 2) ? CTRL_SIGUIENTE : CTRL_ATRAS;
}

/**
 * \brief Qu&eacute; control del primer paso est&aacute; bajo el puntero, o SIN_ENFOQUE.
 */
static int controlBajoElRatonModo(const ConfigPartida& config)
{
    if(ratonEncima(tarjetaModo(0))) return CTRL_SOLO;
    if(ratonEncima(tarjetaModo(1))) return CTRL_VS;

    if(config.modo == Modo_multijugador){
        if(ratonEncima(campoNombreDoble(0))) return CTRL_NOMBRE1;
        if(ratonEncima(campoNombreDoble(1))) return CTRL_NOMBRE2;
    } else {
        if(ratonEncima(campoNombreGrande())) return CTRL_NOMBRE1;
    }

    if(ratonEncima(botonSiguiente())) return CTRL_SIGUIENTE;
    if(ratonEncima(botonAtras()))     return CTRL_ATRAS;

    return SIN_ENFOQUE;
}

/**
 * \brief Mueve el enfoque del primer paso con las flechas.
 *
 * Izquierda y derecha sobre los escudos tambi&eacute;n cambian el modo -es un
 * interruptor, como antes-; sobre los nombres en 1 vs 1, cambian de pergamino.
 */
static void moverEnfoqueModo(ConfigPartida& config)
{
    bool arriba = IsKeyPressed(KEY_UP);
    bool abajo  = IsKeyPressed(KEY_DOWN);
    bool lado   = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT);

    if(!arriba && !abajo && !lado) return;

    // La primera flecha estrena el enfoque: en el escudo del modo elegido, o en
    // Atras si fue hacia arriba.
    if(enfoque == SIN_ENFOQUE){
        enfoque = arriba ? CTRL_ATRAS : entradaDeRenglon(0, config);
        return;
    }

    if(lado){
        int renglon = renglonDe(enfoque);

        if(renglon == 0){
            config.modo = (config.modo == Modo_solitario) ? Modo_multijugador : Modo_solitario;
            enfoque     = (config.modo == Modo_multijugador) ? CTRL_VS : CTRL_SOLO;
        }
        else if(renglon == 1 && config.modo == Modo_multijugador){
            enfoque = (enfoque == CTRL_NOMBRE1) ? CTRL_NOMBRE2 : CTRL_NOMBRE1;
        }
    }

    if(arriba || abajo){
        int paso    = abajo ? 1 : -1;
        int renglon = (renglonDe(enfoque) + paso + NUM_RENGLONES) % NUM_RENGLONES;

        enfoque = entradaDeRenglon(renglon, config);
    }
}

/**
 * \brief Mueve el enfoque del paso de dificultad: tarjetas en un rengl&oacute;n y Atras
 *        debajo.
 */
static void moverEnfoqueDificultad()
{
    bool arriba    = IsKeyPressed(KEY_UP);
    bool abajo     = IsKeyPressed(KEY_DOWN);
    bool izquierda = IsKeyPressed(KEY_LEFT);
    bool derecha   = IsKeyPressed(KEY_RIGHT);

    if(!arriba && !abajo && !izquierda && !derecha) return;

    if(enfoque == SIN_ENFOQUE){
        enfoque = arriba ? CTRL_ATRAS_DIFICULTAD : 0;
        return;
    }

    if(enfoque == CTRL_ATRAS_DIFICULTAD){
        // Desde Atras, cualquier flecha menos abajo regresa a las tarjetas.
        if(!abajo) enfoque = ultimaTarjeta;
        return;
    }

    if(abajo){
        enfoque = CTRL_ATRAS_DIFICULTAD;
        return;
    }

    if(izquierda) enfoque = (enfoque + NUM_DIFICULTADES - 1) % NUM_DIFICULTADES;
    if(derecha)   enfoque = (enfoque + 1) % NUM_DIFICULTADES;
}

Escena_Estado ActualizarConfiguracion(ConfigPartida& config)
{
    // Con la ventana de opciones abierta, ella se queda con toda la entrada: el
    // ESC la cierra a ella y las letras no se escriben en el nombre de atras.
    if(enOpciones){
        if(ActualizarOpciones()) enOpciones = false;

        return Escena_configuracion;
    }

    if(botonClicado(zonaBotonOpciones())){
        enOpciones = true;
        PrepararOpciones();
        return Escena_configuracion;
    }

    // ESC y el boton Atras hacen lo mismo. Con solo dos pasos, "el anterior"
    // siempre es Paso_modoYnombre; desde ahi, al menu.
    bool enterEnAtras = IsKeyPressed(KEY_ENTER) &&
                        ((pasoActual == Paso_modoYnombre && enfoque == CTRL_ATRAS) ||
                         (pasoActual == Paso_dificultad  && enfoque == CTRL_ATRAS_DIFICULTAD));

    if(IsKeyPressed(KEY_ESCAPE) || botonClicado(botonAtras()) || enterEnAtras){

        if(pasoActual == Paso_modoYnombre) return Escena_menu;

        irAPaso(Paso_modoYnombre);
        return Escena_configuracion;
    }

    switch(pasoActual){

        case Paso_modoYnombre:
        {
            // El raton solo cuenta cuando de verdad se movio. Si se esta escribiendo
            // un nombre, hacerlo a un lado para no tapar las letras no suelta el
            // pergamino: se suelta al pasar por otro control.
            Vector2 movimientoRaton = GetMouseDelta();

            if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
                int  bajo        = controlBajoElRatonModo(config);
                bool escribiendo = (enfoque == CTRL_NOMBRE1 || enfoque == CTRL_NOMBRE2);

                if(bajo != SIN_ENFOQUE || !escribiendo) enfoque = bajo;
            }

            moverEnfoqueModo(config);

            // El modo es un interruptor, no una lista que se "confirma": tocar un
            // escudo lo elige sin salir de la pantalla, porque todavia falta
            // escribir el/los nombres aqui mismo.
            if(botonClicado(tarjetaModo(0))){ config.modo = Modo_solitario;    enfoque = CTRL_SOLO; }
            if(botonClicado(tarjetaModo(1))){ config.modo = Modo_multijugador; enfoque = CTRL_VS; }

            bool dos = (config.modo == Modo_multijugador);

            if(dos){
                if(botonClicado(campoNombreDoble(0))){ campoActivo = 0; enfoque = CTRL_NOMBRE1; }
                if(botonClicado(campoNombreDoble(1))){ campoActivo = 1; enfoque = CTRL_NOMBRE2; }

                // TAB alterna, como siempre.
                if(IsKeyPressed(KEY_TAB)){
                    campoActivo = 1 - campoActivo;
                    enfoque     = (campoActivo == 0) ? CTRL_NOMBRE1 : CTRL_NOMBRE2;
                }
            } else if(botonClicado(campoNombreGrande())){
                enfoque = CTRL_NOMBRE1;
            }

            // Con el enfoque en un pergamino, ese es el que recibe el teclado. En
            // solitario solo hay uno.
            if(enfoque == CTRL_NOMBRE1) campoActivo = 0;
            if(enfoque == CTRL_NOMBRE2) campoActivo = 1;
            if(!dos)                    campoActivo = 0;

            // Se puede escribir en cualquier momento, aunque el enfoque este en
            // otro control: lo tecleado va al pergamino activo. Asi el nino que
            // llega y escribe su nombre no tiene que saber nada de flechas.
            escribirEn(campoActivo == 0 ? config.nombre1 : config.nombre2);

            if(IsKeyPressed(KEY_ENTER)){

                // Enter sobre un escudo elige ese modo; en cualquier otro lugar
                // (un nombre, Siguiente, o nada) avanza. Sobre Atras ya se atendio
                // arriba.
                if(enfoque == CTRL_SOLO)     config.modo = Modo_solitario;
                else if(enfoque == CTRL_VS)  config.modo = Modo_multijugador;
                else                         irAPaso(Paso_dificultad);

            } else if(botonClicado(botonSiguiente())){
                irAPaso(Paso_dificultad);
            }
            break;
        }

        case Paso_dificultad:
        {
            // Ultimo paso: elegir una dificultad ya arranca la partida, sin
            // pantalla de resumen de por medio.
            moverEnfoqueDificultad();

            // Si el raton se mueve sobre Atras, tambien se lleva el enfoque.
            Vector2 movimientoRaton = GetMouseDelta();

            int elegido = actualizarLista(NUM_DIFICULTADES, tarjetaDificultad);

            if((movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f) && ratonEncima(botonAtras())){
                enfoque = CTRL_ATRAS_DIFICULTAD;
            }

            if(enfoque >= 0 && enfoque < NUM_DIFICULTADES) ultimaTarjeta = enfoque;

            if(elegido != NADA_ELEGIDO){
                config.dificultad = (Dificultad)elegido;
                return Escena_juego;
            }
            break;
        }
    }

    return Escena_configuracion;
}

//***********************************************
// DIBUJAR
//***********************************************

/**
 * \brief Escribe un texto centrado dentro de un ancho dado.
 */
static void dibujarTextoCentradoEn(const char* texto, float x, float ancho, int y, int tamano, Color color)
{
    int anchoTexto = MeasureText(texto, tamano);

    DrawText(texto, (int)(x + (ancho - anchoTexto) / 2.0f), y, tamano, color);
}

/**
 * \brief El fondo de una tarjeta de opcion.
 *
 * Resaltada (elegida, o por teclado) se ve solida; el resto del tiempo, crema con un
 * tinte y borde del color. El crema de base es para que se despegue de la madera del
 * fondo: un tinte transparente encima de ella se ve turbio.
 */
static void dibujarTarjeta(Rectangle rec, Color color, bool resaltada)
{
    if(resaltada){
        DrawRectangleRounded(rec, 0.12f, 10, color);
        return;
    }

    DrawRectangleRounded(rec, 0.12f, 10, COLOR_PANEL);
    DrawRectangleRounded(rec, 0.12f, 10, Fade(color, 0.18f));
    DrawRectangleRoundedLinesEx(rec, 0.12f, 10, 3.0f, color);
}

/**
 * \brief La tarjeta de modo, en su version chica: comparte pantalla con el nombre.
 */
static void dibujarTarjetaModo(Rectangle rec, const char* titulo, const char* subtitulo,
                               Color color, bool dosPersonas, bool resaltada)
{
    dibujarTarjeta(rec, color, resaltada);

    Color   tinta      = resaltada ? WHITE : color;
    Color   tintaTexto = resaltada ? WHITE : COLOR_TEXTO;
    Vector2 centro     = { rec.x + rec.width / 2.0f, rec.y + 48.0f };

    if(dosPersonas) iconoDosPersonas(centro, 34.0f, tinta);
    else            iconoPersona(centro, 34.0f, tinta);

    dibujarTextoCentradoEn(titulo,    rec.x, rec.width, (int)rec.y + 88,  24, tinta);
    dibujarTextoCentradoEn(subtitulo, rec.x, rec.width, (int)rec.y + 118, 14, tintaTexto);
}

/**
 * \brief Dibuja el tablero de una dificultad en chiquito, con sus filas y columnas.
 *
 * Es lo que le dice a un nino que no lee de corrido cual partida es mas larga: se ve.
 */
static void dibujarMiniTablero(Rectangle zona, int filas, int columnas, Color color)
{
    if(filas <= 0 || columnas <= 0) return;

    const float HUECO = 4.0f;

    float ladoAncho = (zona.width  - (columnas - 1) * HUECO) / columnas;
    float ladoAlto  = (zona.height - (filas    - 1) * HUECO) / filas;
    float lado      = (ladoAncho < ladoAlto) ? ladoAncho : ladoAlto;

    float anchoTotal = columnas * lado + (columnas - 1) * HUECO;
    float altoTotal  = filas    * lado + (filas    - 1) * HUECO;
    float x0         = zona.x + (zona.width  - anchoTotal) / 2.0f;
    float y0         = zona.y + (zona.height - altoTotal ) / 2.0f;

    for(int f = 0; f < filas; f++){
        for(int c = 0; c < columnas; c++){
            DrawRectangleRounded(rectangulo(x0 + c * (lado + HUECO), y0 + f * (lado + HUECO), lado, lado),
                                 0.25f, 4, color);
        }
    }
}

static void dibujarTarjetaDificultad(Rectangle rec, const InfoDificultad& nivel, bool resaltada)
{
    dibujarTarjeta(rec, COLOR_SELECCION, resaltada);

    Color tinta      = resaltada ? WHITE : COLOR_SELECCION;
    Color tintaTexto = resaltada ? WHITE : COLOR_TEXTO;

    dibujarMiniTablero(rectangulo(rec.x + 30.0f, rec.y + 26.0f, rec.width - 60.0f, 120.0f),
                       nivel.filas, nivel.columnas, tinta);

    dibujarTextoCentradoEn(nivel.nombre, rec.x, rec.width, (int)rec.y + 168, 32, tinta);

    // Debajo del nombre, cuantas cartas trae. Es el dato que de verdad le dice al
    // jugador que tan larga va a ser la partida.
    dibujarTextoCentradoEn(TextFormat("%d cartas", nivel.filas * nivel.columnas),
                           rec.x, rec.width, (int)rec.y + 220, 20, tintaTexto);
}

/**
 * \brief Un boton grande de accion, en el color que se le pase. Va siempre solido:
 *        es lo unico que hay que tocar en el paso.
 */
static void dibujarBotonAccion(Rectangle rec, const char* texto, Color color)
{
    const int TAMANO = 26;

    DrawRectangleRounded(rec, 0.35f, 10, color);

    dibujarTextoCentradoEn(texto, rec.x, rec.width,
                           (int)(rec.y + (rec.height - TAMANO) / 2.0f), TAMANO, WHITE);
}

/**
 * \brief Un campo de nombre, con su nombre por omision si esta vacio.
 *
 * Compartido entre el campo unico de solitario y los dos de 1 vs 1. El borde va
 * solido en el color cuando el campo es el que recibe lo tecleado (\p activo), y
 * atenuado cuando no -asi con dos campos a la vista se ve cual es cual-. El cursor
 * parpadeante solo se dibuja en el activo: dos parpadeando a la vez no dirian
 * cual de los dos escucha el teclado.
 *
 * \param campo      Su rectangulo.
 * \param tecleado   Lo que el jugador lleva escrito.
 * \param porOmision El nombre que se usara si se deja vacio.
 * \param color      Color del modo (coral en solo, verde-azulado en 1 vs 1).
 * \param tamano     Tamano de letra: 36 para el campo unico, mas chico para el
 *                    par -no caben 12 letras a 36 en 260 de ancho.
 * \param activo     Si es el campo que esta recibiendo el teclado ahorita.
 */
static void dibujarCampoNombre(Rectangle campo, const char* tecleado, const char* porOmision,
                               Color color, int tamano, bool activo)
{
    int x = (int)campo.x + 20;
    int y = (int)(campo.y + (campo.height - tamano) / 2.0f);

    DrawRectangleRounded(campo, 0.3f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(campo, 0.3f, 10, 4.0f, activo ? color : Fade(color, 0.35f));

    if(tecleado[0] == '\0'){
        dibujarDato(porOmision, x, y, tamano, COLOR_TENUE);
    } else {
        dibujarDato(tecleado, x, y, tamano, COLOR_TEXTO);
    }

    if(!activo) return;

    // El cursor prende y apaga cada medio segundo. GetTime da los segundos desde
    // que abrio el juego; el residuo entre 1 parte ese segundo en dos mitades, y en
    // una se dibuja y en la otra no.
    bool visible = (GetTime() - (int)GetTime()) < 0.5;

    if(visible){
        DrawRectangle(x + anchoDato(tecleado, tamano) + 2, y, 3, tamano, color);
    }
}

/**
 * \brief Un velo claro sobre una zona del arte, para marcar lo elegido o lo que est&aacute;
 *        bajo el cursor.
 *
 * Es blanco y no morado como marcarPlaca: el arte de esta pantalla es morado, y un
 * velo morado encima no se ver&iacute;a.
 *
 * \param rec       La zona.
 * \param opacidad  Qu&eacute; tanto aclara, de 0 a 1.
 */
static void aclararZona(Rectangle rec, float opacidad)
{
    DrawRectangleRounded(rec, 0.15f, 8, Fade(WHITE, opacidad));
}

/**
 * \brief Un nombre escrito sobre su pergamino del arte, con cursor si se est&aacute; editando.
 *
 * \param tecleado  Lo que lleva escrito.
 * \param porOmision El nombre que se usar&aacute; si se deja vac&iacute;o (se ve m&aacute;s tenue).
 * \param x, y      D&oacute;nde va la primera letra.
 * \param tamano    Alto de la letra.
 * \param activo    Si recibe el teclado: lleva cursor.
 */
static void dibujarNombreEnPergamino(const char* tecleado, const char* porOmision,
                                     int x, int y, int tamano, bool activo)
{
    if(tecleado[0] == '\0'){
        dibujarDato(porOmision, x, y, tamano, Fade(COLOR_TINTA, 0.45f));
    } else {
        dibujarDato(tecleado, x, y, tamano, COLOR_TINTA);
    }

    if(!activo) return;

    bool visible = (GetTime() - (int)GetTime()) < 0.5;

    if(visible) DrawRectangle(x + anchoDato(tecleado, tamano) + 2, y, 3, tamano, COLOR_TINTA);
}

/**
 * \brief El primer paso dibujado con el arte: escudos, pergamino(s) y Siguiente.
 */
static void dibujarPasoModoConArte(const ConfigPartida& config)
{
    bool dos = (config.modo == Modo_multijugador);

    DrawTexture(dos ? texturaDosJugadores : texturaUnJugador, 0, 0, WHITE);

    // El escudo del modo elegido se aclara; con el enfoque encima, un poco mas.
    for(int i = 0; i < 2; i++){
        bool      elegido    = (i == 0) ? !dos : dos;
        bool      conEnfoque = (enfoque == ((i == 0) ? CTRL_SOLO : CTRL_VS));
        Rectangle escudo     = tarjetaModo(i);

        if(elegido)         aclararZona(escudo, conEnfoque ? 0.32f : 0.22f);
        else if(conEnfoque) aclararZona(escudo, 0.12f);
    }

    if(dos){
        for(int i = 0; i < 2; i++){
            Rectangle   campo  = campoNombreDoble(i);
            bool        activo = (campoActivo == i);
            const char* nombre = (i == 0) ? config.nombre1 : config.nombre2;

            // Con dos a la vista, el que recibe el teclado se aclara.
            if(activo) aclararZona(campo, 0.35f);

            // Debajo del "PlayerN:" impreso en el arte.
            dibujarNombreEnPergamino(nombre, nombreDeJugador(config, i + 1),
                                     (int)campo.x + 22, (int)campo.y + 36, 30, activo);
        }
    } else {
        Rectangle campo = campoNombreGrande();

        if(enfoque == CTRL_NOMBRE1) aclararZona(campo, 0.35f);

        dibujarNombreEnPergamino(config.nombre1, nombreDeJugador(config, 1),
                                 (int)campo.x + 24, (int)(campo.y + (campo.height - 40.0f) / 2.0f),
                                 40, true);
    }

    marcarPlaca(botonSiguiente(), false, enfoque == CTRL_SIGUIENTE);
}

/**
 * \brief El paso combinado: elegir modo y escribir el/los nombres.
 *
 * En solitario, un campo ancho para el jugador 1. En 1 vs 1, dos campos uno junto
 * al otro -mismo panel, no una pantalla aparte-, cada uno con su rotulo encima y
 * su propio nombre por omision.
 */
static void dibujarPasoModoYNombre(const ConfigPartida& config)
{
    if(artePasoModo()){
        dibujarPasoModoConArte(config);
        return;
    }

    dibujarTarjetaModo(tarjetaModo(0), "Solitario", "Juegas tu solo",
                       COLOR_MODO_SOLO, false, config.modo == Modo_solitario);

    dibujarTarjetaModo(tarjetaModo(1), "1 vs 1", "Juegas con un amigo",
                       COLOR_MODO_VS, true, config.modo == Modo_multijugador);

    Color color = (config.modo == Modo_multijugador) ? COLOR_MODO_VS : COLOR_MODO_SOLO;

    if(config.modo == Modo_multijugador){

        Rectangle campo1 = campoNombreDoble(0);
        Rectangle campo2 = campoNombreDoble(1);

        dibujarTextoCentradoEn("Jugador 1", campo1.x, campo1.width, (int)campo1.y - 22, 16, COLOR_FONDO_TENUE);
        dibujarTextoCentradoEn("Jugador 2", campo2.x, campo2.width, (int)campo2.y - 22, 16, COLOR_FONDO_TENUE);

        dibujarCampoNombre(campo1, config.nombre1, nombreDeJugador(config, 1), color, 26, campoActivo == 0);
        dibujarCampoNombre(campo2, config.nombre2, nombreDeJugador(config, 2), color, 26, campoActivo == 1);

    } else {
        dibujarCampoNombre(campoNombreGrande(), config.nombre1, nombreDeJugador(config, 1), color, 36, true);
    }

    Rectangle siguiente = botonSiguiente();

    dibujarBotonAccion(siguiente, "Siguiente", color);

    if(enfoque == CTRL_SIGUIENTE) dibujarAnilloEnfoque(siguiente);

    if(enfoque == CTRL_SOLO) dibujarAnilloEnfoque(tarjetaModo(0));
    if(enfoque == CTRL_VS)   dibujarAnilloEnfoque(tarjetaModo(1));
}

static void dibujarPasoDificultad()
{
    if(hayDificultad){
        DrawTexture(texturaDificultad, 0, 0, WHITE);

        // El enfoque puede estar en Atras (indice 3), que no es una tarjeta.
        if(enfoque >= 0 && enfoque < NUM_DIFICULTADES) aclararZona(tarjetaDificultad(enfoque), 0.30f);
        return;
    }

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        dibujarTarjetaDificultad(tarjetaDificultad(i), DIFICULTADES[i], enfoque == i);
    }
}

static const char* tituloDelPaso(const ConfigPartida& config)
{
    if(pasoActual == Paso_dificultad) return "ELIGE LA DIFICULTAD";

    // Un solo titulo para los dos modos: como ya no hay resumen final, es el unico
    // texto que ve el jugador antes de escribir su nombre, y no tiene por que
    // cambiar segun cuantos campos haya debajo.
    (void)config;

    return "ELIGE TU MODO Y TU NOMBRE";
}

static const char* ayudaDelPaso(const ConfigPartida& config)
{
    if(pasoActual == Paso_dificultad) return "Toca una dificultad para empezar a jugar     ESC para volver";

    if(config.modo == Modo_multijugador)
        return "Clic o TAB para cambiar de campo     ENTER para continuar     ESC para volver";

    return "Elige un modo, escribe tu nombre y ENTER     ESC para volver";
}

void DibujarConfiguracion(const ConfigPartida& config)
{
    // Con el arte del paso, el titulo, "Atras" y la ayuda ya vienen en la imagen o
    // sobran: el arte se explica solo, y la linea de ayuda chocaba con el escudo.
    bool conArte = (pasoActual == Paso_modoYnombre) ? artePasoModo() : hayDificultad;

    if(!conArte) dibujarTextoCentradoSobreFondo(tituloDelPaso(config), 24, 30, COLOR_FONDO_TITULO);

    switch(pasoActual){
        case Paso_modoYnombre: dibujarPasoModoYNombre(config); break;
        case Paso_dificultad:  dibujarPasoDificultad();        break;
    }

    Rectangle atras = botonAtras();

    bool atrasConEnfoque = (pasoActual == Paso_modoYnombre) ? (enfoque == CTRL_ATRAS)
                                                            : (enfoque == CTRL_ATRAS_DIFICULTAD);

    if(conArte){
        marcarPlaca(atras, false, atrasConEnfoque);
    } else {
        dibujarBoton(atras, "Atras", false);

        if(atrasConEnfoque) dibujarAnilloEnfoque(atras);

        dibujarTextoCentradoSobreFondo(ayudaDelPaso(config), GetScreenHeight() - 40, 18, COLOR_FONDO_TENUE);
    }

    dibujarBotonOpciones(zonaBotonOpciones());

    // La ventana va hasta el final, encima de todo.
    if(enOpciones) DibujarOpciones();
}
