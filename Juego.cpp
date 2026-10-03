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
#include "Opciones.hpp"
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

//***********************************************
// ANIMACION DE VOLTEO
//***********************************************

// Cuanto tarda una carta en girar, en segundos. Mas rapido no se alcanza a ver
// como giro; mas lento estorba cuando se voltean dos seguidas.
static const float DURACION_GIRO = 0.3f;

// Tope de cartas que se siguen. El tablero mas grande trae 30.
const int MAX_CARTAS_ANIMADAS = 64;

// Por cada carta: que cara se ve -o hacia cual esta girando- y que tanto lleva del
// giro, de 0 (acaba de empezar) a 1 (quieta). Las reglas no saben nada de esto: la
// carta se voltea al instante en Partida, y aqui solo se nota el cambio y se
// anima. Asi el modelo sigue sin depender de segundos ni de raylib.
static bool  caraMostrada[MAX_CARTAS_ANIMADAS];
static float avanceGiro[MAX_CARTAS_ANIMADAS];

//***********************************************
// RECOLECCION DE PARES
//***********************************************

// Cuando se forma un par, el jugador "se queda" con las cartas: despues de un
// momento a la vista, vuelan encogiendose a una pila en la esquina inferior de su
// lado -la izquierda para el jugador 1 y para el solitario, la derecha para el
// jugador 2- y ahi se quedan apiladas. Igual que el giro, es solo como se ve: para
// las reglas la carta ya estaba emparejada desde el clic.
enum EstadoRecoleccion {
    Rec_enTablero,   ///< En su lugar (sin emparejar, o recien emparejada y esperando)
    Rec_volando,     ///< Camino a la pila
    Rec_enPila       ///< Ya en la pila de su dueno
};

// Cuanto se queda el par a la vista despues de terminar de girar, antes de volar.
// Es lo que deja ver que se acerto; sin esto las cartas desaparecen de golpe.
static const float ESPERA_RECOLECCION = 0.4f;

// Cuanto tarda el vuelo, y cuanto sale despues la segunda carta del par. El
// desfase hace que se vean irse una tras otra, como recogidas con la mano.
static const float DURACION_VUELO = 0.5f;
static const float DESFASE_VUELO  = 0.08f;

// Tamano de una carta en la pila, con la forma de la carta.
static const float ANCHO_EN_PILA = 54.0f;

static EstadoRecoleccion estadoRec[MAX_CARTAS_ANIMADAS];
static float             relojRec[MAX_CARTAS_ANIMADAS];   ///< Espera (si < 0) o avance del vuelo (0 a 1)
static int               duenoRec[MAX_CARTAS_ANIMADAS];   ///< Jugador que encontro el par
static int               lugarRec[MAX_CARTAS_ANIMADAS];   ///< Posicion dentro de la pila de su dueno
static int               tamanoPila[2];                    ///< Cuantas cartas lleva cada pila
static int   cartaResaltada  = -1;   ///< Carta bajo el puntero; -1 si ninguna
static bool  enPausa         = false;
static bool  enInstrucciones = false;
static bool  enOpciones      = false;   ///< La ventana del engrane; tambien congela el reloj
static bool  resultadoGuardado = false;   ///< Para no anotar la misma partida dos veces
static float esperaResultados  = 0.0f;    ///< Lo que falta para cambiar de pantalla


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
    return calcularDiseno(t.Filas(), t.Columnas(), areaDeCartas(), RELACION_CARTA,
                          DIFICULTADES[configActual.dificultad].huecoX);
}

//***********************************************
// ARTE DEL MARCADOR
//***********************************************

// El liston de arriba, con la palabra "GATORAMA" dibujada: con el arte, el titulo
// ya no se escribe con codigo. Va centrado arriba, entre el boton de pausa y el
// engrane, y se reduce a 76 de alto para que sus colas no pisen los recuadros de
// los jugadores, que empiezan en y = 74.
static const char* RUTA_LISTON = "recursos/listonJuego.png";

static Texture2D texturaListon;
static bool      hayListon = false;

