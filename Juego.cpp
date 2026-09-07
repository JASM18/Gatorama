/**
 * \file Juego.cpp
 * \brief Implementaci&oacute;n de la pantalla de la partida.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Juego.hpp"
#include "Tablero.hpp"
#include "VistaTablero.hpp"
#include "Dificultad.hpp"
#include "Pausa.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ESTADO DE LA PARTIDA
//***********************************************

// Cuanto se queda un par equivocado a la vista antes de taparse. Menos de medio
// segundo no alcanza para memorizarlo y mas de un segundo se siente lento.
static const float ESPERA_OCULTAR = 0.9f;

static Tablero*     tablero = 0;      ///< El modelo; se reserva al iniciar partida
static ConfigPartida configActual;    ///< Con que se armo, para poder reiniciar

// Que cartas estan destapadas esperando comparacion. -1 significa "ninguna", igual
// que en indiceCartaEnPunto: cero es una carta valida y confundirlos voltearia la
// esquina del tablero.
static int   primera        = -1;
static int   segunda        = -1;
static float esperaOcultar  = 0.0f;

static int   intentos       = 0;
static float tiempo         = 0.0f;
static int   cartaResaltada = -1;
static bool  enPausa        = false;

/**
 * \brief El acomodo de las cartas para el tablero actual.
 *
 * Actualizar y dibujar lo recalculan por su cuenta. Es una decena de
 * multiplicaciones: sale m&aacute;s barato que guardarlo y arriesgarse a detectar el clic
 * en un lugar distinto del que se dibuj&oacute;.
 */
static DisenoTablero disenoActual()
{
    return calcularDiseno(tablero->Filas(), tablero->Columnas(),
                          areaDelTablero(), RELACION_CARTA);
}

/**
 * \brief El bot&oacute;n de pausa, arriba a la izquierda.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonDePausa()
{
    return rectangulo(20.0f, 18.0f, 120.0f, 40.0f);
}

//***********************************************
// ARMAR Y REINICIAR
//***********************************************

/**
 * \brief Deja el estado de la partida en cero, sin tocar el tablero.
 */
static void limpiarEstado()
{
    primera        = -1;
    segunda        = -1;
    esperaOcultar  = 0.0f;
    intentos       = 0;
    tiempo         = 0.0f;
    cartaResaltada = -1;
    enPausa        = false;
}

void IniciarPartida(const ConfigPartida& config)
{
    configActual = config;

    // Se libera la partida anterior antes de armar la nueva. Sin esto, cada vez
    // que alguien saliera al menu y volviera a jugar se quedaria un tablero
    // reservado sin que nadie lo pueda alcanzar: una fuga de las clasicas.
    delete tablero;
    tablero = 0;

    const InfoDificultad& nivel = DIFICULTADES[config.dificultad];

    try {
        tablero = new Tablero(nivel.filas, nivel.columnas);
    }
    catch(const Tablero::TableroInvalido&) {
        // Solo puede pasar si alguien edita la tabla de dificultades y deja un
        // tablero de total impar. No se puede jugar eso, pero tampoco se va a
        // cerrar el juego enfrente de un nino: se cae de pie al tablero mas chico.
        tablero = new Tablero(2, 5);
    }

    limpiarEstado();
}

void ReiniciarPartida()
{
    if(tablero == 0) return;

    // Repartir vuelve a barajar sobre el mismo tablero, sin reservar de nuevo.
    tablero->Repartir();
    limpiarEstado();
}

void LiberarPartida()
{
    delete tablero;
    tablero = 0;
}

//***********************************************
// ACTUALIZAR
//***********************************************

/**
 * \brief Atiende el clic sobre una carta y compara cuando ya hay dos destapadas.
 */
static void intentarVoltear()
{
    if(cartaResaltada < 0) return;

    Carta& elegida = tablero->EnIndice(cartaResaltada);

    // Volver a picarle a una carta ya destapada no hace nada. Sin esta guarda se
    // podria "emparejar" una carta consigo misma picandole dos veces seguidas.
    if(elegida.EstaVolteada() || elegida.EstaEmparejada()) return;

    elegida.Voltear();

    if(primera == -1){
        primera = cartaResaltada;
        return;
    }

    segunda = cartaResaltada;
    intentos++;

    if(tablero->EnIndice(primera).HacePareja(elegida)){
        // Acierto: se marcan de inmediato y quedan descubiertas para siempre.
        tablero->EnIndice(primera).Emparejar();
        tablero->EnIndice(segunda).Emparejar();

        primera = -1;
        segunda = -1;
    } else {
        // Error: se quedan a la vista un momento para poder memorizarlas, y el
        // temporizador de abajo las tapa. Taparlas de inmediato haria imposible
        // el juego.
        esperaOcultar = ESPERA_OCULTAR;
    }
}

