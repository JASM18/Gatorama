/**
 * \file Juego.cpp
 * \brief Implementaci&oacute;n de la pantalla de la partida.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Juego.hpp"
#include "Partida.hpp"
#include "VistaTablero.hpp"
#include "Dificultad.hpp"
#include "Pausa.hpp"
#include "Instrucciones.hpp"
#include "Puntajes.hpp"
#include "Audio.hpp"
#include "Resultados.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ESTADO DE LA PANTALLA
//***********************************************

// Cuanto se queda un par equivocado a la vista antes de taparse. Menos de medio
// segundo no alcanza para memorizarlo y mas de un segundo se siente lento.
//
// Vive aqui y no en Partida a proposito: es una decision de cuanto tarda algo en
// pantalla, no una regla del juego. Las reglas no deben saber de segundos.
static const float ESPERA_OCULTAR = 0.9f;

// Cuanto se deja ver el tablero completo antes de pasar a la pantalla de
// resultados. Cortar de inmediato no deja ver la ultima pareja que se destapo.
static const float ESPERA_RESULTADOS = 1.6f;

static Partida*      partida = 0;    ///< Las reglas y el tablero; se reserva al iniciar
static ConfigPartida configActual;   ///< Con que se armo, para poder reiniciar

static float esperaOcultar   = 0.0f; ///< Lo que falta para tapar un par fallido
static int   cartaResaltada  = -1;   ///< Carta bajo el puntero; -1 si ninguna
static bool  enPausa         = false;
static bool  enInstrucciones = false;
static bool  resultadoGuardado = false;   ///< Para no anotar la misma partida dos veces
static float esperaResultados  = 0.0f;    ///< Lo que falta para cambiar de pantalla

// De donde se abrio la ventana de instrucciones. Si se abrio desde el tablero, al
// cerrarla hay que regresar al juego; si se abrio desde la pausa, hay que regresar
// a la pausa. Sin esta bandera, cerrar la ayuda del tablero dejaba el juego en
// pausa sin que nadie la hubiera pedido.
static bool  ayudaDesdePausa   = false;

/**
 * \brief El acomodo de las cartas para el tablero actual.
 *
 * Actualizar y dibujar lo recalculan por su cuenta. Es una decena de
 * multiplicaciones: sale m&aacute;s barato que guardarlo y arriesgarse a detectar el clic
 * en un lugar distinto del que se dibuj&oacute;.
 */
static DisenoTablero disenoActual()
{
    const Tablero& t = partida->ElTablero();

    // areaDeCartas y no areaDelTablero: el tapete cubre todo, las cartas van
    // metidas hacia adentro para no encimarse con su marco.
    return calcularDiseno(t.Filas(), t.Columnas(), areaDeCartas(), RELACION_CARTA);
}

/**
 * \brief El bot&oacute;n de pausa, arriba a la izquierda.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonDePausa()
{
    return rectangulo(20.0f, 18.0f, 120.0f, 40.0f);
}

/**
 * \brief El bot&oacute;n de ayuda, arriba a la derecha, como en el boceto.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonDeAyuda()
{
    return rectangulo(GetScreenWidth() - 60.0f, 18.0f, 40.0f, 40.0f);
}

/**
 * \brief El recuadro del marcador de un jugador.
 * \param jugador 0 para el de la izquierda, 1 para el de la derecha.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle bloqueJugador(int jugador)
{
    const float ANCHO = 330.0f;

    if(jugador == 0) return rectangulo(20.0f, 76.0f, ANCHO, 56.0f);

    return rectangulo(GetScreenWidth() - 20.0f - ANCHO, 76.0f, ANCHO, 56.0f);
}

//***********************************************
// ARMAR Y REINICIAR
//***********************************************

void IniciarPartida(const ConfigPartida& config)
{
    configActual = config;

    // Se libera la partida anterior antes de armar la nueva. Sin esto, cada vez
    // que alguien saliera al menu y volviera a jugar se quedaria una partida
    // reservada sin que nadie la pueda alcanzar: una fuga de las clasicas.
    delete partida;
    partida = 0;

    int jugadores = (config.modo == Modo_multijugador) ? 2 : 1;

    try {
        partida = new Partida(config.dificultad, jugadores);
    }
    catch(const std::exception&) {
        // Solo puede pasar si alguien deja mal la tabla de dificultades o el modo.
        // No se puede jugar eso, pero tampoco se va a cerrar el juego enfrente de
        // un nino: se cae de pie a una partida chica de un jugador.
        partida = new Partida(Dificultad_facil, 1);
    }

    // Que gatos salen en esta partida se sortea aqui, junto con el reparto.
    barajarIlustraciones();

    esperaOcultar     = 0.0f;
    cartaResaltada    = -1;
    enPausa           = false;
    enInstrucciones   = false;
    resultadoGuardado = false;
    esperaResultados  = 0.0f;
    ayudaDesdePausa   = false;
}

void ReiniciarPartida()
{
    if(partida == 0) return;

    partida->Reiniciar();
    barajarIlustraciones();

    // La pausa se cierra tambien. Reiniciar significa volver a jugar: dejar el
    // panel abierto encima del tablero nuevo obligaba a cerrarlo a mano.
    enPausa           = false;

    esperaOcultar     = 0.0f;
    cartaResaltada    = -1;
    enInstrucciones   = false;
    resultadoGuardado = false;
    esperaResultados  = 0.0f;
    ayudaDesdePausa   = false;
}

void LiberarPartida()
{
    delete partida;
    partida = 0;
}

//***********************************************
// ACTUALIZAR
//***********************************************

/**
 * \brief Mueve la carta resaltada con las flechas del teclado.
 *
 * Se topa en los bordes en lugar de dar la vuelta: en un tablero de 3x10, saltar
 * de la ultima carta a la primera desorienta m&aacute;s de lo que ayuda.
 */