// Las placas de los recuadros de jugador, al doble del recuadro (660x124 para uno
// de 330x62). En 1 vs 1, la activa es la del jugador en turno -ya trae "Tu Turno"
// escrito- y la desactivada la del que espera. En solitario va su propia placa,
// del color de la activa pero sin el letrero: no hay a quien pasarle el turno.
static const char* RUTA_TURNO_ACTIVO    = "recursos/tuTurnoActivo.png";
static const char* RUTA_TURNO_DESACTIVO = "recursos/tuTurnoDesactivo.png";
static const char* RUTA_TURNO_SOLITARIO = "recursos/tuTurnoSolitario.png";

static Texture2D texturaTurnoActivo;
static Texture2D texturaTurnoDesactivo;
static Texture2D texturaTurnoSolitario;

static bool hayTurnoActivo    = false;
static bool hayTurnoDesactivo = false;
static bool hayTurnoSolitario = false;

/** \brief Carga una placa que se dibuja reducida, con mipmaps para que no se vea dentada. */
static bool cargarReducida(const char* ruta, Texture2D* destino)
{
    if(!cargarTexturaSiEsta(ruta, destino)) return false;

    GenTextureMipmaps(destino);
    SetTextureFilter(*destino, TEXTURE_FILTER_TRILINEAR);

    return true;
}

void CargarTexturasJuego()
{
    // Las tres se dibujan mas chicas que el archivo.
    hayListon         = cargarReducida(RUTA_LISTON,          &texturaListon);
    hayTurnoActivo    = cargarReducida(RUTA_TURNO_ACTIVO,    &texturaTurnoActivo);
    hayTurnoDesactivo = cargarReducida(RUTA_TURNO_DESACTIVO, &texturaTurnoDesactivo);
    hayTurnoSolitario = cargarReducida(RUTA_TURNO_SOLITARIO, &texturaTurnoSolitario);
}

/**
 * \brief D&oacute;nde va el list&oacute;n: centrado arriba, 76 de alto, con su proporci&oacute;n.
 */
static Rectangle zonaListon()
{
    const float ALTO  = 76.0f;
    float       ancho = ALTO * texturaListon.width / (float)texturaListon.height;

    return rectangulo((GetScreenWidth() - ancho) / 2.0f, 0.0f, ancho, ALTO);
}

