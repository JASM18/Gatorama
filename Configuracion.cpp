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

// La configuracion ya no es una sola pantalla con todo encima: es un asistente que
// hace una pregunta a la vez, como se preguntaria en voz alta.
//
//   Paso_modo  ->  Paso_nombre1  ->  [Paso_nombre2]  ->  Paso_dificultad  ->  Paso_resumen
//
// Paso_nombre2 solo existe en multijugador. En solitario simplemente no esta en la
// cadena, asi que ya no hay un campo que esconder ni un enfoque que brincar.
//
// Modo y dificultad avanzan en cuanto se toca una opcion. Los nombres no pueden
// hacerlo (hay que terminar de escribir), asi que se avanza con ENTER o con el boton
// Siguiente. ESC regresa un paso; desde el primero, regresa al menu.
//
// Todo vive en este archivo, con un enum interno, para no tener que dar de alta
// archivos nuevos en el proyecto.
enum PasoConfiguracion {
    Paso_modo,
    Paso_nombre1,
    Paso_nombre2,
    Paso_dificultad,
    Paso_resumen
};

static PasoConfiguracion pasoActual = Paso_modo;

//***********************************************
// ACOMODO DE LA PANTALLA
//***********************************************

const int SIN_ENFOQUE  = -1;
const int NADA_ELEGIDO = -1;

// Cual opcion del paso actual esta resaltada (0, 1, 2...). Un solo dato para el
// raton y el teclado: el raton lo mueve cuando de verdad se mueve, las flechas cuando
// se presionan, y quien lo tenga es el que se ve resaltado. Es el mismo modelo que el
// cursor de cartas del tablero (VistaTablero + Juego): empieza en nada y se limpia al
// cambiar de paso.
static int enfoque = SIN_ENFOQUE;

// La ventana del engrane, abierta encima de la configuracion.
static bool enOpciones = false;

// Las tarjetas de modo y de dificultad miden lo mismo de alto y van a la misma altura,
// para que al pasar de un paso a otro no brinque nada.
static const float TARJETA_Y    = 200.0f;
static const float TARJETA_ALTO = 280.0f;

static Rectangle tarjetaModo(int indice)
{
    // Dos de 300 con 50 de hueco, centradas.
    const float ANCHO = 300.0f;
    const float HUECO = 50.0f;
    float       x0    = (GetScreenWidth() - (2.0f * ANCHO + HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), TARJETA_Y, ANCHO, TARJETA_ALTO);
}

static Rectangle tarjetaDificultad(int indice)
{
    // Tres de 240 con 30 de hueco, centradas.
    const float ANCHO = 240.0f;
    const float HUECO = 30.0f;
    float       x0    = (GetScreenWidth() - (3.0f * ANCHO + 2.0f * HUECO)) / 2.0f;

    return rectangulo(x0 + indice * (ANCHO + HUECO), TARJETA_Y, ANCHO, TARJETA_ALTO);
}

