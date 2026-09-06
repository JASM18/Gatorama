/**
 * \file Menu.cpp
 * \brief Implementaci&oacute;n del men&uacute; principal.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#include "raylib.h"

#include "Menu.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// DATOS DEL MENU
//***********************************************

const int NUM_OPCIONES = 4;

// Las etiquetas y el destino de cada opcion se guardan en dos arreglos paralelos:
// ETIQUETAS[i] lleva a DESTINOS[i]. Se hace asi para que agregar una opcion al
// menu sea agregar dos lineas de datos y nada mas. La alternativa (un switch con
// un case por opcion) obliga a tocar codigo cada vez, y es donde se cuelan los
// errores cuando el menu crece.

static const char* ETIQUETAS[NUM_OPCIONES] = {
    "Jugar",
    "Mejores puntajes",
    "Creditos",
    "Salir"
};

// TEMPORAL: "Jugar" entra directo a la partida. Lo correcto es que pase antes por
// la pantalla de configuracion -elegir dificultad y si son uno o dos jugadores-,
// pero esa pantalla todavia no existe y mandar ahi dejaba el juego inalcanzable.
// Cuando Escena_configuracion se implemente, este renglon vuelve a ser
// Escena_configuracion y ella decide cuando pasar a Escena_juego.
static const Escena_Estado DESTINOS[NUM_OPCIONES] = {
    Escena_juego,
    Escena_puntajes,
    Escena_creditos,
    Escena_salir
};

// 'static' a nivel de archivo significa que esta variable solo existe dentro de
// Menu.cpp: ningun otro archivo la puede ver ni modificar. Eso es lo que hace que
// esto sea un modulo y no una variable global suelta. main.cpp no sabe -ni tiene
// por que saber- cual opcion esta resaltada.
static int opcionSeleccionada = 0;

//***********************************************
// MENU PRINCIPAL
//***********************************************

Escena_Estado ActualizarMenu()
{
    // IsKeyPressed es verdadero SOLO en el fotograma exacto en que la tecla baja.
    // Con IsKeyDown recorreriamos las 4 opciones antes de soltar la tecla, porque
    // el bucle da 60 vueltas por segundo y un toque normal dura varias.
    if(IsKeyPressed(KEY_DOWN)) opcionSeleccionada++;
    if(IsKeyPressed(KEY_UP))   opcionSeleccionada--;

    // Menu circular: de la ultima opcion se pasa a la primera y al reves.
    // Se suma NUM_OPCIONES antes del modulo porque en C++ el residuo de un
    // negativo es negativo: -1 % 4 da -1, no 3. Sumar primero lo evita.
    opcionSeleccionada = (opcionSeleccionada + NUM_OPCIONES) % NUM_OPCIONES;

    if(IsKeyPressed(KEY_ENTER)){
        return DESTINOS[opcionSeleccionada];
    }

    // Nadie ha elegido nada: nos quedamos donde estamos.
    return Escena_menu;
}

void DibujarMenu()
{
    dibujarTextoCentrado("GATORAMA", 110, 80, COLOR_TITULO);

    for(int i = 0; i < NUM_OPCIONES; i++){

        // Lo unico que distingue a la opcion resaltada es como se dibuja. No hay
        // que guardar ningun estado extra ni avisarle a nadie: se decide aqui,
        // en el momento de dibujar. Eso es el modo inmediato.
        bool seleccionada = (i == opcionSeleccionada);

        Color color   = seleccionada ? COLOR_SELECCION : COLOR_TEXTO;
        int   tamano  = seleccionada ? 38 : 32;
        int   y       = 300 + i * 65;

        // TextFormat es el sprintf de raylib: arma una cadena con formato.
        const char* etiqueta = seleccionada
                             ? TextFormat("> %s <", ETIQUETAS[i])
                             : ETIQUETAS[i];

        dibujarTextoCentrado(etiqueta, y, tamano, color);
    }

    dibujarTextoCentrado("Flechas para moverte     Enter para elegir", 640, 20, COLOR_TENUE);
}
