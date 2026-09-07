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
#include "Boton.hpp"
#include "Tema.hpp"

//***********************************************
// DATOS DEL MENU
//***********************************************

const int NUM_OPCIONES = 5;

// Las etiquetas y el destino de cada opcion se guardan en dos arreglos paralelos:
// ETIQUETAS[i] lleva a DESTINOS[i]. Se hace asi para que agregar una opcion al
// menu sea agregar dos lineas de datos y nada mas. La alternativa (un switch con
// un case por opcion) obliga a tocar codigo cada vez, y es donde se cuelan los
// errores cuando el menu crece.

static const char* ETIQUETAS[NUM_OPCIONES] = {
    "Jugar",
    "Mejores puntajes",
    "Opciones",
    "Creditos",
    "Salir"
};

static const Escena_Estado DESTINOS[NUM_OPCIONES] = {
    Escena_configuracion,
    Escena_puntajes,
    Escena_opciones,
    Escena_creditos,
    Escena_salir
};

// 'static' a nivel de archivo significa que esta variable solo existe dentro de
// Menu.cpp: ningun otro archivo la puede ver ni modificar. Eso es lo que hace que
// esto sea un modulo y no una variable global suelta. main.cpp no sabe -ni tiene
// por que saber- cual opcion esta resaltada.
static int opcionSeleccionada = 0;

//***********************************************
// ARTE DEL MENU
//***********************************************

// Las tres imagenes son opcionales: si falta alguna, esa parte se dibuja con
// texto. Asi el juego sigue corriendo mientras el arte esta a medio hacer, y nadie
// se queda bloqueado esperando a nadie.
static const char* RUTA_FONDO  = "recursos/fondo.png";
static const char* RUTA_TITULO = "recursos/titulo.png";
static const char* RUTA_JUGAR  = "recursos/jugar.png";

static Texture2D texturaFondo;
static Texture2D texturaTitulo;
static Texture2D texturaJugar;

static bool hayFondo  = false;
static bool hayTitulo = false;
static bool hayJugar  = false;

/**
 * \brief Carga una imagen si existe y avisa si se pudo.
 * \param ruta    Archivo a cargar.
 * \param destino D&oacute;nde dejar la textura.
 * \return Verdadero si qued&oacute; cargada.
 */
static bool cargarSiEsta(const char* ruta, Texture2D* destino)
{
    if(!FileExists(ruta)) return false;

    *destino = LoadTexture(ruta);

    if(!IsTextureValid(*destino)) return false;

    // Filtro suave: la imagen se dibuja a su tamano exacto, pero si algun dia la
    // ventana cambia de medida esto evita que se vea dentada.
    SetTextureFilter(*destino, TEXTURE_FILTER_BILINEAR);

    return true;
}

void CargarTexturasMenu()
{
    hayFondo  = cargarSiEsta(RUTA_FONDO,  &texturaFondo);
    hayTitulo = cargarSiEsta(RUTA_TITULO, &texturaTitulo);
    hayJugar  = cargarSiEsta(RUTA_JUGAR,  &texturaJugar);
}

void DescargarTexturasMenu()
{
    if(hayFondo)  UnloadTexture(texturaFondo);
    if(hayTitulo) UnloadTexture(texturaTitulo);
    if(hayJugar)  UnloadTexture(texturaJugar);

    hayFondo  = false;
    hayTitulo = false;
    hayJugar  = false;
}

/**
 * \brief Dibuja una textura centrada horizontalmente, a su tama&ntilde;o real.
 * \param tex Textura a dibujar.
 * \param y   Altura de su borde superior.
 */
static void dibujarCentrada(Texture2D tex, int y)
{
    DrawTexture(tex, (GetScreenWidth() - tex.width) / 2, y, WHITE);
}

//***********************************************
// MENU PRINCIPAL
//***********************************************

/**
 * \brief La zona clicable de una opci&oacute;n del men&uacute;.
 *
 * Es m&aacute;s alta y ancha que su texto a prop&oacute;sito: en un stand se apunta r&aacute;pido y
 * mal, y un blanco de 460 por 52 p&iacute;xeles perdona el pulso de un ni&ntilde;o de seis a&ntilde;os.
 *
 * \param indice Opci&oacute;n, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle zonaOpcion(int indice)
{
    const float ANCHO = 460.0f;

    return rectangulo((GetScreenWidth() - ANCHO) / 2.0f,
                      276.0f + indice * 62.0f, ANCHO, 52.0f);
}

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

    // El raton manda sobre las flechas: si el puntero esta encima de una opcion,
    // esa es la resaltada. Sin esta linea las dos formas de navegar se
    // contradirian en pantalla -una cosa resaltada y otra bajo el cursor-.
    for(int i = 0; i < NUM_OPCIONES; i++){
        if(ratonEncima(zonaOpcion(i))) opcionSeleccionada = i;
    }

    if(IsKeyPressed(KEY_ENTER)){
        return DESTINOS[opcionSeleccionada];
    }

    for(int i = 0; i < NUM_OPCIONES; i++){
        if(botonClicado(zonaOpcion(i))) return DESTINOS[i];
    }

    // Nadie ha elegido nada: nos quedamos donde estamos.
    return Escena_menu;
}

void DibujarMenu()
{
    // El fondo va primero, tapando el color liso que dejo ClearBackground.
    if(hayFondo) DrawTexture(texturaFondo, 0, 0, WHITE);

    if(hayTitulo) dibujarCentrada(texturaTitulo, 110);
    else          dibujarTextoCentrado("GATORAMA", 110, 80, COLOR_TITULO);

    for(int i = 0; i < NUM_OPCIONES; i++){

        // Lo unico que distingue a la opcion resaltada es como se dibuja. No hay
        // que guardar ningun estado extra ni avisarle a nadie: se decide aqui,
        // en el momento de dibujar. Eso es el modo inmediato.
        bool      seleccionada = (i == opcionSeleccionada);
        Rectangle zona         = zonaOpcion(i);

        // La primera opcion ya tiene su imagen; las demas siguen siendo texto
        // mientras el arte se termina.
        if(i == 0 && hayJugar){
            // Sin resaltar se dibuja un poco mas oscura. El tinte multiplica, asi
            // que respeta la transparencia del PNG: no le pinta un rectangulo
            // encima como haria un velo.
            Color tinte = seleccionada ? WHITE : (Color){ 165, 165, 165, 255 };

            DrawTexture(texturaJugar, (int)zona.x, (int)zona.y, tinte);
        } else {
            Color color  = seleccionada ? COLOR_SELECCION : COLOR_TEXTO;
            int   tamano = seleccionada ? 38 : 32;

            // El texto se centra dentro de su zona clicable, para que lo que se ve
            // y lo que responde al raton sean lo mismo.
            int y = (int)(zona.y + (zona.height - tamano) / 2.0f);

            const char* etiqueta = seleccionada
                                 ? TextFormat("> %s <", ETIQUETAS[i])
                                 : ETIQUETAS[i];

            dibujarTextoCentrado(etiqueta, y, tamano, color);
        }

        // El anillo marca la opcion elegida igual sobre texto que sobre imagen.
        if(seleccionada) dibujarAnilloEnfoque(zona);
    }

    dibujarTextoCentrado("Clic para elegir     o flechas y Enter", 640, 20, COLOR_TENUE);
}