// El campo donde se escribe el nombre, grande: es lo unico que hay en la pantalla.
static Rectangle campoNombreGrande()
{
    const float ANCHO = 560.0f;
    const float ALTO  = 76.0f;

    return rectangulo((GetScreenWidth() - ANCHO) / 2.0f, 300.0f, ANCHO, ALTO);
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

// El pergamino ya no comparte pantalla con los botones de modo y dificultad, asi que
// baja a donde quede centrado entre los puntos de avance y el pie. Es el unico numero
// que hay que tocar para moverlo (antes: 258).
static Rectangle panelResumen()
{
    return rectangulo((GetScreenWidth() - 560.0f) / 2.0f, 170.0f, 560.0f, 400.0f);
}

/**
 * \brief La l&iacute;nea del pergamino donde va el nombre de un jugador.
 *
 * Es la raya que va despu&eacute;s de "Jugador 1:" o "Jugador 2:" en configResumen.png.
 * El texto se dibuja 8 px adentro, en la misma columna que Modo y Dificultad.
 *
 * \param numJugador 1 o 2.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle campoNombre(int numJugador)
{
    Rectangle panel = panelResumen();

    return rectangulo(panel.x + 200.0f, panel.y + (numJugador == 1 ? 188.0f : 242.0f),
                      210.0f, 40.0f);
}

static Rectangle botonIniciar()
{
    Rectangle panel = panelResumen();

    // En la esquina inferior derecha del pergamino, como la firma al pie de un
    // contrato. 210x105 mantiene la proporcion 2:1 del PNG para que "Iniciar" no se
    // estire.
    const float ANCHO = 210.0f;
    const float ALTO  = 105.0f;

    return rectangulo(panel.x + panel.width  - ANCHO - 20.0f,
                      panel.y + panel.height - ALTO  - 16.0f,
                      ANCHO, ALTO);
}

// Las listas de opciones se recorren con una funcion que da el rectangulo del
// indice i. Iniciar es una lista de uno.
static Rectangle rectIniciar(int)
{
    return botonIniciar();
}

//***********************************************
// ARTE DE LA PANTALLA
//***********************************************

// El panel del resumen. Mide 560x400, lo mismo que panelResumen(). La lamina ya
// trae dibujados el titulo y los rotulos -Modo, Dificultad, Jugador 1 y 2- con una
// linea al lado de cada uno: el codigo solo pone el valor sobre la linea.
static const char* RUTA_RESUMEN = "recursos/configResumen.png";

// El boton Iniciar, en sus dos estados. Cada PNG es 240x120, con la palabra
// "Iniciar" sobre transparente. Se dibuja el "activo" cuando el raton esta encima
// o el teclado lo tiene enfocado, y el "inactivo" el resto del tiempo. Es la misma
// idea que las placas del menu (botonesInactivo_fondo / botonActivo_fondo).
static const char* RUTA_INICIAR_ACTIVO   = "recursos/botonIniciar_activo.png";
static const char* RUTA_INICIAR_INACTIVO = "recursos/botonIniciar_inactivo.png";

static Texture2D texturaResumen;
static Texture2D texturaIniciarActivo;
static Texture2D texturaIniciarInactivo;

static bool hayResumen         = false;
static bool hayIniciarActivo   = false;
static bool hayIniciarInactivo = false;

void CargarTexturasConfiguracion()
{
    hayResumen         = cargarTexturaSiEsta(RUTA_RESUMEN,           &texturaResumen);
    hayIniciarActivo   = cargarTexturaSiEsta(RUTA_INICIAR_ACTIVO,    &texturaIniciarActivo);
    hayIniciarInactivo = cargarTexturaSiEsta(RUTA_INICIAR_INACTIVO,  &texturaIniciarInactivo);
}

void DescargarTexturasConfiguracion()
{
    if(hayResumen)         UnloadTexture(texturaResumen);
    if(hayIniciarActivo)   UnloadTexture(texturaIniciarActivo);
    if(hayIniciarInactivo) UnloadTexture(texturaIniciarInactivo);

    hayResumen         = false;
    hayIniciarActivo   = false;
    hayIniciarInactivo = false;
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

static PasoConfiguracion pasoSiguiente(const ConfigPartida& config)
{
    switch(pasoActual){
        case Paso_modo:       return Paso_nombre1;
        case Paso_nombre1:    return config.modo == Modo_multijugador ? Paso_nombre2 : Paso_dificultad;
        case Paso_nombre2:    return Paso_dificultad;
        default:              return Paso_resumen;
    }
}

static PasoConfiguracion pasoAnterior(const ConfigPartida& config)
{
    switch(pasoActual){
        case Paso_resumen:    return Paso_dificultad;
        case Paso_dificultad: return config.modo == Modo_multijugador ? Paso_nombre2 : Paso_nombre1;
        case Paso_nombre2:    return Paso_nombre1;
        default:              return Paso_modo;
    }
}

/**
 * \brief Cambia de paso y deja limpio lo que era del paso anterior.
 */
static void irAPaso(PasoConfiguracion nuevo)
{
    pasoActual = nuevo;
    enfoque    = SIN_ENFOQUE;

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

    irAPaso(Paso_modo);
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

    // ESC y el boton Atras hacen lo mismo: un paso para atras, o al menu si ya se
    // esta en el primero.
    if(IsKeyPressed(KEY_ESCAPE) || botonClicado(botonAtras())){

        if(pasoActual == Paso_modo) return Escena_menu;

        irAPaso(pasoAnterior(config));
        return Escena_configuracion;
    }

    switch(pasoActual){

        case Paso_modo:
        {
            int elegido = actualizarLista(2, tarjetaModo);

            if(elegido != NADA_ELEGIDO){
                config.modo = (elegido == 0) ? Modo_solitario : Modo_multijugador;
                irAPaso(Paso_nombre1);
            }
            break;
        }

        case Paso_nombre1:
        case Paso_nombre2:
        {
            // Aqui no hay lista que recorrer: el campo siempre esta listo para
            // escribir, sin tener que picarle primero.
            char* destino = (pasoActual == Paso_nombre1) ? config.nombre1 : config.nombre2;

            escribirEn(destino);

            // ENTER es lo que uno espera despues de escribir un nombre.
            if(IsKeyPressed(KEY_ENTER) || botonClicado(botonSiguiente())){
                irAPaso(pasoSiguiente(config));
            }
            break;
        }

        case Paso_dificultad:
        {
            int elegido = actualizarLista(NUM_DIFICULTADES, tarjetaDificultad);

            if(elegido != NADA_ELEGIDO){
                config.dificultad = (Dificultad)elegido;
                irAPaso(Paso_resumen);
            }
            break;
        }

        case Paso_resumen:
        {
            // ENTER inicia aunque nada este resaltado: es el ultimo paso.
            if(actualizarLista(1, rectIniciar) != NADA_ELEGIDO || IsKeyPressed(KEY_ENTER)){
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
 * Resaltada (por raton o por teclado) se ve solida; el resto del tiempo, crema con un
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

static void dibujarTarjetaModo(Rectangle rec, const char* titulo, const char* subtitulo,
                               Color color, bool dosPersonas, bool resaltada)
{
    dibujarTarjeta(rec, color, resaltada);

    Color   tinta      = resaltada ? WHITE : color;
    Color   tintaTexto = resaltada ? WHITE : COLOR_TEXTO;
    Vector2 centro     = { rec.x + rec.width / 2.0f, rec.y + 100.0f };

    if(dosPersonas) iconoDosPersonas(centro, 70.0f, tinta);
    else            iconoPersona(centro, 70.0f, tinta);

    dibujarTextoCentradoEn(titulo,    rec.x, rec.width, (int)rec.y + 185, 34, tinta);
    dibujarTextoCentradoEn(subtitulo, rec.x, rec.width, (int)rec.y + 235, 18, tintaTexto);
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

static void dibujarPasoModo()
{
    dibujarTarjetaModo(tarjetaModo(0), "Solitario", "Juegas tu solo",
                       COLOR_MODO_SOLO, false, enfoque == 0);

    dibujarTarjetaModo(tarjetaModo(1), "1 vs 1", "Juegas con un amigo",
                       COLOR_MODO_VS, true, enfoque == 1);
}

static void dibujarPasoDificultad()
{
    for(int i = 0; i < NUM_DIFICULTADES; i++){
        dibujarTarjetaDificultad(tarjetaDificultad(i), DIFICULTADES[i], enfoque == i);
    }
}

/**
 * \brief La pantalla de un nombre: un icono, un campo grande y Siguiente.
 *
 * Mientras se escribe se ve lo tecleado con su cursor parpadeante. Si el campo esta
 * vacio se ve, atenuado, el nombre que quedara por omision.
 *
 * \param numJugador 1 o 2.
 */
static void dibujarPasoNombre(const ConfigPartida& config, int numJugador)
{
    Color color = (config.modo == Modo_multijugador) ? COLOR_MODO_VS : COLOR_MODO_SOLO;

    // El icono va sobre un circulo crema para que se lea contra la madera.
    Vector2 centroIcono = { GetScreenWidth() / 2.0f, 205.0f };

    DrawCircleV(centroIcono, 64.0f, COLOR_PANEL);
    DrawRing(centroIcono, 60.0f, 64.0f, 0.0f, 360.0f, 48, color);
    iconoPersona(centroIcono, 52.0f, color);

    // ---- Campo ----
    Rectangle   campo    = campoNombreGrande();
    const char* tecleado = (numJugador == 1) ? config.nombre1 : config.nombre2;
    const int   TAMANO   = 36;
    int         x        = (int)campo.x + 24;
    int         y        = (int)(campo.y + (campo.height - TAMANO) / 2.0f);

    DrawRectangleRounded(campo, 0.3f, 10, COLOR_PANEL);
    DrawRectangleRoundedLinesEx(campo, 0.3f, 10, 4.0f, color);

    if(tecleado[0] == '\0'){
        dibujarDato(nombreDeJugador(config, numJugador), x, y, TAMANO, COLOR_TENUE);
    } else {
        dibujarDato(tecleado, x, y, TAMANO, COLOR_TEXTO);
    }

    // El cursor prende y apaga cada medio segundo. GetTime da los segundos desde
    // que abrio el juego; el residuo entre 1 parte ese segundo en dos mitades, y en
    // una se dibuja y en la otra no.
    bool visible = (GetTime() - (int)GetTime()) < 0.5;

    if(visible){
        DrawRectangle(x + anchoDato(tecleado, TAMANO) + 2, y, 3, TAMANO, color);
    }

    // ---- Siguiente ----
    Rectangle siguiente = botonSiguiente();

    dibujarBotonAccion(siguiente, "Siguiente", color);

    if(ratonEncima(siguiente)) dibujarAnilloEnfoque(siguiente);
}

/**
 * \brief El resumen: el pergamino con lo elegido y la firma para empezar.
 *
 * Todo se lee, nada se escribe aqui: modo, dificultad y nombres ya se eligieron en
 * los pasos de antes.
 */
static void dibujarPasoResumen(const ConfigPartida& config)
{
    Rectangle panel = panelResumen();

    const InfoDificultad& nivel = DIFICULTADES[config.dificultad];

    const char* modoTexto = config.modo == Modo_solitario ? "Solitario" : "1 vs 1";

    // Los cuatro renglones, medidos sobre configResumen.png: donde empieza el
    // valor y la altura de cada linea. Sin la lamina se usan los mismos, para que
    // los valores no cambien de lugar si falta el arte.
    const int XV     = (int)panel.x + 208;
    const int XR     = (int)panel.x + 36;
    const int Y_MODO = (int)panel.y + 88;
    const int Y_DIF  = (int)panel.y + 141;
    const int Y_J1   = (int)panel.y + 198;
    const int Y_J2   = (int)panel.y + 252;
    const int TAMANO = 22;

    if(hayResumen){

        Rectangle origen  = { 0.0f, 0.0f,
                              (float)texturaResumen.width, (float)texturaResumen.height };
        Vector2   desfase = { 0.0f, 0.0f };

        DrawTexturePro(texturaResumen, origen, panel, desfase, 0.0f, WHITE);

    } else {

        // Sin lamina, el mismo pergamino hecho con figuras y rotulos.
        DrawRectangleRounded(panel, 0.06f, 10, COLOR_PANEL);
        DrawRectangleRoundedLinesEx(panel, 0.06f, 10, 2.0f, COLOR_TENUE);

        DrawText("Configuracion", XR, (int)panel.y + 30, 28, COLOR_TITULO);
        DrawText("Modo:",         XR, Y_MODO, TAMANO, COLOR_TENUE);
        DrawText("Dificultad:",   XR, Y_DIF,  TAMANO, COLOR_TENUE);
        DrawText("Jugador 1:",    XR, Y_J1,   TAMANO, COLOR_TENUE);
        DrawText("Jugador 2:",    XR, Y_J2,   TAMANO, COLOR_TENUE);
    }

    // La dificultad va corta -"nombre  FxC"- porque la version larga no cabe en la linea.
    dibujarDato(modoTexto, XV, Y_MODO, TAMANO, COLOR_TEXTO);
    dibujarDato(TextFormat("%s   %dx%d", nivel.nombre, nivel.filas, nivel.columnas),
                XV, Y_DIF, TAMANO, COLOR_TEXTO);

    // Los nombres ya escritos: lo que hay tecleado, o el de por omision si se dejo
    // vacio. "Jugador 2:" esta impreso siempre; en solitario su linea se queda vacia.
    dibujarDato(nombreDeJugador(config, 1), (int)campoNombre(1).x + 8, (int)campoNombre(1).y + 10,
                TAMANO, COLOR_TEXTO);

    if(config.modo == Modo_multijugador){
        dibujarDato(nombreDeJugador(config, 2), (int)campoNombre(2).x + 8, (int)campoNombre(2).y + 10,
                    TAMANO, COLOR_TEXTO);
    }

    // ---- Boton Iniciar ----
    Rectangle zonaIniciar = botonIniciar();

    if(hayIniciarActivo && hayIniciarInactivo){

        // Resaltado por raton o por teclado: en los dos casos se ve el activo.
        bool      resaltado = ratonEncima(zonaIniciar) || enfoque == 0;
        Texture2D tex       = resaltado ? texturaIniciarActivo : texturaIniciarInactivo;

        Rectangle origen  = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Vector2   desfase = { 0.0f, 0.0f };

        DrawTexturePro(tex, origen, zonaIniciar, desfase, 0.0f, WHITE);

    } else {
        // Sin las laminas, el boton dibujado de siempre.
        dibujarBoton(zonaIniciar, "Iniciar", enfoque == 0);
    }

    // La firma no lleva anillo de enfoque: el cambio de tinta -cafe a morada-
    // cuando el raton entra o el teclado la elige ya dice cual esta a punto de
    // activarse, y un recuadro sobre el pergamino rompia la ilusion.
}

/**
 * \brief Que numero de paso es el actual, contando desde 0, segun el modo elegido.
 */
static int indiceDelPaso(const ConfigPartida& config)
{
    bool multi = (config.modo == Modo_multijugador);

    switch(pasoActual){
        case Paso_modo:       return 0;
        case Paso_nombre1:    return 1;
        case Paso_nombre2:    return 2;
        case Paso_dificultad: return multi ? 3 : 2;
        default:              return multi ? 4 : 3;
    }
}

/**
 * \brief Los puntitos de avance: cuantos pasos van y cuantos faltan.
 *
 * En solitario son cuatro; en multijugador cinco, porque hay un nombre mas. Al
 * elegir el modo en el primer paso, la cuenta puede pasar de cuatro a cinco.
 */
static void dibujarAvance(const ConfigPartida& config)
{
    const int   total  = (config.modo == Modo_multijugador) ? 5 : 4;
    const int   actual = indiceDelPaso(config);
    const float RADIO  = 7.0f;
    const float HUECO  = 26.0f;

    float x0 = GetScreenWidth() / 2.0f - (total - 1) * HUECO / 2.0f;

    for(int i = 0; i < total; i++){
        Vector2 centro = { x0 + i * HUECO, 92.0f };

        if(i <= actual){
            DrawCircleV(centro, RADIO, COLOR_SELECCION);
        } else {
            DrawCircleV(centro, RADIO, Fade(WHITE, 0.35f));
        }
    }
}

static const char* tituloDelPaso(const ConfigPartida& config)
{
    switch(pasoActual){
        case Paso_modo:       return "ELIGE COMO QUIERES JUGAR";
        case Paso_nombre1:    return config.modo == Modo_multijugador ? "JUGADOR 1: ESCRIBE TU NOMBRE"
                                                                       : "ESCRIBE TU NOMBRE";
        case Paso_nombre2:    return "JUGADOR 2: ESCRIBE TU NOMBRE";
        case Paso_dificultad: return "ELIGE LA DIFICULTAD";
        default:              return "LISTO PARA JUGAR";
    }
}

static const char* ayudaDelPaso()
{
    switch(pasoActual){
        case Paso_modo:
        case Paso_dificultad: return "Toca una opcion     ESC para volver";
        case Paso_nombre1:
        case Paso_nombre2:    return "Escribe tu nombre     ENTER para continuar     ESC para volver";
        default:              return "ESC para volver     ENTER para iniciar";
    }
}

void DibujarConfiguracion(const ConfigPartida& config)
{
    dibujarTextoCentradoSobreFondo(tituloDelPaso(config), 32, 34, COLOR_FONDO_TITULO);

    dibujarAvance(config);

    switch(pasoActual){
        case Paso_modo:       dibujarPasoModo();              break;
        case Paso_nombre1:    dibujarPasoNombre(config, 1);   break;
        case Paso_nombre2:    dibujarPasoNombre(config, 2);   break;
        case Paso_dificultad: dibujarPasoDificultad();        break;
        case Paso_resumen:    dibujarPasoResumen(config);     break;
    }

    Rectangle atras = botonAtras();

    dibujarBoton(atras, "Atras", ratonEncima(atras));

    dibujarTextoCentradoSobreFondo(ayudaDelPaso(), GetScreenHeight() - 40, 18, COLOR_FONDO_TENUE);

    dibujarBotonOpciones(zonaBotonOpciones());

    // La ventana va hasta el final, encima de todo.
    if(enOpciones) DibujarOpciones();
}