Escena_Estado ActualizarJuego()
{
    // Nadie deberia llegar aqui sin partida, pero si pasa es mejor regresar al
    // menu que desreferenciar un puntero nulo.
    if(tablero == 0) return Escena_menu;

    if(enPausa){
        AccionPausa accion = ActualizarPausa();

        if(accion == Pausa_continuar)      enPausa = false;
        else if(accion == Pausa_reiniciar) ReiniciarPartida();
        else if(accion == Pausa_menu)      return Escena_menu;

        // Se regresa aqui mismo: nada de lo de abajo corre. Eso es lo que congela
        // el reloj y lo que hace que los clics del panel no volteen cartas.
        return Escena_juego;
    }

    if(IsKeyPressed(KEY_ESCAPE) || botonClicado(botonDePausa())){
        enPausa = true;
        return Escena_juego;
    }

    // Con el tablero resuelto el reloj se detiene: el tiempo final ya es el bueno.
    if(tablero->EstaResuelto()) return Escena_juego;

    // GetFrameTime da los segundos que duro el fotograma anterior. Sumarlo es lo
    // que hace que el reloj mida tiempo real y no fotogramas: en una maquina lenta
    // el juego va igual de rapido en segundos.
    tiempo += GetFrameTime();

    cartaResaltada = indiceCartaEnPunto(disenoActual(), GetMousePosition());

    if(esperaOcultar > 0.0f){
        esperaOcultar -= GetFrameTime();

        if(esperaOcultar <= 0.0f){
            tablero->EnIndice(primera).Ocultar();
            tablero->EnIndice(segunda).Ocultar();

            primera = -1;
            segunda = -1;
        }

        // Mientras se resuelve el par no se aceptan clics. Si no, se podria
        // destapar una tercera carta y quedarian tres a la vista.
        return Escena_juego;
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) intentarVoltear();

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
 * \brief Dibuja el marcador de arriba.
 */
static void dibujarMarcador()
{
    dibujarBoton(botonDePausa(), "Pausa", false);

    dibujarTextoCentrado("GATORAMA", 16, 30, COLOR_TITULO);

    const InfoDificultad& nivel = DIFICULTADES[configActual.dificultad];

    dibujarTextoCentrado(TextFormat("%s   %dx%d      Pares  %d / %d      Intentos  %d      Tiempo  %s",
                                    nivel.nombre, nivel.filas, nivel.columnas,
                                    tablero->ParesEncontrados(), tablero->NumeroDePares(),
                                    intentos, comoReloj(tiempo)),
                         62, 20, COLOR_TEXTO);

    // Los nombres se dibujan en las esquinas, uno por lado, como en el boceto. El
    // segundo solo aparece en multijugador. Todavia no hay marcador por jugador
    // porque los turnos son una regla y las reglas no estan cerradas.
    DrawText(nombreDeJugador(configActual, 1), 20, 96, 20, COLOR_SELECCION);

    if(configActual.modo == Modo_multijugador){
        const char* dos = nombreDeJugador(configActual, 2);

        DrawText(dos, GetScreenWidth() - 20 - MeasureText(dos, 20), 96, 20, COLOR_SELECCION);
    }
}

void DibujarJuego()
{
    if(tablero == 0) return;

    DisenoTablero diseno = disenoActual();

    for(int fila = 0; fila < tablero->Filas(); fila++){
        for(int columna = 0; columna < tablero->Columnas(); columna++){

            int          indice = fila * tablero->Columnas() + columna;
            const Carta& carta  = tablero->En(fila, columna);
            Rectangle    rec    = rectanguloDeCarta(diseno, fila, columna);

            // El resaltado solo tiene sentido sobre una carta que se puede voltear.
            // Iluminar una ya emparejada prometeria algo que no va a pasar.
            bool resaltada = (indice == cartaResaltada)
                          && !carta.EstaVolteada()
                          && !enPausa;

            if(carta.EstaVolteada()){
                dibujarCaraCarta(rec, false, carta.IdPareja());
            } else {
                dibujarDorsoCarta(rec, resaltada);
            }
        }
    }

    dibujarMarcador();

    if(tablero->EstaResuelto()){
        dibujarTextoCentrado(TextFormat("Encontraste las %d parejas en %s",
                                        tablero->NumeroDePares(), comoReloj(tiempo)),
                             GetScreenHeight() - 40, 24, COLOR_BOTON_ACTIVO);
    } else {
        dibujarTextoCentrado("Clic para voltear una carta     ESC para pausar",
                             GetScreenHeight() - 38, 18, COLOR_TENUE);
    }

    // La pausa va hasta el final para que quede encima de todo lo demas.
    if(enPausa) DibujarPausa();
}