void DescargarTexturasJuego()
{
    if(hayListon)         UnloadTexture(texturaListon);
    if(hayTurnoActivo)    UnloadTexture(texturaTurnoActivo);
    if(hayTurnoDesactivo) UnloadTexture(texturaTurnoDesactivo);
    if(hayTurnoSolitario) UnloadTexture(texturaTurnoSolitario);

    hayListon         = false;
    hayTurnoActivo    = false;
    hayTurnoDesactivo = false;
    hayTurnoSolitario = false;
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
 * \brief El bot&oacute;n de opciones -el engrane-, arriba a la derecha. Mismo lugar y
 *        tama&ntilde;o que el del men&uacute;.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonDeOpciones()
{
    return zonaBotonOpciones();
}

/**
 * \brief El recuadro del marcador de un jugador.
 * \param jugador 0 para el de la izquierda, 1 para el de la derecha.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle bloqueJugador(int jugador)
{
    const float ANCHO = 330.0f;

    // 62 de alto para las letras grandes: llega a y = 136, justo antes del tapete.
    if(jugador == 0) return rectangulo(20.0f, 74.0f, ANCHO, 62.0f);

    return rectangulo(GetScreenWidth() - 20.0f - ANCHO, 74.0f, ANCHO, 62.0f);
}

//***********************************************
// ARMAR Y REINICIAR
//***********************************************

/**
 * \brief Deja todas las cartas quietas, mostrando la cara que tienen ahorita.
 *
 * Se llama al repartir: sin esto, la primera vuelta despu&eacute;s de un reinicio ver&iacute;a
 * las cartas del tablero anterior "cambiar" y las animar&iacute;a todas de golpe.
 */
/**
 * \brief D&oacute;nde va la carta n&uacute;mero \p lugar de la pila de un jugador.
 *
 * Cada carta nueva queda un poco m&aacute;s arriba y hacia adentro que la anterior, para
 * que se vea un mont&oacute;n y no una sola carta. Despu&eacute;s de diez ya no se corre: la
 * pila crecer&iacute;a hasta encimarse con el tapete.
 */
static Rectangle zonaEnPila(int jugador, int lugar)
{
    const float ALTO   = ANCHO_EN_PILA / RELACION_CARTA;
    const float MARGEN = 30.0f;

    int   escalon = (lugar < 10) ? lugar : 10;
    float corrido = escalon * 2.5f;

    float x = (jugador == 0) ? MARGEN + corrido
                             : GetScreenWidth() - MARGEN - ANCHO_EN_PILA - corrido;
    float y = GetScreenHeight() - ALTO - 12.0f - corrido;

    return rectangulo(x, y, ANCHO_EN_PILA, ALTO);
}

/**
 * \brief Todas las cartas en su lugar y las pilas vac&iacute;as. Al repartir.
 */
static void reiniciarRecoleccion()
{
    for(int i = 0; i < MAX_CARTAS_ANIMADAS; i++){
        estadoRec[i] = Rec_enTablero;
        relojRec[i]  = 0.0f;
        duenoRec[i]  = 0;
        lugarRec[i]  = 0;
    }

    tamanoPila[0] = 0;
    tamanoPila[1] = 0;
}

/**
 * \brief Nota los pares reci&eacute;n formados y avanza sus esperas y vuelos.
 *
 * Va despu&eacute;s de la l&oacute;gica del fotograma, igual que los giros, para enterarse del
 * par en el mismo fotograma en que se form&oacute;.
 */
static void actualizarRecoleccion()
{
    const Tablero& t = partida->ElTablero();

    // Cuantas cartas del mismo par ya se anotaron en este fotograma: la segunda
    // sale un poco despues que la primera.
    int anotadasAhora = 0;

    for(int f = 0; f < t.Filas(); f++){
        for(int c = 0; c < t.Columnas(); c++){
            int i = f * t.Columnas() + c;

            if(i >= MAX_CARTAS_ANIMADAS) continue;

            // Par nuevo: se anota con su dueno. Al acertar el turno no cambia, asi
            // que el jugador en turno es quien lo encontro.
            if(estadoRec[i] == Rec_enTablero && t.En(f, c).EstaEmparejada() && relojRec[i] == 0.0f){

                int dueno = partida->TurnoActual();
                if(dueno < 0 || dueno > 1) dueno = 0;

                duenoRec[i] = dueno;
                lugarRec[i] = tamanoPila[dueno]++;

                // La espera cuenta el giro que falta, el rato a la vista y el desfase.
                relojRec[i] = -(DURACION_GIRO + ESPERA_RECOLECCION + anotadasAhora * DESFASE_VUELO);
                anotadasAhora++;
                continue;
            }

            if(estadoRec[i] == Rec_enTablero && relojRec[i] < 0.0f){
                relojRec[i] += GetFrameTime();

                if(relojRec[i] >= 0.0f){
                    estadoRec[i] = Rec_volando;
                    relojRec[i]  = 0.0f;
                }
            }
            else if(estadoRec[i] == Rec_volando){
                relojRec[i] += GetFrameTime() / DURACION_VUELO;

                if(relojRec[i] >= 1.0f){
                    estadoRec[i] = Rec_enPila;
                    relojRec[i]  = 1.0f;
                }
            }
        }
    }
}

static void reiniciarGiros()
{
    for(int i = 0; i < MAX_CARTAS_ANIMADAS; i++){
        caraMostrada[i] = false;
        avanceGiro[i]   = 1.0f;
    }

    reiniciarRecoleccion();

    if(partida == 0) return;

    const Tablero& t = partida->ElTablero();

    for(int f = 0; f < t.Filas(); f++){
        for(int c = 0; c < t.Columnas(); c++){
            int i = f * t.Columnas() + c;

            if(i < MAX_CARTAS_ANIMADAS) caraMostrada[i] = t.En(f, c).EstaVolteada();
        }
    }
}

/**
 * \brief Nota las cartas que cambiaron de cara y avanza sus giros.
 *
 * Va en la parte de actualizar, una vez por fotograma.
 */
static void actualizarGiros()
{
    const Tablero& t = partida->ElTablero();

    for(int f = 0; f < t.Filas(); f++){
        for(int c = 0; c < t.Columnas(); c++){
            int i = f * t.Columnas() + c;

            if(i >= MAX_CARTAS_ANIMADAS) continue;

            bool volteada = t.En(f, c).EstaVolteada();

            // Cambio de cara: arranca el giro hacia la nueva.
            if(volteada != caraMostrada[i]){
                caraMostrada[i] = volteada;
                avanceGiro[i]   = 0.0f;
            }

            if(avanceGiro[i] < 1.0f){
                avanceGiro[i] += GetFrameTime() / DURACION_GIRO;

                if(avanceGiro[i] > 1.0f) avanceGiro[i] = 1.0f;
            }
        }
    }
}

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
    reiniciarGiros();
    enPausa           = false;
    enInstrucciones   = false;
    enOpciones        = false;
    resultadoGuardado = false;
    esperaResultados  = 0.0f;
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
    reiniciarGiros();
    enInstrucciones   = false;
    enOpciones        = false;
    resultadoGuardado = false;
    esperaResultados  = 0.0f;
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
 * \brief Si una carta todav&iacute;a se puede elegir: las de un par ya encontrado no.
 *
 * Esas se van volando a la pila y dejan su lugar vac&iacute;o; ni el rat&oacute;n ni las
 * flechas deben poder pararse ah&iacute;, como si hubiera una carta invisible.
 */
static bool cartaElegible(int indice)
{
    const Tablero& t = partida->ElTablero();

    if(indice < 0 || indice >= t.Filas() * t.Columnas()) return false;

    return !t.En(indice / t.Columnas(), indice % t.Columnas()).EstaEmparejada();
}

/**
 * \brief La columna elegible de un rengl&oacute;n m&aacute;s cercana a una columna dada.
 * \return La columna, o -1 si en ese rengl&oacute;n ya no queda ninguna.
 */
static int columnaMasCercana(int fila, int columna)
{
    const Tablero& t     = partida->ElTablero();
    int            mejor = -1;

    for(int c = 0; c < t.Columnas(); c++){
        if(!cartaElegible(fila * t.Columnas() + c)) continue;

        int distancia = (c > columna) ? c - columna : columna - c;
        int actual    = (mejor > columna) ? mejor - columna : columna - mejor;

        if(mejor < 0 || distancia < actual) mejor = c;
    }

    return mejor;
}

/**
 * \brief La carta elegible m&aacute;s cercana a un lugar, en todo el tablero.
 * \return Su &iacute;ndice, o -1 si ya no queda ninguna.
 */
static int cartaMasCercana(int fila, int columna)
{
    const Tablero& t     = partida->ElTablero();
    int            mejor = -1;
    int            menor = 0;

    for(int f = 0; f < t.Filas(); f++){
        for(int c = 0; c < t.Columnas(); c++){
            if(!cartaElegible(f * t.Columnas() + c)) continue;

            int distancia = ((f > fila) ? f - fila : fila - f) + ((c > columna) ? c - columna : columna - c);

            if(mejor < 0 || distancia < menor){
                mejor = f * t.Columnas() + c;
                menor = distancia;
            }
        }
    }

    return mejor;
}

/**
 * \brief Mueve la carta resaltada con las flechas (o W A S D), salt&aacute;ndose los
 *        lugares vac&iacute;os.
 *
 * Da la vuelta en los bordes. A los lados, se brinca a la siguiente carta que
 * quede en el rengl&oacute;n. Arriba y abajo, al siguiente rengl&oacute;n que tenga alguna,
 * en la columna m&aacute;s cercana: as&iacute; se llega a cualquier carta aunque el tablero
 * est&eacute; lleno de huecos.
 */
static void moverConFlechas()
{
    int pasoX = 0;
    int pasoY = 0;

    if(teclaDerecha())   pasoX =  1;
    if(teclaIzquierda()) pasoX = -1;
    if(teclaAbajo())     pasoY =  1;
    if(teclaArriba())    pasoY = -1;

    if(pasoX == 0 && pasoY == 0) return;

    const Tablero& t        = partida->ElTablero();
    int            filas    = t.Filas();
    int            columnas = t.Columnas();

    // La primera flecha estrena el cursor en la primera carta que quede.
    if(cartaResaltada < 0){
        cartaResaltada = cartaMasCercana(0, 0);
        return;
    }

    int fila    = cartaResaltada / columnas;
    int columna = cartaResaltada % columnas;

    if(pasoX != 0){
        // La siguiente del renglon, dando la vuelta. Se suma el total antes del
        // residuo porque en C++ el residuo de un negativo es negativo.
        for(int k = 1; k < columnas; k++){
            int c = ((columna + pasoX * k) % columnas + columnas) % columnas;

            if(cartaElegible(fila * columnas + c)){
                cartaResaltada = fila * columnas + c;
                return;
            }
        }
    } else {
        // El siguiente renglon que tenga alguna carta, en la columna mas cercana.
        for(int k = 1; k < filas; k++){
            int f = ((fila + pasoY * k) % filas + filas) % filas;
            int c = columnaMasCercana(f, columna);

            if(c >= 0){
                cartaResaltada = f * columnas + c;
                return;
            }
        }
    }

    // No hubo a donde moverse en esa direccion. Si el cursor quedo en un hueco -su
    // carta se acaba de ir a la pila-, se pasa a la mas cercana que quede.
    if(!cartaElegible(cartaResaltada)) cartaResaltada = cartaMasCercana(fila, columna);
}

/**
 * \brief Todo lo que pasa en un fotograma de juego, menos la animaci&oacute;n de volteo.
 *
 * Tiene muchas salidas tempranas -pausa, opciones, fin de partida-, por eso la
 * animaci&oacute;n se atiende afuera, en ActualizarJuego, despu&eacute;s de esto.
 */
static Escena_Estado actualizarPartida()
{
    // Las instrucciones van encima de la pausa, asi que se atienden primero: si
    // estan abiertas, son ellas las que se quedan con el ESC. Solo se abren desde
    // la pausa, asi que al cerrarlas se vuelve a ella.
    if(enInstrucciones){
        if(ActualizarInstrucciones()) enInstrucciones = false;

        return Escena_juego;
    }

    // La ventana de opciones tambien se queda con toda la entrada, y como se
    // regresa antes de CorrerReloj, el tiempo no corre mientras esta abierta:
    // subirle al volumen no deberia costarle segundos al jugador.
    if(enOpciones){
        if(ActualizarOpciones()) enOpciones = false;

        return Escena_juego;
    }

    if(enPausa){
        AccionPausa accion = ActualizarPausa();

        if(accion == Pausa_continuar)           enPausa = false;
        else if(accion == Pausa_reiniciar)      ReiniciarPartida();
        else if(accion == Pausa_instrucciones)  enInstrucciones = true;
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

    if(botonClicado(botonDeOpciones())){
        enOpciones = true;
        PrepararOpciones();
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

        // Un hueco no se resalta: ahi ya no hay carta.
        if(!cartaElegible(cartaResaltada)) cartaResaltada = -1;
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

Escena_Estado ActualizarJuego()
{
    // Nadie deberia llegar aqui sin partida, pero si pasa es mejor regresar al
    // menu que desreferenciar un puntero nulo.
    if(partida == 0) return Escena_menu;

    Escena_Estado siguiente = actualizarPartida();

    // Los giros se revisan DESPUES de todo lo que puede voltear o tapar una carta.
    // Si fuera antes, en el fotograma del clic la carta ya estaria volteada en las
    // reglas sin que el giro lo supiera, y se veria la cara completa un instante
    // antes de empezar a girar.
    //
    // Siguen aunque haya una ventana abierta: es solo como se ve, y una carta
    // congelada a medio girar debajo de la pausa se veria descompuesta.
    actualizarGiros();
    actualizarRecoleccion();

    return siguiente;
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

    const char* nombre = nombreDeJugador(configActual, jugador + 1);
    const char* marcas = TextFormat("Pares %d    Puntos %d    Racha %d",
                                    partida->ParesDe(jugador),
                                    partida->PuntajeDe(jugador),
                                    partida->RachaDe(jugador));

    int x = (int)rec.x + 14;

    if(hayTurnoActivo && hayTurnoDesactivo){

        // Con el arte, cada recuadro va sobre su placa. "Tu Turno" ya viene escrito
        // en la activa, arriba a la derecha.
        bool      solo    = (partida->NumJugadores() == 1);
        Texture2D placa   = marcarTurno ? texturaTurnoActivo : texturaTurnoDesactivo;

        if(solo && hayTurnoSolitario) placa = texturaTurnoSolitario;

        // En solitario el nombre va en morado, como el del que tiene el turno: la
        // placa amarilla es la misma familia.
        bool nombreMorado = marcarTurno || (solo && hayTurnoSolitario);
        Rectangle origen  = { 0.0f, 0.0f, (float)placa.width, (float)placa.height };
        Vector2   desfase = { 0.0f, 0.0f };

        DrawTexturePro(placa, origen, rec, desfase, 0.0f, WHITE);

        dibujarDato(nombre, x, (int)rec.y + 5,  26, nombreMorado ? COLOR_BOTON_ACTIVO : COLOR_TEXTO);
        dibujarDato(marcas, x, (int)rec.y + 36, 20, COLOR_TEXTO);
        return;
    }

    if(marcarTurno){
        DrawRectangleRounded(rec, 0.18f, 8, COLOR_BOTON);
        DrawRectangleRoundedLinesEx(rec, 0.18f, 8, 3.0f, COLOR_BOTON_ACTIVO);
    }

    if(marcarTurno){
        // Sobre el recuadro claro, los colores de siempre.
        dibujarDato(nombre, x, (int)rec.y + 5,  26, COLOR_BOTON_ACTIVO);
        dibujarDato(marcas, x, (int)rec.y + 36, 20, COLOR_TEXTO);
    } else {
        // Sin recuadro, el texto va directo sobre la madera.
        dibujarDatoSobreFondo(nombre, x, (int)rec.y + 5,  26, COLOR_FONDO_TEXTO);
        dibujarDatoSobreFondo(marcas, x, (int)rec.y + 36, 20, COLOR_FONDO_TENUE);
    }

    if(marcarTurno){
        const char* aviso = "TU TURNO";
        int         ancho = MeasureText(aviso, 16);

        DrawText(aviso, (int)(rec.x + rec.width - ancho - 14), (int)rec.y + 10,
                 16, COLOR_BOTON_ACTIVO);
    }
}

/**
 * \brief Dibuja todo el marcador de arriba.
 */
static void dibujarMarcador()
{
    // El liston va primero: todo lo demas del marcador se dibuja encima.
    if(hayListon){
        Rectangle origen  = { 0.0f, 0.0f, (float)texturaListon.width, (float)texturaListon.height };
        Vector2   desfase = { 0.0f, 0.0f };

        DrawTexturePro(texturaListon, origen, zonaListon(), desfase, 0.0f, WHITE);
    }

    dibujarBotonPausa(botonDePausa());
    dibujarBotonOpciones(botonDeOpciones());

    if(!hayListon) dibujarTextoCentradoSobreFondo("GATORAMA", 12, 38, COLOR_FONDO_TITULO);

    const InfoDificultad& nivel = DIFICULTADES[configActual.dificultad];

    // Baja hasta la altura de los marcadores de jugador, en el hueco que queda
    // entre los dos. Pegada al titulo se veia apretada, y ese hueco estaba vacio.
    // Blanco con contorno negro: lo que mas resalta sobre la madera.
    dibujarDatoCentradoConContorno(TextFormat("%s  %dx%d      Intentos  %d      Tiempo  %s",
                                              nivel.nombre, nivel.filas, nivel.columnas,
                                              partida->Intentos(), comoReloj(partida->Tiempo())),
                                   94, 24, WHITE, BLACK);

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
        dibujarDatoCentradoSobreFondo(TextFormat("Encontraste las %d parejas en %s   -   %d puntos",
                                                 partida->ElTablero().NumeroDePares(),
                                                 comoReloj(partida->Tiempo()),
                                                 partida->PuntajeDe(0)),
                                      GetScreenHeight() - 40, 22, COLOR_FONDO_RESALTE);
        return;
    }

    int ganador = partida->Ganador();

    // Con 5, 9 y 15 parejas el empate es imposible, pero el mensaje existe por si
    // algun dia se agrega un tablero de parejas pares.
    if(ganador < 0){
        dibujarTextoCentradoSobreFondo("Empate", GetScreenHeight() - 40, 22, COLOR_FONDO_RESALTE);
        return;
    }

    dibujarDatoCentradoSobreFondo(TextFormat("Gano %s con %d parejas",
                                             nombreDeJugador(configActual, ganador + 1),
                                             partida->ParesDe(ganador)),
                                  GetScreenHeight() - 40, 22, COLOR_FONDO_RESALTE);
}

/**
 * \brief Las pilas de cada jugador y las cartas que van volando hacia ellas.
 *
 * Primero las pilas, en orden, para que la &uacute;ltima recogida quede arriba; despu&eacute;s
 * las que vuelan, encima de todo, para que no las tape ni el tapete ni la pila.
 */
static void dibujarPilasYVuelos(const Tablero& tablero, const DisenoTablero& diseno)
{
    int total = tablero.Filas() * tablero.Columnas();
    if(total > MAX_CARTAS_ANIMADAS) total = MAX_CARTAS_ANIMADAS;

    for(int jugador = 0; jugador < 2; jugador++){
        for(int lugar = 0; lugar < tamanoPila[jugador]; lugar++){
            for(int i = 0; i < total; i++){
                if(estadoRec[i] != Rec_enPila || duenoRec[i] != jugador || lugarRec[i] != lugar) continue;

                const Carta& carta = tablero.En(i / tablero.Columnas(), i % tablero.Columnas());

                dibujarCaraCarta(zonaEnPila(jugador, lugar), false, ilustracionDePareja(carta.IdPareja()));
            }
        }
    }

    for(int i = 0; i < total; i++){
        if(estadoRec[i] != Rec_volando) continue;

        int          fila    = i / tablero.Columnas();
        int          columna = i % tablero.Columnas();
        const Carta& carta   = tablero.En(fila, columna);

        Rectangle desde = rectanguloDeCarta(diseno, fila, columna);
        Rectangle hasta = zonaEnPila(duenoRec[i], lugarRec[i]);

        // Arranca despacio, acelera y frena al llegar: se ve como recogida con la
        // mano y no como empujada a velocidad fija.
        float a = relojRec[i];
        float t = a * a * (3.0f - 2.0f * a);

        Rectangle ahora;
        ahora.x      = desde.x      + (hasta.x      - desde.x)      * t;
        ahora.y      = desde.y      + (hasta.y      - desde.y)      * t;
        ahora.width  = desde.width  + (hasta.width  - desde.width)  * t;
        ahora.height = desde.height + (hasta.height - desde.height) * t;

        dibujarCaraCarta(ahora, false, ilustracionDePareja(carta.IdPareja()));
    }
}

void DibujarJuego()
{
    if(partida == 0) return;

    const Tablero& tablero = partida->ElTablero();
    DisenoTablero  diseno  = disenoActual();

    // El tapete va primero: las cartas se reparten encima de el.
    dibujarFondoTablero();

    // Donde va el anillo del cursor. Se anota aqui y se dibuja hasta despues del
    // ciclo: si se pintara dentro, la carta siguiente lo taparia, porque el anillo
    // sobresale hacia el hueco que comparten.
    Rectangle recCursor       = { 0.0f, 0.0f, 0.0f, 0.0f };
    bool      hayCursor       = false;
    bool      cursorVolteable = false;

    for(int fila = 0; fila < tablero.Filas(); fila++){
        for(int columna = 0; columna < tablero.Columnas(); columna++){

            int          indice = fila * tablero.Columnas() + columna;
            const Carta& carta  = tablero.En(fila, columna);
            Rectangle    rec    = rectanguloDeCarta(diseno, fila, columna);

            bool esElCursor = (indice == cartaResaltada) && !enPausa && !enOpciones;

            // La que ya se fue -volando o en la pila- deja su lugar vacio, sin
            // anillo: ahi ya no hay nada que elegir.
            if(indice < MAX_CARTAS_ANIMADAS && estadoRec[indice] != Rec_enTablero) continue;

            // El giro: en la primera mitad se ve la cara de antes y la carta se
            // angosta hasta quedar de canto; en la segunda ya se ve la nueva y se
            // vuelve a abrir. Se angosta desde el centro, para que gire sobre su eje
            // y no sobre un borde.
            bool  mostrarCara = carta.EstaVolteada();
            float escala      = 1.0f;

            if(indice < MAX_CARTAS_ANIMADAS && avanceGiro[indice] < 1.0f){
                float a = avanceGiro[indice];

                if(a < 0.5f){
                    mostrarCara = !caraMostrada[indice];   // todavia la cara de antes
                    escala      = 1.0f - a * 2.0f;
                } else {
                    escala      = a * 2.0f - 1.0f;
                }
            }

            Rectangle visible = rec;
            visible.width = rec.width * escala;
            visible.x     = rec.x + (rec.width - visible.width) / 2.0f;

            // De canto no hay nada que dibujar, y un rectangulo de un pixel con
            // esquinas redondeadas sale como una raya rara.
            if(visible.width >= 2.0f){

                // Aclarar la carta solo tiene sentido si se puede voltear: iluminar
                // una ya destapada prometeria algo que no va a pasar. Marcar donde
                // esta el cursor es otra cosa, y eso si hay que verlo siempre; de eso
                // se encarga el anillo de abajo.
                if(mostrarCara){
                    dibujarCaraCarta(visible, false, ilustracionDePareja(carta.IdPareja()));
                } else {
                    dibujarDorsoCarta(visible, esElCursor && escala >= 1.0f);
                }
            }

            if(esElCursor){
                recCursor       = rec;
                cursorVolteable = !carta.EstaVolteada();
                hayCursor       = true;
            }
        }
    }

    if(hayCursor) dibujarCursorCarta(recCursor, cursorVolteable);

    dibujarPilasYVuelos(tablero, diseno);

    dibujarMarcador();

    // Aviso de baraja incompleta. Con menos gatos que parejas, dos parejas
    // distintas comparten dibujo y el juego se vuelve imposible de ganar. Vale mas
    // decirlo en pantalla que dejar que alguien lo descubra jugando.
    if(numeroDeIlustraciones() < tablero.NumeroDePares()){
        dibujarDatoCentradoSobreFondo(TextFormat("Faltan ilustraciones: hay %d y se necesitan %d",
                                                 numeroDeIlustraciones(), tablero.NumeroDePares()),
                                      136, 16, COLOR_FONDO_RESALTE);
    }

    // Abajo solo sale el mensaje del final; la linea de ayuda ya no va.
    if(partida->Terminada()) dibujarResultado();

    // Las ventanas van hasta el final para que queden encima de todo lo demas, y
    // las instrucciones encima de la pausa.
    if(enInstrucciones)  DibujarInstrucciones();
    else if(enPausa)     DibujarPausa();
    else if(enOpciones)  DibujarOpciones();
}
