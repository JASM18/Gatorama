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
#include "Opciones.hpp"
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
const int SIN_ENFOQUE    = -1;

// Cual control esta resaltado. Mismo modelo que el cursor de cartas del tablero y
// que la configuracion: empieza en nada, el raton lo mueve solo cuando de verdad se
// mueve (y lo apaga al salir de todo), y las flechas lo estrenan.
static int enfoque = SIN_ENFOQUE;

// La ventana del engrane, abierta encima de la tabla.
static bool enOpciones = false;

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
// ARTE DE LA PANTALLA
//***********************************************

// La pantalla completa, 1280x720: cortinas, pergamino, titulo, las cinco placas de
// filtro, los encabezados de la tabla con sus rayas y la placa de "Volver al
// menu". Se dibuja en 0,0 y un pixel de la imagen es un pixel de la pantalla, asi
// que todas las zonas de abajo se midieron directo sobre el PNG.
static const char* RUTA_FONDO = "recursos/mejoresPuntajes.png";

static Texture2D texturaFondo;
static bool      hayFondo = false;

void CargarTexturasPuntajes()
{
    hayFondo = cargarTexturaSiEsta(RUTA_FONDO, &texturaFondo);
}

void DescargarTexturasPuntajes()
{
    if(hayFondo) UnloadTexture(texturaFondo);

    hayFondo = false;
}

//***********************************************
// ACOMODO DE LA PANTALLA
//***********************************************

// Las placas, medidas sobre mejoresPuntajes.png por su contorno exterior. No son
// una formula de "x mas N por boton" porque estan dibujadas a mano: cada una quedo
// de su tamano y a su altura.
static const Rectangle PLACAS_MODO[2] = {
    { 474.0f,  76.0f, 167.0f, 49.0f },   // Solo
    { 653.0f,  77.0f, 170.0f, 51.0f }    // 1 vs 1
};

static const Rectangle PLACAS_NIVEL[NUM_DIFICULTADES] = {
    { 401.0f, 128.0f, 155.0f, 50.0f },   // Facil
    { 563.0f, 127.0f, 162.0f, 50.0f },   // Normal
    { 736.0f, 130.0f, 162.0f, 48.0f }    // Dificil
};

static const Rectangle PLACA_VOLVER = { 512.0f, 641.0f, 249.0f, 65.0f };

// Las columnas de la tabla, de raya a raya en el arte. La primera -"# Jugador"- no
// se ordena; las otras cuatro van en el mismo orden que el enum ColumnaPuntaje.
static const float X_RAYA_JUGADOR = 180.0f;
static const float X_RAYAS[NUM_COLUMNAS_ORDEN + 1] = { 466.0f, 605.0f, 733.0f, 868.0f, 1060.0f };

static const int X_LUGAR   = 214;
static const int X_NOMBRE  = 262;

static const float Y_ENCABEZADO = 188.0f;   // arriba de los titulos del arte
static const float ALTO_ENCABEZADO = 46.0f; // hasta la raya horizontal (y = 234)

static const int Y_PRIMERO    = 256;   // deja lugar a la flecha de orden
static const int ALTO_RENGLON = 40;

// Tinta cafe oscura, la del arte, para lo que se escribe sobre el pergamino.
static const Color COLOR_TINTA = { 62, 46, 30, 255 };

static Rectangle botonModo(int cual)
{
    return PLACAS_MODO[cual];
}

static Rectangle botonNivel(int cual)
{
    return PLACAS_NIVEL[cual];
}

/**
 * \brief La zona clicable del encabezado de una columna: la celda del arte entre
 *        sus dos rayas, del t&iacute;tulo a la raya horizontal.
 * \param columna Columna, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle zonaEncabezado(int columna)
{
    float izquierda = X_RAYAS[columna];
    float derecha   = X_RAYAS[columna + 1];

    return rectangulo(izquierda + 3.0f, Y_ENCABEZADO, derecha - izquierda - 6.0f, ALTO_ENCABEZADO);
}

static Rectangle botonVolver()
{
    return PLACA_VOLVER;
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
    enfoque          = SIN_ENFOQUE;
    enOpciones       = false;

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
 * \brief Qu&eacute; control est&aacute; bajo el puntero, o SIN_ENFOQUE si ninguno.
 */
