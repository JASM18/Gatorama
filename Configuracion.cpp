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
// ACOMODO DE LA PANTALLA
//***********************************************

const int SIN_ENFOQUE  = -1;
const int NADA_ELEGIDO = -1;

// Cual opcion de dificultad esta resaltada (0, 1, 2...). Ya no hace falta para el
// modo: ese se lee directo de config.modo, ver mas abajo.
static int enfoque = SIN_ENFOQUE;

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
    const float ANCHO = 260.0f;
    const float ALTO  = 150.0f;
    const float HUECO = 40.0f;
    const float Y     = 100.0f;
    float       x0    = (GetScreenWidth() - (2.0f * ANCHO + HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), Y, ANCHO, ALTO);
}

static Rectangle tarjetaDificultad(int indice)
{
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
    const float ANCHO = 260.0f;
    const float HUECO = 40.0f;
    const float Y     = 312.0f;
    float       x0    = (GetScreenWidth() - (2.0f * ANCHO + HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), Y, ANCHO, ALTO_CAMPO_NOMBRE);
}

static Rectangle botonSiguiente()
{
    const float ANCHO = 240.0f;
    const float ALTO  = 60.0f;

    return rectangulo((GetScreenWidth() - ANCHO) / 2.0f, 430.0f, ANCHO, ALTO);
}

// Para quien juega solo con el raton: ESC ya regresa, pero un boton a la vista dice
// que se puede. En el primer paso regresa al menu.
static Rectangle botonAtras()
{
    return rectangulo(40.0f, GetScreenHeight() - 96.0f, 140.0f, 46.0f);
}

//***********************************************
// ARTE DE LA PANTALLA
//***********************************************

// Ya no hay pergamino ni boton Iniciar con textura: la partida arranca en cuanto
// se elige la dificultad, asi que no hay una pantalla final que "firmar". Estas dos
// funciones se quedan -Juego.cpp las sigue llamando- pero ya no cargan nada.
void CargarTexturasConfiguracion()
{
}

void DescargarTexturasConfiguracion()
{
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
    enfoque     = SIN_ENFOQUE;
    campoActivo = 0;

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

    irAPaso(Paso_modoYnombre);
    enOpciones = false;
}

/**
 * \brief Mueve el enfoque entre las opciones de una lista con las flechas.
 *
 * \param cantidad Cuantas opciones tiene el paso.
 */
static void moverEnfoqueLista(int cantidad)
{
    int salto = 0;

    if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) salto =  1;
    if(IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_LEFT))  salto = -1;

    if(salto == 0) return;

    // La primera flecha estrena el enfoque sin saltarse nada: hacia adelante entra
    // por la primera opcion, hacia atras por la ultima.
    if(enfoque == SIN_ENFOQUE){
        enfoque = (salto > 0) ? 0 : cantidad - 1;
        return;
    }

    enfoque = (enfoque + salto + cantidad) % cantidad;
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

    moverEnfoqueLista(cantidad);

    // Un clic elige, y ademas se lleva el enfoque.
    for(int i = 0; i < cantidad; i++){
        if(botonClicado(rectDe(i))){
            enfoque = i;
            return i;
        }
    }

    if(IsKeyPressed(KEY_ENTER) && enfoque != SIN_ENFOQUE) return enfoque;

    return NADA_ELEGIDO;
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
    if(IsKeyPressed(KEY_ESCAPE) || botonClicado(botonAtras())){

        if(pasoActual == Paso_modoYnombre) return Escena_menu;

        irAPaso(Paso_modoYnombre);
        return Escena_configuracion;
    }

    switch(pasoActual){

        case Paso_modoYnombre:
        {
            // El modo es un interruptor, no una lista que se "confirma": tocar una
            // tarjeta o mover las flechas cambia cual esta elegida sin salir de la
            // pantalla, porque todavia falta escribir el/los nombres aqui mismo.
            // Por eso NO pasa por actualizarLista, que si haria avanzar con ENTER.
            if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)){
                config.modo = (config.modo == Modo_solitario) ? Modo_multijugador : Modo_solitario;
            }
            if(botonClicado(tarjetaModo(0))) config.modo = Modo_solitario;
            if(botonClicado(tarjetaModo(1))) config.modo = Modo_multijugador;

            if(config.modo == Modo_multijugador){

                // Dos campos a la vista: un clic en uno lo activa, TAB alterna.
                // Solo el activo recibe lo que se teclee.
                if(botonClicado(campoNombreDoble(0))) campoActivo = 0;
                if(botonClicado(campoNombreDoble(1))) campoActivo = 1;
                if(IsKeyPressed(KEY_TAB))             campoActivo = 1 - campoActivo;

                escribirEn(campoActivo == 0 ? config.nombre1 : config.nombre2);

            } else {
                escribirEn(config.nombre1);
            }

            // ENTER es lo unico que de verdad avanza: asi nunca se sale de la
            // pantalla a medio escribir un nombre.
            if(IsKeyPressed(KEY_ENTER) || botonClicado(botonSiguiente())){
                irAPaso(Paso_dificultad);
            }
            break;
        }

        case Paso_dificultad:
        {
            // Ultimo paso: elegir una dificultad ya arranca la partida, sin
            // pantalla de resumen de por medio.
            int elegido = actualizarLista(NUM_DIFICULTADES, tarjetaDificultad);

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
 * \brief El paso combinado: elegir modo y escribir el/los nombres.
 *
 * En solitario, un campo ancho para el jugador 1. En 1 vs 1, dos campos uno junto
 * al otro -mismo panel, no una pantalla aparte-, cada uno con su rotulo encima y
 * su propio nombre por omision.
 */
static void dibujarPasoModoYNombre(const ConfigPartida& config)
{
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

    if(ratonEncima(siguiente)) dibujarAnilloEnfoque(siguiente);
}

static void dibujarPasoDificultad()
{
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
    dibujarTextoCentradoSobreFondo(tituloDelPaso(config), 24, 30, COLOR_FONDO_TITULO);

    switch(pasoActual){
        case Paso_modoYnombre: dibujarPasoModoYNombre(config); break;
        case Paso_dificultad:  dibujarPasoDificultad();        break;
    }

    Rectangle atras = botonAtras();

    dibujarBoton(atras, "Atras", ratonEncima(atras));

    dibujarTextoCentradoSobreFondo(ayudaDelPaso(config), GetScreenHeight() - 40, 18, COLOR_FONDO_TENUE);

    dibujarBotonOpciones(zonaBotonOpciones());

    // La ventana va hasta el final, encima de todo.
    if(enOpciones) DibujarOpciones();
}
