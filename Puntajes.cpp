/**
 * \file Puntajes.cpp
 * \brief Implementaci&oacute;n de la pantalla de mejores puntajes.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstring>

#include "raylib.h"

#include "Puntajes.hpp"
#include "TablaPuntajes.hpp"
#include "Dificultad.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ESTADO DE LA PANTALLA
//***********************************************

// Cuantos renglones se muestran. Ocho caben sin apretar debajo de los filtros.
const int RENGLONES = 8;

static TablaPuntajes tabla;
static bool          cargada = false;

static Puntaje mejores[RENGLONES];
static int     numMejores = 0;

// Por que columna se esta ordenando y en que direccion, como en el explorador de
// archivos: el encabezado es el control. El primer clic en una columna la ordena
// como uno espera, y el segundo invierte.
static ColumnaPuntaje columnaOrden = Columna_puntos;
static bool           descendente  = true;

// Los controles que recorre el teclado, en orden: los dos modos, las tres
// dificultades, los cuatro encabezados y el boton de volver.
const int CTRL_MODO      = 0;
const int CTRL_NIVEL     = 2;
const int CTRL_COLUMNA   = 5;
const int CTRL_VOLVER    = 9;
const int NUM_CONTROLES  = 10;

static int enfoque = CTRL_VOLVER;

// La categoria que se esta viendo. La tabla NO mezcla dificultades ni modos: un
// puntaje de Dificil siempre le ganaria a uno de Facil por el tamano del tablero
// -15 parejas contra 5- y no por jugar mejor. Cada categoria es su propio ranking.
static ModoJuego  modoFiltro       = Modo_solitario;
static Dificultad dificultadFiltro = Dificultad_facil;

/**
 * \brief Se asegura de que la tabla est&eacute; le&iacute;da del archivo.
 *
 * Se lee una sola vez por ejecuci&oacute;n. Volver a leerla en cada dibujo ser&iacute;a abrir el
 * archivo sesenta veces por segundo para nada.
 */
static void asegurarCargada()
{
    if(cargada) return;

    tabla.Cargar(RUTA_PUNTAJES);
    cargada = true;
}

/**
 * \brief Rellena el arreglo de los mejores seg&uacute;n la pesta&ntilde;a elegida.
 *
 * Se hace al cambiar de pesta&ntilde;a y al entrar, no al dibujar: armar un mont&iacute;culo por
 * fotograma ser&iacute;a trabajo tirado a la basura sesenta veces por segundo.
 */
static void recalcularMejores()
{
    numMejores = tabla.Mejores(mejores, RENGLONES,
                               comparadorDe(columnaOrden, descendente),
                               modoFiltro, dificultadFiltro);
}

//***********************************************
// REGISTRO DE PARTIDAS
//***********************************************

void GuardarResultado(const ConfigPartida& config, const Partida& partida)
{
    asegurarCargada();

    for(int j = 0; j < partida.NumJugadores(); j++){

        Puntaje p = puntajeVacio();

        strncpy(p.nombre, nombreDeJugador(config, j + 1), LARGO_NOMBRE);
        p.nombre[LARGO_NOMBRE] = '\0';

        p.modo       = config.modo;
        p.dificultad = config.dificultad;
        p.pares      = partida.ParesDe(j);
        p.puntos     = partida.PuntajeDe(j);
        p.racha      = partida.RachaMaximaDe(j);   // la mejor de la partida, no la de ahorita
        p.intentos   = partida.Intentos();
        // El tiempo DEL JUGADOR, no el de la partida. En 1 vs 1 el reloj de la
        // partida los mezcla a los dos y no sirve para comparar a nadie.
        p.tiempo     = partida.TiempoDe(j);
        p.gano       = (partida.Ganador() == j);

        tabla.Agregar(p);
    }

    // Se escribe de inmediato y no al cerrar el juego. En un stand alguien va a
    // cerrar la ventana con la tacha, y todo lo que no se haya guardado se pierde.
    tabla.Guardar(RUTA_PUNTAJES);
}

//***********************************************
// PANTALLA
//***********************************************

//***********************************************
// ACOMODO DE LA PANTALLA
//***********************************************

// Donde empieza cada columna. Las cuatro ordenables van en el mismo orden que el
// enum ColumnaPuntaje, para que el indice sirva para las dos cosas.
static const int X_LUGAR   = 200;
static const int X_NOMBRE  = 250;

static const int X_COLUMNA[NUM_COLUMNAS_ORDEN] = { 520, 660, 790, 920 };

static const int Y_ENCABEZADO = 206;
static const int Y_PRIMERO    = 250;
static const int ALTO_RENGLON = 40;

static Rectangle botonModo(int cual)
{
    return rectangulo(470.0f + cual * 180.0f, 84.0f, 160.0f, 40.0f);
}