static int controlBajoElRaton()
{
    for(int i = 0; i < NUM_CONTROLES; i++){
        if(ratonEncima(zonaControl(i))) return i;
    }

    return SIN_ENFOQUE;
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
    // Con la ventana de opciones abierta, ella se queda con toda la entrada: el
    // ESC la cierra a ella, no saca de la pantalla.
    if(enOpciones){
        if(ActualizarOpciones()) enOpciones = false;

        return Escena_puntajes;
    }

    if(botonClicado(zonaBotonOpciones())){
        enOpciones = true;
        PrepararOpciones();
        return Escena_puntajes;
    }

    if(IsKeyPressed(KEY_ESCAPE))    return Escena_menu;
    if(botonClicado(botonVolver())) return Escena_menu;

    // El raton solo cuenta cuando de verdad se movio: si se leyera cada fotograma,
    // pisaria al instante lo que se acaba de elegir con las flechas.
    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        enfoque = controlBajoElRaton();
    }

    bool adelante = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT);
    bool atras    = IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_LEFT);

    if(enfoque == SIN_ENFOQUE){
        // La primera flecha estrena el enfoque: hacia adelante por el primer
        // filtro, hacia atras por "Volver".
        if(adelante)   enfoque = CTRL_MODO;
        else if(atras) enfoque = CTRL_VOLVER;
    } else {
        enfoque = moverEnfoque(enfoque, NUM_CONTROLES, true);
    }

    if(enfoqueActivado() && enfoque != SIN_ENFOQUE){
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

    return hayFondo ? COLOR_TINTA : COLOR_TEXTO;
}

/**
 * \brief Una flechita que dice hacia d&oacute;nde ordena la columna activa.
 *
 * Tri&aacute;ngulo y no una letra "v": con la fuente de f&aacute;brica se ve como una ve.
 *
 * \param x      Centro horizontal.
 * \param y      Centro vertical.
 * \param abajo  Verdadero si ordena de mayor a menor.
 * \param color  Color.
 */
static void dibujarFlecha(float x, float y, bool abajo, Color color)
{
    const float MITAD = 6.0f;

    // raylib dibuja los triangulos en sentido contrario a las manecillas.
    if(abajo){
        DrawTriangle({ x - MITAD, y - MITAD / 2 }, { x, y + MITAD / 2 + 2 }, { x + MITAD, y - MITAD / 2 }, color);
    } else {
        DrawTriangle({ x - MITAD, y + MITAD / 2 }, { x + MITAD, y + MITAD / 2 }, { x, y - MITAD / 2 - 2 }, color);
    }
}

/**
 * \brief Dibuja el encabezado de una columna ordenable.
 *
 * Con el arte, el t&iacute;tulo ya est&aacute; impreso: solo se agrega el velo al pasar por
 * encima y la flecha de la columna activa. Sin esa flecha nadie sabe si est&aacute;
 * viendo lo mejor o lo peor.
 *
 * \param columna Columna, desde cero.
 */
static void dibujarEncabezado(int columna)
{
    Rectangle zona   = zonaEncabezado(columna);
    bool      activa = (columnaOrden == (ColumnaPuntaje)columna);

    // Un velo al pasar el raton avisa que el encabezado se puede picar. Sin eso
    // nadie adivina que la tabla se ordena desde ahi.
    if(enfoque == CTRL_COLUMNA + columna){
        DrawRectangleRounded(zona, 0.3f, 6, Fade(WHITE, 0.30f));
    }

    if(hayFondo){
        // La flecha va centrada justo debajo de la raya del encabezado: en la
        // celda no cabe junto al titulo sin encimarse con la ultima letra.
        if(activa){
            dibujarFlecha(zona.x + zona.width / 2.0f, zona.y + zona.height + 8.0f,
                          descendente, COLOR_TINTA);
        }
        return;
    }

    const char* titulo = tituloDeColumna((ColumnaPuntaje)columna);
    Color       color  = activa ? COLOR_BOTON_ACTIVO : COLOR_TENUE;
    int         x      = (int)zona.x + 12;
    int         y      = (int)(zona.y + 14.0f);

    DrawText(titulo, x, y, 18, color);

    if(activa){
        dibujarFlecha(x + MeasureText(titulo, 18) + 12.0f, y + 9.0f, descendente, COLOR_BOTON_ACTIVO);
    }
}

/**
 * \brief Dibuja la tabla completa: encabezados y renglones.
 */