static void moverConFlechas()
{
    int pasoX = 0;
    int pasoY = 0;

    if(IsKeyPressed(KEY_RIGHT)) pasoX =  1;
    if(IsKeyPressed(KEY_LEFT))  pasoX = -1;
    if(IsKeyPressed(KEY_DOWN))  pasoY =  1;
    if(IsKeyPressed(KEY_UP))    pasoY = -1;

    if(pasoX == 0 && pasoY == 0) return;

    // La primera flecha estrena el cursor en la esquina, sin mover nada mas.
    if(cartaResaltada < 0){
        cartaResaltada = 0;
        return;
    }

    const Tablero& t        = partida->ElTablero();
    int            columnas = t.Columnas();

    int fila    = cartaResaltada / columnas + pasoY;
    int columna = cartaResaltada % columnas + pasoX;

    if(fila < 0)              fila = 0;
    if(fila >= t.Filas())     fila = t.Filas() - 1;
    if(columna < 0)           columna = 0;
    if(columna >= columnas)   columna = columnas - 1;

    cartaResaltada = fila * columnas + columna;
}

Escena_Estado ActualizarJuego()
{
    // Nadie deberia llegar aqui sin partida, pero si pasa es mejor regresar al
    // menu que desreferenciar un puntero nulo.
    if(partida == 0) return Escena_menu;

    // Las instrucciones van encima de la pausa, asi que se atienden primero: si
    // estan abiertas, son ellas las que se quedan con el ESC.
    if(enInstrucciones){
        if(ActualizarInstrucciones()){
            enInstrucciones = false;

            // Se vuelve a donde se estaba: al panel de pausa si de ahi se abrio, o
            // directo al tablero si se abrio con el boton de ayuda.
            if(!ayudaDesdePausa) enPausa = false;
        }

        return Escena_juego;
    }

    if(enPausa){
        AccionPausa accion = ActualizarPausa();

        if(accion == Pausa_continuar)           enPausa = false;
        else if(accion == Pausa_reiniciar)      ReiniciarPartida();
        else if(accion == Pausa_instrucciones){
            enInstrucciones = true;
            ayudaDesdePausa = true;
        }
        else if(accion == Pausa_menu)           return Escena_menu;

        // Se regresa aqui mismo: nada de lo de abajo corre. Eso es lo que congela
        // el reloj y lo que hace que los clics del panel no volteen cartas.
        return Escena_juego;
    }

    if(IsKeyPressed(KEY_ESCAPE) || botonClicado(botonDePausa())){
        enPausa = true;
        PrepararPausa();
        return Escena_juego;
    }

    // El boton de ayuda tambien pausa: leer las instrucciones no deberia costarle
    // tiempo al jugador.
    if(botonClicado(botonDeAyuda())){
        enPausa         = true;   // congela el reloj mientras lee
        enInstrucciones = true;
        ayudaDesdePausa = false;  // al cerrar se vuelve al tablero, no a la pausa
        PrepararPausa();
        return Escena_juego;
    }

    if(partida->Terminada()){

        // La partida se anota en el archivo en cuanto termina, y una sola vez. Se
        // hace aqui y no al salir de la pantalla porque en un stand alguien va a
        // cerrar la ventana con la tacha, y lo que no se guardo se pierde.
        if(!resultadoGuardado){
            // Suena en el momento en que se junta la ultima pareja, no al llegar a
            // la pantalla de resultados: el aplauso va cuando pasa la cosa.
            ReproducirVictoria();

            GuardarResultado(configActual, *partida);
            PrepararResultados(configActual, *partida);

            resultadoGuardado = true;
            esperaResultados  = ESPERA_RESULTADOS;
        }

        esperaResultados -= GetFrameTime();

        if(esperaResultados <= 0.0f) return Escena_resultados;

        return Escena_juego;
    }

    // El reloj lo lleva la partida. Se le pasan los segundos que duro el fotograma
    // anterior en vez de que ella los mida: asi el modelo no depende de raylib, y
    // una prueba de consola puede simular una partida larga al instante. Partida
    // se detiene sola cuando ya no quedan parejas.
    partida->CorrerReloj(GetFrameTime());

    // El raton solo manda cuando de verdad se movio. Si se leyera cada fotograma,
    // borraria en el acto la carta que el jugador acaba de elegir con las flechas.
    Vector2 movimiento = GetMouseDelta();

    if(movimiento.x != 0.0f || movimiento.y != 0.0f){
        cartaResaltada = indiceCartaEnPunto(disenoActual(), GetMousePosition());
    }

    moverConFlechas();

    // Un par equivocado se queda a la vista y luego se tapa. Quien decide CUANDO
    // es esta pantalla; quien sabe QUE significa taparlo -romper la racha, pasar
    // el turno- es Partida.
    if(esperaOcultar > 0.0f){
        esperaOcultar -= GetFrameTime();

        if(esperaOcultar <= 0.0f) partida->ResolverFallo();

        // Mientras se resuelve no se aceptan clics. Partida tambien los rechaza,
        // pero salir aqui evita hasta recalcular el acomodo.
        return Escena_juego;
    }

    // Un clic del raton y un Enter del teclado hacen exactamente lo mismo: voltear
    // la carta resaltada. Por eso los dos caminos terminan en la misma llamada.
    bool pidioVoltear = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && cartaResaltada >= 0)
                     || IsKeyPressed(KEY_ENTER)
                     || IsKeyPressed(KEY_SPACE);

    if(pidioVoltear && cartaResaltada >= 0){

        if(partida->Voltear(cartaResaltada) == Volteo_fallo){
            esperaOcultar = ESPERA_OCULTAR;
        }
    }

    return Escena_juego;
}

