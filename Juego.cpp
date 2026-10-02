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
 * \brief Mueve la carta resaltada con las flechas del teclado.
 *
 * Da la vuelta en los bordes: a la derecha de la &uacute;ltima columna est&aacute; la primera,
 * y abajo del &uacute;ltimo rengl&oacute;n est&aacute; el primero, en la misma columna o fila. As&iacute; se
 * llega a cualquier carta sin tener que recorrer el tablero entero.
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

    // Se suma el total antes del residuo porque en C++ el residuo de un negativo
    // es negativo: -1 % 6 da -1, no 5.
    fila    = (fila    + t.Filas()) % t.Filas();
    columna = (columna + columnas)  % columnas;

    cartaResaltada = fila * columnas + columna;
}

Escena_Estado ActualizarJuego()
{
    // Nadie deberia llegar aqui sin partida, pero si pasa es mejor regresar al
    // menu que desreferenciar un puntero nulo.
    if(partida == 0) return Escena_menu;

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
    dibujarDatoCentradoSobreFondo(TextFormat("%s  %dx%d      Intentos  %d      Tiempo  %s",
                                             nivel.nombre, nivel.filas, nivel.columnas,
                                             partida->Intentos(), comoReloj(partida->Tiempo())),
                                  94, 24, COLOR_FONDO_TEXTO);

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

            // Aclarar la carta solo tiene sentido si se puede voltear: iluminar una
            // ya destapada prometeria algo que no va a pasar. Marcar donde esta el
            // cursor es otra cosa, y eso si hay que verlo siempre; de eso se encarga
            // el anillo de abajo.
            if(carta.EstaVolteada()){
                dibujarCaraCarta(rec, false, ilustracionDePareja(carta.IdPareja()));
            } else {
                dibujarDorsoCarta(rec, esElCursor);
            }

            if(esElCursor){
                recCursor       = rec;
                cursorVolteable = !carta.EstaVolteada();
                hayCursor       = true;
            }
        }
    }

    if(hayCursor) dibujarCursorCarta(recCursor, cursorVolteable);

    dibujarMarcador();

    // Aviso de baraja incompleta. Con menos gatos que parejas, dos parejas
    // distintas comparten dibujo y el juego se vuelve imposible de ganar. Vale mas
    // decirlo en pantalla que dejar que alguien lo descubra jugando.
    if(numeroDeIlustraciones() < tablero.NumeroDePares()){
        dibujarDatoCentradoSobreFondo(TextFormat("Faltan ilustraciones: hay %d y se necesitan %d",
                                                 numeroDeIlustraciones(), tablero.NumeroDePares()),
                                      136, 16, COLOR_FONDO_RESALTE);
    }

    if(partida->Terminada()){
        dibujarResultado();
    } else {
        dibujarTextoCentradoSobreFondo("Clic o flechas y Enter para voltear     ESC para pausar",
                                       GetScreenHeight() - 38, 18, COLOR_FONDO_TENUE);
    }

    // Las ventanas van hasta el final para que queden encima de todo lo demas, y
    // las instrucciones encima de la pausa.
    if(enInstrucciones)  DibujarInstrucciones();
    else if(enPausa)     DibujarPausa();
    else if(enOpciones)  DibujarOpciones();
}
