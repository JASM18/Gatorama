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
#include "Instrucciones.hpp"
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

// Si la ventana de instrucciones esta abierta encima del menu. Es una bandera y no
// una escena por lo mismo que la pausa: el menu se sigue viendo debajo.
static bool enInstrucciones = false;

/**
 * \brief El bot&oacute;n de ayuda, arriba a la derecha.
 *
 * Mismo lugar y mismo tama&ntilde;o que el del tablero, para que sea el mismo bot&oacute;n en
 * la cabeza de quien juega y no dos cosas parecidas en esquinas distintas.
 *
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle botonDeAyuda()
{
    return rectangulo(GetScreenWidth() - 60.0f, 18.0f, 40.0f, 40.0f);
}

//***********************************************
// ARTE DEL MENU
//***********************************************

// El menu se arma con tres imagenes que trabajan juntas, y el truco esta en como
// esta hecho el letrero: **las letras estan caladas**. Donde va cada palabra, el
// PNG es transparente, asi que el color de un renglon no se pinta encima sino
// DEBAJO y se ve a traves del hueco.
//
//   fondoPrincipal        el letrero completo, con las letras caladas. Va hasta
//                         arriba y tapa todo lo demas menos esos huecos.
//   botonesInactivo_fondo las placas cafes de los cinco renglones, ya colocadas
//                         en coordenadas de pantalla: se dibuja tal cual en 0,0.
//   botonActivo_fondo     la placa clara de un solo renglon, la del elegido.
//
// Las tres son opcionales: si falta el letrero se dibuja el menu de texto de
// siempre, para poder seguir trabajando con el arte a medias.
static const char* RUTA_FONDO  = "recursos/fondoPrincipal.png";
static const char* RUTA_PLACAS = "recursos/botonesInactivo_fondo.png";
static const char* RUTA_ACTIVA = "recursos/botonActivo_fondo.png";

static Texture2D texturaFondo;
static Texture2D texturaPlacas;
static Texture2D texturaActiva;

static bool hayFondo  = false;
static bool hayPlacas = false;
static bool hayActiva = false;

void CargarTexturasMenu()
{
    hayFondo  = cargarTexturaSiEsta(RUTA_FONDO,  &texturaFondo);
    hayPlacas = cargarTexturaSiEsta(RUTA_PLACAS, &texturaPlacas);
    hayActiva = cargarTexturaSiEsta(RUTA_ACTIVA, &texturaActiva);
}

void DescargarTexturasMenu()
{
    if(hayFondo)  UnloadTexture(texturaFondo);
    if(hayPlacas) UnloadTexture(texturaPlacas);
    if(hayActiva) UnloadTexture(texturaActiva);

    hayFondo  = false;
    hayPlacas = false;
    hayActiva = false;
}

//***********************************************
// MENU PRINCIPAL
//***********************************************

// Antes estas zonas salian de una formula -276 mas 62 por renglon-, porque el
// menu era texto y el texto se acomoda donde uno le diga. Ya no: el letrero esta
// dibujado a mano y cada palabra quedo donde quedo, a distancias que no son
// iguales. Asi que ahora los rectangulos vienen del arte, y no al reves.
//
// Salieron de medir las letras caladas de fondoPrincipal.png. Cada uno cubre por
// completo su renglon y no alcanza el de al lado: los cortes van a la mitad del
// hueco entre dos palabras, asi que los cinco se tocan sin encimarse y no queda
// ninguna franja muerta donde un clic no haga nada.
static const Rectangle ZONAS_OPCION[NUM_OPCIONES] = {
    { 410.0f, 271.0f, 460.0f, 71.0f },   // Jugar
    { 410.0f, 342.0f, 460.0f, 65.0f },   // Mejores puntajes
    { 410.0f, 407.0f, 460.0f, 59.0f },   // Opciones
    { 410.0f, 466.0f, 460.0f, 57.0f },   // Creditos
    { 410.0f, 523.0f, 460.0f, 61.0f }    // Salir
};

/**
 * \brief La zona clicable de una opci&oacute;n del men&uacute;.
 *
 * Es m&aacute;s alta y ancha que su palabra a prop&oacute;sito: en un stand se apunta r&aacute;pido y
 * mal, y un blanco de 460 p&iacute;xeles de ancho perdona el pulso de un ni&ntilde;o de seis a&ntilde;os.
 *
 * Sirve para dos cosas a la vez: decidir d&oacute;nde cay&oacute; un clic y d&oacute;nde va la placa
 * clara del rengl&oacute;n elegido. Por eso no pueden separarse nunca.
 *
 * \param indice Opci&oacute;n, desde cero.
 * \return Su rect&aacute;ngulo en pantalla.
 */