static Rectangle botonNivel(int cual)
{
    return rectangulo(399.0f + cual * 166.0f, 134.0f, 150.0f, 40.0f);
}

/**
 * \brief La zona clicable del encabezado de una columna.
 * \param columna Columna, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle zonaEncabezado(int columna)
{
    return rectangulo((float)X_COLUMNA[columna] - 12.0f, (float)Y_ENCABEZADO - 8.0f,
                      116.0f, 32.0f);
}

static Rectangle botonVolver()
{
    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 74.0f, 220.0f, 48.0f);
}

void PrepararPuntajes()
{
    // Se relee por si otra cosa toco el archivo, y porque entrar a la pantalla es
    // exactamente el momento en que importa estar al dia.
    tabla.Cargar(RUTA_PUNTAJES);
    cargada = true;

    modoFiltro       = Modo_solitario;
    dificultadFiltro = Dificultad_facil;
    columnaOrden     = Columna_puntos;
    descendente      = true;
    enfoque          = CTRL_VOLVER;

    recalcularMejores();
}

/**
 * \brief El rect&aacute;ngulo de un control, sea del tipo que sea.
 *
 * Tener los diez en una sola funci&oacute;n permite que el enfoque y el rat&oacute;n hablen el
 * mismo idioma: un &iacute;ndice.
 *
 * \param control &Iacute;ndice del control, de 0 a NUM_CONTROLES - 1.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle zonaControl(int control)
{
    if(control < CTRL_NIVEL)   return botonModo(control);
    if(control < CTRL_COLUMNA) return botonNivel(control - CTRL_NIVEL);
    if(control < CTRL_VOLVER)  return zonaEncabezado(control - CTRL_COLUMNA);

    return botonVolver();
}

/**
 * \brief Activa el control enfocado, venga de un clic o de un Enter.
 * \param control &Iacute;ndice del control.
 */
static void activarControl(int control)
{
    if(control < CTRL_NIVEL){
        ModoJuego nuevo = (control == 0) ? Modo_solitario : Modo_multijugador;

        if(modoFiltro != nuevo){ modoFiltro = nuevo; recalcularMejores(); }
        return;
    }

    if(control < CTRL_COLUMNA){
        Dificultad nuevo = (Dificultad)(control - CTRL_NIVEL);

        if(dificultadFiltro != nuevo){ dificultadFiltro = nuevo; recalcularMejores(); }
        return;
    }

    if(control < CTRL_VOLVER){
        ColumnaPuntaje elegida = (ColumnaPuntaje)(control - CTRL_COLUMNA);

        if(columnaOrden == elegida){
            // Segundo clic en la misma columna: se invierte, como en el explorador.
            descendente = !descendente;
        } else {
            columnaOrden = elegida;
            descendente  = ordenDescendentePorOmision(elegida);
        }

        recalcularMejores();
    }
}

Escena_Estado ActualizarPuntajes()
{
    // El raton manda sobre el teclado: lo que esta bajo el puntero toma el enfoque.
    for(int i = 0; i < NUM_CONTROLES; i++){
        if(ratonEncima(zonaControl(i))) enfoque = i;
    }

    enfoque = moverEnfoque(enfoque, NUM_CONTROLES, true);

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonVolver())) return Escena_menu;

    if(enfoqueActivado()){
        if(enfoque == CTRL_VOLVER) return Escena_menu;

        activarControl(enfoque);
    }

    // Los clics pasan por la misma puerta que el teclado, para que las dos formas
    // de usar la pantalla no puedan comportarse distinto.
    for(int i = 0; i < CTRL_VOLVER; i++){
        if(botonClicado(zonaControl(i))) activarControl(i);
    }

    return Escena_puntajes;
}

//***********************************************
// DIBUJAR
//***********************************************

/**
 * \brief Escribe un tiempo en segundos como minutos:segundos.
 * \param segundos Tiempo del jugador.
 * \return Cadena lista para dibujar, v&aacute;lida hasta la siguiente llamada.
 */
static const char* comoReloj(float segundos)
{
    int total = (int)segundos;

    return TextFormat("%02d:%02d", total / 60, total % 60);
}

/**
 * \brief Color del rengl&oacute;n seg&uacute;n el lugar.
 * \param lugar Posici&oacute;n empezando en cero.
 * \return Color del texto.
 */
static Color colorDeLugar(int lugar)
{
    if(lugar == 0) return COLOR_BOTON_ACTIVO;   // el primero resalta
    if(lugar < 3)  return COLOR_SELECCION;      // segundo y tercero, un poco

    return COLOR_TEXTO;
}

