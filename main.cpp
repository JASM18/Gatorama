/**
 * \file main.cpp
 * \brief Archivo principal de Gatorama
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#include "raylib.h"
#include <iostream>

#include "Menu.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"
#include "VistaTablero.hpp"

// ***********************************************
// CONFIGURACION DE LA VENTANA
// ***********************************************

// Mas adelante estos valores saldran del archivo de tema (tema.json), para que el tamano de la ventana se pueda ajustar sin recompilar.

const int PantallaAncho = 1280;
const int PantallaAlto = 720;
const int FPS = 60;

using namespace std;

// ***********************************************

int main()
{
    // InitWindow crea la ventana
    InitWindow(PantallaAncho, PantallaAlto, "Gatorama");
    SetTargetFPS(FPS); // Se establece el juego a 60 fps

    // Por defecto WindowShouldClose() tambien es verdadero al presionar ESC, no
    // solo al cerrar la ventana. Como nosotros queremos usar ESC para regresar al
    // menu, hay que quitarle ese trabajo. Sin esta linea, ESC cierra el juego.
    SetExitKey(KEY_NULL);

    // Las imagenes se cargan aqui, ya con la ventana abierta: una textura vive en
    // la memoria de la tarjeta de video, y esa memoria no existe antes de que
    // InitWindow cree el contexto de OpenGL.
    cargarTexturasTablero();

    Escena_Estado escenaActual = Escena_menu;

    // Loop principal del juego
    // Se sale por la X de la ventana o cuando el menu pide Escena_salir.
    while(!WindowShouldClose() && escenaActual != Escena_salir){

        // -------------------------------------------
        // ACTUALIZAR: leer entrada y cambiar el estado
        // -------------------------------------------
        switch(escenaActual)
        {
            case Escena_menu:
                // El menu se encarga de su propia navegacion y nos devuelve a
                // donde hay que ir. Si nadie eligio nada, devuelve Escena_menu
                // y aqui no cambia nada.
                escenaActual = ActualizarMenu();
            break;

            case Escena_juego:
                // Todavia no hay partida: lo que vive aqui es el banco de pruebas
                // del tablero, que sirve para medir a que tamano quedan las cartas
                // en cada dificultad. Cuando exista el modelo, esta linea cambia
                // por la partida de verdad y el resto de main.cpp no se entera.
                escenaActual = ActualizarPruebaTablero();
            break;

            // Estas pantallas todavia no existen, asi que por ahora se comportan
            // igual: ESC regresa al menu. Conforme cada una se implemente, saldra
            // de esta lista y tendra su propio case.
            case Escena_configuracion:
            case Escena_puntajes:
            case Escena_creditos:

                if(IsKeyPressed(KEY_ESCAPE)){
                    escenaActual = Escena_menu;
                }

            break;

            default: break;
        }

        // -------------------------------------------
        // DIBUJAR: pintar el estado, sin modificarlo
        // -------------------------------------------
        // Todo lo que se dibuja va entre BeginDrawing y EndDrawing
        BeginDrawing();
            ClearBackground(COLOR_FONDO);

            switch(escenaActual)
            {
                case Escena_menu:
                    DibujarMenu();
                break;

                case Escena_configuracion:
                    dibujarPantallaPendiente("CONFIGURAR PARTIDA");
                break;

                case Escena_puntajes:
                    dibujarPantallaPendiente("MEJORES PUNTAJES");
                break;

                case Escena_creditos:
                    dibujarPantallaPendiente("CREDITOS");
                break;

                case Escena_juego:
                    DibujarPruebaTablero();
                break;

                default: break;
            }

        EndDrawing();
    }

    // Y se liberan antes de cerrar, por la misma razon al reves: despues de
    // CloseWindow ya no hay a quien devolverle esa memoria.
    descargarTexturasTablero();

    CloseWindow(); // Cierra la ventana

    //cout << "\n\nEl programa se acabo!" << endl;
    //system("pause");
    return 0;
}
