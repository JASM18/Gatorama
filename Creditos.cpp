/**
 * \file Creditos.cpp
 * \brief Implementaci&oacute;n de la pantalla de cr&eacute;ditos.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Creditos.hpp"
#include "Boton.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// TEXTO DE LOS CREDITOS
//***********************************************

// Los nombres van SIN ACENTO porque la fuente que trae raylib de fabrica solo
// cubre ASCII: "Jesus" con acento saldria con un simbolo en lugar de la u. Son
// nombres de personas, asi que esto hay que arreglarlo antes del rally cargando
// una fuente propia con LoadFontEx y su juego de caracteres en espanol.
const int NUM_DESARROLLADORES = 4;

static const char* DESARROLLADORES[NUM_DESARROLLADORES] = {
    "Jesus Axel Sanchez Montoy",
    "Alba Rosa Helleon Cardenas",
    "Ivana Lin Chenoweth Galaz",
    "Angel Daniel Duron Urbina"
};

static Rectangle botonVolver()
{
    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 110.0f, 220.0f, 52.0f);
}

//***********************************************
// PANTALLA
//***********************************************

Escena_Estado ActualizarCreditos()
{
    if(IsKeyPressed(KEY_ESCAPE))     return Escena_menu;
    if(botonClicado(botonVolver()))  return Escena_menu;

    return Escena_creditos;
}

void DibujarCreditos()
{
    dibujarTextoCentrado("CREDITOS", 60, 44, COLOR_TITULO);

    dibujarTextoCentrado("Videojuego desarrollado para el", 150, 22, COLOR_TEXTO);
    dibujarTextoCentrado("Rally de la Ninez Cientifica 2026", 182, 26, COLOR_SELECCION);

    dibujarTextoCentrado("por estudiantes de la Licenciatura en Ciencias de la Computacion",
                         228, 20, COLOR_TEXTO);
    dibujarTextoCentrado("Universidad de Sonora", 256, 20, COLOR_TEXTO);

    dibujarTextoCentrado("Desarrolladores", 320, 24, COLOR_TITULO);

    for(int i = 0; i < NUM_DESARROLLADORES; i++){
        dibujarTextoCentrado(DESARROLLADORES[i], 366 + i * 34, 22, COLOR_TEXTO);
    }

    // Credito a raylib. Su licencia zlib no obliga a ponerlo en el juego, pero es
    // la costumbre y no cuesta nada: alguien hizo el trabajo del que dependemos.
    dibujarTextoCentrado("Hecho con raylib 6.0, de Ramon Santamaria (raysan5)",
                         540, 18, COLOR_TENUE);

    dibujarBoton(botonVolver(), "Volver al menu", false);
}