/**
 * \brief Dibuja el encabezado de una columna ordenable.
 *
 * La columna activa va en color y con una flecha que dice hacia d&oacute;nde ordena. Sin
 * esa flecha nadie sabe si est&aacute; viendo lo mejor o lo peor.
 *
 * \param columna Columna, desde cero.
 */
static void dibujarEncabezado(int columna)
{
    Rectangle zona   = zonaEncabezado(columna);
    bool      activa = (columnaOrden == (ColumnaPuntaje)columna);

    // Un fondo tenue al pasar el raton avisa que el encabezado se puede picar.
    // Sin eso nadie adivina que la tabla se ordena desde ahi.
    if(ratonEncima(zona)) DrawRectangleRounded(zona, 0.3f, 6, COLOR_BOTON);

    if(enfoque == CTRL_COLUMNA + columna) dibujarAnilloEnfoque(zona);

    const char* titulo = tituloDeColumna((ColumnaPuntaje)columna);
    Color       color  = activa ? COLOR_BOTON_ACTIVO : COLOR_TENUE;

    DrawText(titulo, X_COLUMNA[columna], Y_ENCABEZADO, 18, color);

    if(activa){
        // La flecha va a la derecha del titulo. Hacia arriba de menor a mayor,
        // hacia abajo de mayor a menor, como en el explorador de archivos.
        int desplazado = X_COLUMNA[columna] + MeasureText(titulo, 18) + 6;

        DrawText(descendente ? "v" : "^", desplazado, Y_ENCABEZADO, 18, COLOR_BOTON_ACTIVO);
    }
}

/**
 * \brief Dibuja la tabla completa: encabezados y renglones.
 */
static void dibujarTabla()
{
    DrawText("#",       X_LUGAR,  Y_ENCABEZADO, 18, COLOR_TENUE);
    DrawText("Jugador", X_NOMBRE, Y_ENCABEZADO, 18, COLOR_TENUE);

    for(int i = 0; i < NUM_COLUMNAS_ORDEN; i++){
        dibujarEncabezado(i);
    }

    for(int i = 0; i < numMejores; i++){
        const Puntaje& p = mejores[i];

        int   y     = Y_PRIMERO + i * ALTO_RENGLON;
        Color color = colorDeLugar(i);

        dibujarDato(TextFormat("%d", i + 1), X_LUGAR,  y, 22, color);
        dibujarDato(p.nombre,                X_NOMBRE, y, 22, color);

        // Los valores van en el mismo orden que las columnas del enum.
        dibujarDato(TextFormat("%d", p.puntos), X_COLUMNA[Columna_puntos], y, 22, COLOR_TEXTO);
        dibujarDato(TextFormat("%d", p.racha),  X_COLUMNA[Columna_racha],  y, 22, COLOR_TEXTO);
        dibujarDato(TextFormat("%d", p.pares),  X_COLUMNA[Columna_pares],  y, 22, COLOR_TEXTO);
        dibujarDato(comoReloj(p.tiempo),        X_COLUMNA[Columna_tiempo], y, 22, COLOR_TEXTO);
    }
}

void DibujarPuntajes()
{
    dibujarTextoCentrado("MEJORES PUNTAJES", 26, 34, COLOR_TITULO);

    // ---- Categoria: modo y dificultad ----
    dibujarBoton(botonModo(0), "Solo",   modoFiltro == Modo_solitario);
    dibujarBoton(botonModo(1), "1 vs 1", modoFiltro == Modo_multijugador);

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        dibujarBoton(botonNivel(i), DIFICULTADES[i].nombre,
                     dificultadFiltro == (Dificultad)i);
    }

    // El anillo del enfoque, salvo en los encabezados: esos lo dibujan ellos
    // mismos, porque solo existen cuando hay tabla que ordenar.
    if(enfoque < CTRL_COLUMNA || enfoque == CTRL_VOLVER){
        dibujarAnilloEnfoque(zonaControl(enfoque));
    }

    if(numMejores == 0){
        dibujarTextoCentrado("Nadie ha jugado en esta categoria todavia",
                             300, 24, COLOR_TEXTO);

        dibujarDatoCentrado(TextFormat("%s en %s",
                                       modoFiltro == Modo_solitario ? "Solo" : "1 vs 1",
                                       DIFICULTADES[dificultadFiltro].nombre),
                            340, 20, COLOR_TENUE);
    } else {
        dibujarTabla();

        // Raya bajo los encabezados, para separarlos de los datos.
        DrawLine(180, Y_ENCABEZADO + 30, GetScreenWidth() - 180, Y_ENCABEZADO + 30,
                 COLOR_TENUE);
    }

    dibujarTextoCentrado("Clic o flechas y Enter en Puntos, Racha, Pares o Tiempo para reordenar",
                         GetScreenHeight() - 108, 16, COLOR_TENUE);

    dibujarBoton(botonVolver(), "Volver al menu", false);
}