//***********************************************
// DIBUJAR
//***********************************************

/**
 * \brief Escribe un tiempo en segundos como minutos:segundos.
 * \param segundos Tiempo transcurrido.
 * \return Cadena lista para dibujar, v&aacute;lida hasta la siguiente llamada.
 */
static const char* comoReloj(float segundos)
{
    int total = (int)segundos;

    return TextFormat("%02d:%02d", total / 60, total % 60);
}

/**
 * \brief Dibuja el marcador de un jugador, resaltado si es su turno.
 *
 * El resaltado es lo &uacute;nico que le dice al jugador que le toca, as&iacute; que no es
 * sutil: recuadro relleno, borde de color y la palabra TU TURNO.
 *
 * \param jugador 0 o 1.
 */
static void dibujarBloqueJugador(int jugador)
{
    Rectangle rec       = bloqueJugador(jugador);
    bool      esSuTurno = (partida->TurnoActual() == jugador);

    // En solitario no hay a quien pasarle el turno, asi que resaltar no informa
    // nada: solo seria ruido. El recuadro se dibuja liso.
    bool marcarTurno = esSuTurno && partida->NumJugadores() > 1;

    if(marcarTurno){
        DrawRectangleRounded(rec, 0.18f, 8, COLOR_BOTON);
        DrawRectangleRoundedLinesEx(rec, 0.18f, 8, 3.0f, COLOR_BOTON_ACTIVO);
    }

    Color colorNombre = marcarTurno ? COLOR_BOTON_ACTIVO : COLOR_TEXTO;

    dibujarDato(nombreDeJugador(configActual, jugador + 1),
                (int)rec.x + 14, (int)rec.y + 7, 20, colorNombre);

    dibujarDato(TextFormat("Pares %d    Puntos %d    Racha %d",
                           partida->ParesDe(jugador),
                           partida->PuntajeDe(jugador),
                           partida->RachaDe(jugador)),
                (int)rec.x + 14, (int)rec.y + 32, 16, COLOR_TEXTO);

    if(marcarTurno){
        const char* aviso = "TU TURNO";
        int         ancho = MeasureText(aviso, 14);

        DrawText(aviso, (int)(rec.x + rec.width - ancho - 14), (int)rec.y + 10,
                 14, COLOR_BOTON_ACTIVO);
    }
}