static Rectangle zonaOpcion(int indice)
{
    return ZONAS_OPCION[indice];
}

Escena_Estado ActualizarMenu()
{
    // Con las instrucciones abiertas, ellas se quedan con toda la entrada: ni las
    // flechas mueven la opcion resaltada ni un clic elige nada de atras.
    if(enInstrucciones){
        if(ActualizarInstrucciones()) enInstrucciones = false;

        return Escena_menu;
    }

    if(botonClicado(botonDeAyuda())){
        enInstrucciones = true;
        return Escena_menu;
    }

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

/**
 * \brief El men&uacute; escrito con letras, para cuando no est&aacute; el letrero.
 *
 * No es un adorno: es lo que deja seguir probando el juego mientras el arte se
 * dibuja, o el d&iacute;a que alguien clone el repositorio sin la carpeta de recursos.
 */
static void dibujarMenuDeTexto()
{
    dibujarTextoCentrado("GATORAMA", 110, 80, COLOR_TITULO);

    for(int i = 0; i < NUM_OPCIONES; i++){

        // Lo unico que distingue a la opcion resaltada es como se dibuja. No hay
        // que guardar ningun estado extra ni avisarle a nadie: se decide aqui, en
        // el momento de dibujar. Eso es el modo inmediato.
        bool      seleccionada = (i == opcionSeleccionada);
        Rectangle zona         = zonaOpcion(i);

        Color color  = seleccionada ? COLOR_SELECCION : COLOR_TEXTO;
        int   tamano = seleccionada ? 38 : 32;

        // El texto se centra dentro de su zona clicable, para que lo que se ve y
        // lo que responde al raton sean lo mismo.
        int y = (int)(zona.y + (zona.height - tamano) / 2.0f);

        const char* etiqueta = seleccionada
                             ? TextFormat("> %s <", ETIQUETAS[i])
                             : ETIQUETAS[i];

        dibujarTextoCentrado(etiqueta, y, tamano, color);

        if(seleccionada) dibujarAnilloEnfoque(zona);
    }

    dibujarTextoCentrado("Clic para elegir     o flechas y Enter", 640, 20, COLOR_TENUE);
}

void DibujarMenu()
{
    if(hayFondo){

        // Cuatro capas, y el orden no es negociable: invertirlo no pierde un
        // detalle, pierde el efecto entero. Todo lo de abajo se ve UNICAMENTE a
        // traves de las letras caladas; el letrero, que va hasta arriba, tapa el
        // resto por completo.
        //
        // La primera capa es un cafe liso de respaldo. La placa pintada no cubre
        // el 100% de los calados -le faltan unos 280 pixeles sueltos, casi todos
        // debajo de "Salir"-, y sin este relleno esos huecos dejarian ver el fondo
        // claro de la pantalla: una mancha blanca sobre la madera. Cuesta una
        // linea y hace que el menu aguante cualquier retoque del letrero.
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), COLOR_LETRERO);

        if(hayPlacas) DrawTexture(texturaPlacas, 0, 0, WHITE);

        if(hayActiva){
            Rectangle zona    = zonaOpcion(opcionSeleccionada);
            Rectangle origen  = { 0.0f, 0.0f,
                                  (float)texturaActiva.width, (float)texturaActiva.height };
            Vector2   desfase = { 0.0f, 0.0f };

            // Se estira al renglon en vez de dibujarse a su tamano real: los
            // renglones no miden todos igual y la placa tiene que cubrir las letras
            // completas, si no la palabra sale mocha de arriba. Como es un color
            // liso, estirarla no se nota en nada.
            DrawTexturePro(texturaActiva, origen, zona, desfase, 0.0f, WHITE);
        }

        DrawTexture(texturaFondo, 0, 0, WHITE);

    } else {
        dibujarMenuDeTexto();
    }

    dibujarBoton(botonDeAyuda(), "?", false);

    // La ventana va hasta el final, para que quede encima de todo lo demas.
    if(enInstrucciones) DibujarInstrucciones();
}
