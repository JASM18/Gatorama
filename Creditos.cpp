/**
 * \file Creditos.cpp
 * \brief Implementaci&oacute;n de la pantalla de cr&eacute;ditos.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Creditos.hpp"
#include "Boton.hpp"
#include "Opciones.hpp"
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

// La ventana del engrane, abierta encima de los creditos.
static bool enOpciones = false;

// Si "Volver al menu" esta resaltado. Es el unico boton, asi que el enfoque es un
// si o un no. Mismo modelo que las demas pantallas: arranca apagado, cualquier
// flecha lo prende, y el raton lo prende o lo apaga solo cuando de verdad se mueve.
static bool volverEnfocado = false;

//***********************************************
// ARTE DE LA PANTALLA
//***********************************************

// La pantalla completa, 1280x720: cortinas, pergamino con todo el texto ya escrito
// -con acentos, que la fuente de fabrica no tiene- y la placa de "Volver al Menu".
// Un pixel de la imagen es un pixel de la pantalla.
static const char* RUTA_FONDO = "recursos/creditos.png";

static Texture2D texturaFondo;
static bool      hayFondo = false;

void CargarTexturasCreditos()
{
    hayFondo = cargarTexturaSiEsta(RUTA_FONDO, &texturaFondo);
}

void DescargarTexturasCreditos()
{
    if(hayFondo) UnloadTexture(texturaFondo);

    hayFondo = false;
}

static Rectangle botonVolver()
{
    // Con el arte, la placa medida sobre creditos.png por su contorno.
    if(hayFondo) return rectangulo(515.0f, 573.0f, 254.0f, 60.0f);

    return rectangulo((GetScreenWidth() - 220.0f) / 2.0f,
                      GetScreenHeight() - 110.0f, 220.0f, 52.0f);
}

//***********************************************
// PANTALLA
//***********************************************

Escena_Estado ActualizarCreditos()
{
    // Con la ventana de opciones abierta, ella se queda con toda la entrada: el
    // ESC la cierra a ella, no saca de los creditos.
    if(enOpciones){
        if(ActualizarOpciones()) enOpciones = false;

        return Escena_creditos;
    }

    if(botonClicado(zonaBotonOpciones())){
        enOpciones = true;
        PrepararOpciones();
        return Escena_creditos;
    }

    Vector2 movimientoRaton = GetMouseDelta();

    if(movimientoRaton.x != 0.0f || movimientoRaton.y != 0.0f){
        volverEnfocado = ratonEncima(botonVolver());
    }

    if(IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_DOWN) ||
       IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)){
        volverEnfocado = true;
    }

    // Enter tambien regresa: es el unico boton de la pantalla. Se apaga el
    // resaltado al salir, para que la siguiente visita llegue sin nada marcado.
    bool salir = IsKeyPressed(KEY_ESCAPE) || enfoqueActivado() || botonClicado(botonVolver());

    if(salir){
        volverEnfocado = false;
        return Escena_menu;
    }

    return Escena_creditos;
}

void DibujarCreditos()
{
    if(hayFondo){
        DrawTexture(texturaFondo, 0, 0, WHITE);

        marcarPlaca(botonVolver(), false, volverEnfocado);

        dibujarBotonOpciones(zonaBotonOpciones());

        if(enOpciones) DibujarOpciones();
        return;
    }

    // ---- Sin arte: el texto de siempre ----
    // Todo va directo sobre la madera, por eso con contorno y colores claros.
    dibujarTextoCentradoSobreFondo("CREDITOS", 60, 44, COLOR_FONDO_TITULO);

    dibujarTextoCentradoSobreFondo("Videojuego desarrollado para el", 150, 22, COLOR_FONDO_TEXTO);
    dibujarTextoCentradoSobreFondo("Rally de la Ninez Cientifica 2026", 182, 26, COLOR_FONDO_RESALTE);

    dibujarTextoCentradoSobreFondo("por estudiantes de la Licenciatura en Ciencias de la Computacion",
                                   228, 20, COLOR_FONDO_TEXTO);
    dibujarTextoCentradoSobreFondo("Universidad de Sonora", 256, 20, COLOR_FONDO_TEXTO);

    dibujarTextoCentradoSobreFondo("Desarrolladores", 320, 24, COLOR_FONDO_TITULO);

    for(int i = 0; i < NUM_DESARROLLADORES; i++){
        dibujarTextoCentradoSobreFondo(DESARROLLADORES[i], 366 + i * 34, 22, COLOR_FONDO_TEXTO);
    }

    // Credito a raylib. Su licencia zlib no obliga a ponerlo en el juego, pero es
    // la costumbre y no cuesta nada: alguien hizo el trabajo del que dependemos.
    dibujarTextoCentradoSobreFondo("Hecho con raylib 6.0, de Ramon Santamaria (raysan5)",
                                   540, 18, COLOR_FONDO_TENUE);

    dibujarBoton(botonVolver(), "Volver al menu", false);

    if(volverEnfocado) dibujarAnilloEnfoque(botonVolver());

    dibujarBotonOpciones(zonaBotonOpciones());

    // La ventana va hasta el final, encima de todo.
    if(enOpciones) DibujarOpciones();
}