/**
 * \brief Dibuja todo el marcador de arriba.
 */
static void dibujarMarcador()
{
    dibujarBoton(botonDePausa(), "Pausa", false);
    dibujarBoton(botonDeAyuda(), "?", false);

    dibujarTextoCentrado("GATORAMA", 14, 28, COLOR_TITULO);

    const InfoDificultad& nivel = DIFICULTADES[configActual.dificultad];

    // Baja hasta la altura de los marcadores de jugador, en el hueco que queda
    // entre los dos. Pegada al titulo se veia apretada, y ese hueco estaba vacio.
    dibujarDatoCentrado(TextFormat("%s  %dx%d      Intentos  %d      Tiempo  %s",
                                   nivel.nombre, nivel.filas, nivel.columnas,
                                   partida->Intentos(), comoReloj(partida->Tiempo())),
                        96, 18, COLOR_TENUE);

    for(int i = 0; i < partida->NumJugadores(); i++){
        dibujarBloqueJugador(i);
    }
}

/**
 * \brief Dibuja el mensaje de cierre cuando ya no quedan parejas.
 */
static void dibujarResultado()
{
    if(partida->NumJugadores() == 1){
        dibujarDatoCentrado(TextFormat("Encontraste las %d parejas en %s   -   %d puntos",
                                       partida->ElTablero().NumeroDePares(),
                                       comoReloj(partida->Tiempo()),
                                       partida->PuntajeDe(0)),
                            GetScreenHeight() - 40, 22, COLOR_BOTON_ACTIVO);
        return;
    }

    int ganador = partida->Ganador();

    // Con 5, 9 y 15 parejas el empate es imposible, pero el mensaje existe por si
    // algun dia se agrega un tablero de parejas pares.
    if(ganador < 0){
        dibujarTextoCentrado("Empate", GetScreenHeight() - 40, 22, COLOR_BOTON_ACTIVO);
        return;
    }

    dibujarDatoCentrado(TextFormat("Gano %s con %d parejas",
                                   nombreDeJugador(configActual, ganador + 1),
                                   partida->ParesDe(ganador)),
                        GetScreenHeight() - 40, 22, COLOR_BOTON_ACTIVO);
}

void DibujarJuego()
{
    if(partida == 0) return;

    const Tablero& tablero = partida->ElTablero();
    DisenoTablero  diseno  = disenoActual();

    // El tapete va primero: las cartas se reparten encima de el.
    dibujarFondoTablero();

    for(int fila = 0; fila < tablero.Filas(); fila++){
        for(int columna = 0; columna < tablero.Columnas(); columna++){

            int          indice = fila * tablero.Columnas() + columna;
            const Carta& carta  = tablero.En(fila, columna);
            Rectangle    rec    = rectanguloDeCarta(diseno, fila, columna);

            // El resaltado solo tiene sentido sobre una carta que se puede voltear.
            // Iluminar una ya destapada prometeria algo que no va a pasar.
            bool resaltada = (indice == cartaResaltada)
                          && !carta.EstaVolteada()
                          && !enPausa;

            if(carta.EstaVolteada()){
                dibujarCaraCarta(rec, false, ilustracionDePareja(carta.IdPareja()));
            } else {
                dibujarDorsoCarta(rec, resaltada);
            }
        }
    }

    dibujarMarcador();

    // Aviso de baraja incompleta. Con menos gatos que parejas, dos parejas
    // distintas comparten dibujo y el juego se vuelve imposible de ganar. Vale mas
    // decirlo en pantalla que dejar que alguien lo descubra jugando.
    if(numeroDeIlustraciones() < tablero.NumeroDePares()){
        dibujarDatoCentrado(TextFormat("Faltan ilustraciones: hay %d y se necesitan %d",
                                       numeroDeIlustraciones(), tablero.NumeroDePares()),
                            136, 16, COLOR_TITULO);
    }

    if(partida->Terminada()){
        dibujarResultado();
    } else {
        dibujarTextoCentrado("Clic o flechas y Enter para voltear     ESC para pausar",
                             GetScreenHeight() - 38, 18, COLOR_TENUE);
    }

    // Las ventanas van hasta el final para que queden encima de todo lo demas, y
    // las instrucciones encima de la pausa.
    if(enInstrucciones)  DibujarInstrucciones(configActual.modo);
    else if(enPausa)     DibujarPausa();
}