static void dibujarTabla()
{
    if(!hayFondo){
        DrawText("#",       X_LUGAR,  (int)Y_ENCABEZADO + 14, 18, COLOR_TENUE);
        DrawText("Jugador", X_NOMBRE, (int)Y_ENCABEZADO + 14, 18, COLOR_TENUE);
    }

    for(int i = 0; i < NUM_COLUMNAS_ORDEN; i++){
        dibujarEncabezado(i);
    }

    Color tinta = hayFondo ? COLOR_TINTA : COLOR_TEXTO;

    for(int i = 0; i < numMejores; i++){
        const Puntaje& p = mejores[i];

        int   y     = Y_PRIMERO + i * ALTO_RENGLON;
        Color color = colorDeLugar(i);

        dibujarDato(TextFormat("%d", i + 1), X_LUGAR,  y, 22, color);
        dibujarDato(p.nombre,                X_NOMBRE, y, 22, color);

        // Cada valor va centrado en su columna, entre las dos rayas del arte.
        const char* valores[NUM_COLUMNAS_ORDEN];
        valores[Columna_puntos] = TextFormat("%d", p.puntos);
        valores[Columna_racha]  = TextFormat("%d", p.racha);
        valores[Columna_pares]  = TextFormat("%d", p.pares);
        valores[Columna_tiempo] = comoReloj(p.tiempo);

        for(int c = 0; c < NUM_COLUMNAS_ORDEN; c++){
            float centro = (X_RAYAS[c] + X_RAYAS[c + 1]) / 2.0f;

            dibujarDato(valores[c], (int)(centro - anchoDato(valores[c], 22) / 2.0f), y, 22, tinta);
        }
    }
}

void DibujarPuntajes()
{
    if(hayFondo){
        DrawTexture(texturaFondo, 0, 0, WHITE);
    } else {
        dibujarTextoCentrado("MEJORES PUNTAJES", 26, 34, COLOR_TITULO);
    }

    // ---- Categoria: modo y dificultad ----
    const char* NOMBRES_MODO[2] = { "Solo", "1 vs 1" };

    for(int i = 0; i < 2; i++){
        bool elegida = (modoFiltro == (i == 0 ? Modo_solitario : Modo_multijugador));

        if(hayFondo) marcarPlaca(botonModo(i), elegida, enfoque == CTRL_MODO + i);
        else         dibujarBoton(botonModo(i), NOMBRES_MODO[i], elegida);
    }

    for(int i = 0; i < NUM_DIFICULTADES; i++){
        bool elegida = (dificultadFiltro == (Dificultad)i);

        if(hayFondo) marcarPlaca(botonNivel(i), elegida, enfoque == CTRL_NIVEL + i);
        else         dibujarBoton(botonNivel(i), DIFICULTADES[i].nombre, elegida);
    }

    if(numMejores == 0){
        dibujarTextoCentrado("Nadie ha jugado en esta categoria todavia",
                             380, 24, hayFondo ? COLOR_TINTA : COLOR_TEXTO);

        dibujarDatoCentrado(TextFormat("%s en %s",
                                       modoFiltro == Modo_solitario ? "Solo" : "1 vs 1",
                                       DIFICULTADES[dificultadFiltro].nombre),
                            420, 20, COLOR_TENUE);

        // Sin tabla que ordenar, los encabezados no se dibujan; con el arte, sus
        // titulos siguen impresos pero sin flecha.
    } else {
        dibujarTabla();

        // Raya bajo los encabezados, para separarlos de los datos. El arte ya la trae.
        if(!hayFondo){
            DrawLine((int)X_RAYA_JUGADOR, (int)(Y_ENCABEZADO + ALTO_ENCABEZADO),
                     (int)X_RAYAS[NUM_COLUMNAS_ORDEN], (int)(Y_ENCABEZADO + ALTO_ENCABEZADO),
                     COLOR_TENUE);
        }
    }

    // Sobre el pergamino, en cafe medio: el gris de COLOR_TENUE casi no se leia.
    dibujarTextoCentrado("Clic o flechas y Enter en Puntos, Racha, Pares o Tiempo para reordenar",
                         612, 16, hayFondo ? Color{ 104, 80, 54, 255 } : COLOR_TENUE);

    if(hayFondo) marcarPlaca(botonVolver(), false, enfoque == CTRL_VOLVER);
    else         dibujarBoton(botonVolver(), "Volver al menu", enfoque == CTRL_VOLVER);

    dibujarBotonOpciones(zonaBotonOpciones());

    // La ventana va hasta el final, encima de todo.
    if(enOpciones) DibujarOpciones();
}
