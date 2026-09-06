/**
 * \file VistaTablero.cpp
 * \brief Implementaci&oacute;n del acomodo y dibujo del tablero de cartas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "VistaTablero.hpp"
#include "Dificultad.hpp"
#include "Dibujo.hpp"
#include "Tema.hpp"

//***********************************************
// ACOMODO DE LAS CARTAS
//***********************************************

Rectangle areaDelTablero()
{
    // Se le quita a la ventana lo que ocupa la interfaz. Si manana el marcador
    // crece, se cambian estos tres numeros y el tablero se reacomoda solo: no hay
    // ninguna posicion de carta escrita a mano en ningun lado.
    const float MARGEN_LATERAL  = 40.0f;
    const float ESPACIO_ARRIBA  = 140.0f;   // marcador, reloj y turno
    const float ESPACIO_ABAJO   = 55.0f;    // linea de ayuda

    Rectangle area;
    area.x      = MARGEN_LATERAL;
    area.y      = ESPACIO_ARRIBA;
    area.width  = GetScreenWidth()  - MARGEN_LATERAL * 2.0f;
    area.height = GetScreenHeight() - ESPACIO_ARRIBA - ESPACIO_ABAJO;

    return area;
}

DisenoTablero calcularDiseno(int filas, int columnas, Rectangle area, float relacionAspecto)
{
    DisenoTablero diseno;
    diseno.filas    = filas;
    diseno.columnas = columnas;

    // Paso 1: repartir el area en celdas iguales, una por carta.
    float celdaAncho = area.width  / columnas;
    float celdaAlto  = area.height / filas;

    // Paso 2: la carta no llena su celda; deja un respiro alrededor para que dos
    // cartas vecinas no se toquen. 12% se ve bien y no desperdicia pantalla.
    const float RESPIRO = 0.12f;
    float dispAncho = celdaAncho * (1.0f - RESPIRO);
    float dispAlto  = celdaAlto  * (1.0f - RESPIRO);

    // Paso 3: meter la carta mas grande que quepa SIN deformarla. Comparar la
    // forma del hueco contra la forma de la carta dice cual lado se queda corto,
    // y ese es el que manda. Si estiraramos los dos lados el gato saldria aplastado.
    if(dispAncho / dispAlto > relacionAspecto){
        // Sobra ancho: el alto es el limite.
        diseno.altoCarta  = dispAlto;
        diseno.anchoCarta = dispAlto * relacionAspecto;
    } else {
        // Sobra alto: el ancho es el limite.
        diseno.anchoCarta = dispAncho;
        diseno.altoCarta  = dispAncho / relacionAspecto;
    }

    // Paso 4: el hueco que sobra puede ser distinto a lo ancho que a lo alto. Se
    // usa el menor de los dos como separacion pareja, para que la cuadricula se
    // vea uniforme en vez de estirada en una direccion.
    float sobraX = celdaAncho - diseno.anchoCarta;
    float sobraY = celdaAlto  - diseno.altoCarta;
    diseno.separacion = (sobraX < sobraY) ? sobraX : sobraY;

    diseno.pasoX = diseno.anchoCarta + diseno.separacion;
    diseno.pasoY = diseno.altoCarta  + diseno.separacion;

    // Paso 5: como se apreto la separacion, el bloque de cartas ya no llena el
    // area. Se centra: la ultima carta no lleva separacion despues, por eso se resta.
    float anchoBloque = columnas * diseno.pasoX - diseno.separacion;
    float altoBloque  = filas    * diseno.pasoY - diseno.separacion;

    diseno.origenX = area.x + (area.width  - anchoBloque) / 2.0f;
    diseno.origenY = area.y + (area.height - altoBloque ) / 2.0f;

    return diseno;
}

Rectangle rectanguloDeCarta(const DisenoTablero& diseno, int fila, int columna)
{
    Rectangle rec;
    rec.x      = diseno.origenX + columna * diseno.pasoX;
    rec.y      = diseno.origenY + fila    * diseno.pasoY;
    rec.width  = diseno.anchoCarta;
    rec.height = diseno.altoCarta;

    return rec;
}

int indiceCartaEnPunto(const DisenoTablero& diseno, Vector2 punto)
{
    // Se recorren todas las cartas y se pregunta por cada una. Con 30 cartas como
    // maximo esto es gratis, y se lee mucho mejor que despejar la fila y la
    // columna con divisiones, que ademas habria que corregir por los huecos.
    for(int fila = 0; fila < diseno.filas; fila++){
        for(int columna = 0; columna < diseno.columnas; columna++){

            if(CheckCollisionPointRec(punto, rectanguloDeCarta(diseno, fila, columna))){
                return fila * diseno.columnas + columna;
            }
        }
    }

    // El punto cayo en un hueco o fuera del tablero. Devolver -1 y no 0 es
    // importante: 0 es una carta valida y confundirlos voltearia la esquina.
    return -1;
}

// Forma de la carta, compartida por el dorso y la cara para que las dos se vean
// iguales. REDONDEZ va de 0 (esquinas rectas) a 1 (totalmente redondeadas).
static const float REDONDEZ  = 0.15f;
static const int   SEGMENTOS = 8;   // triangulos por esquina; con 8 ya se ve liso

void dibujarDorsoCarta(Rectangle rec, bool resaltada)
{
    Color relleno = resaltada ? COLOR_CARTA_HOVER : COLOR_CARTA_DORSO;
    float grosor  = resaltada ? 4.0f : 2.0f;

    DrawRectangleRounded(rec, REDONDEZ, SEGMENTOS, relleno);
    DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
}

//***********************************************
// ILUSTRACIONES DE LAS CARTAS
//***********************************************

// Las rutas son relativas a la carpeta desde donde se ejecuta el programa, no a
// donde esta el .exe. Code::Blocks corre el juego desde la carpeta del proyecto,
// asi que esto funciona con F9. Si algun dia se abre el .exe a mano desde
// bin/Debug no las va a encontrar; por eso se avisa en vez de tronar.
//
// Cuando el mazo este completo esta lista saldra de un archivo y no del codigo,
// para que agregar un gato no obligue a recompilar. Por ahora son dos y caben.
const int NUM_GATOS = 2;

static const char* RUTAS_GATOS[NUM_GATOS] = {
    "recursos/gato01.png",
    "recursos/gato02.png"
};

// Solo se guardan las que si cargaron, apretadas al principio del arreglo. Asi
// numTexturasCargadas siempre sirve para sacar el residuo sin dejar huecos.
static Texture2D   texturasGato[NUM_GATOS];
static const char* rutasCargadas[NUM_GATOS];
static int         numTexturasCargadas = 0;

void cargarTexturasTablero()
{
    numTexturasCargadas = 0;

    for(int i = 0; i < NUM_GATOS; i++){

        // Se pregunta antes de cargar para distinguir dos fallas distintas: que el
        // archivo no este, o que este pero no se pueda leer como imagen.
        if(!FileExists(RUTAS_GATOS[i])) continue;

        Texture2D tex = LoadTexture(RUTAS_GATOS[i]);
        if(!IsTextureValid(tex)) continue;

        // Una mipmap es la misma imagen guardada ya reducida a la mitad, a la
        // cuarta parte, y asi. Sin ellas, dibujar una imagen de 320 px a 102 obliga
        // a la tarjeta a saltarse pixeles del original, y el resultado tiembla y se
        // ve dentado. Con mipmaps toma la copia del tamano mas cercano.
        GenTextureMipmaps(&tex);

        // TRILINEAR es el filtro que sabe usar esas copias: mezcla las dos mas
        // cercanas. BILINEAR sin mipmaps solo promedia vecinos del original, que
        // alcanza para reducciones chicas pero no para bajar de 320 a 102.
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);

        texturasGato[numTexturasCargadas]  = tex;
        rutasCargadas[numTexturasCargadas] = RUTAS_GATOS[i];
        numTexturasCargadas++;
    }
}

int numeroDeIlustraciones()
{
    return numTexturasCargadas;
}

void descargarTexturasTablero()
{
    for(int i = 0; i < numTexturasCargadas; i++){
        UnloadTexture(texturasGato[i]);
    }

    numTexturasCargadas = 0;
}

/**
 * \brief Redondea las esquinas de una ilustraci&oacute;n ya dibujada.
 *
 * La imagen es un rect&aacute;ngulo, as&iacute; que sus cuatro esquinas cuadradas asoman por
 * fuera del borde redondeado de la carta. Esto las tapa pintando encima, del color
 * del fondo, la rebanada que queda **entre el arco y la esquina cuadrada**: cuatro
 * sectores de anillo, uno por esquina.
 *
 * **Ojo:** funciona porque el fondo del juego es un color liso. El d&iacute;a que se ponga
 * una imagen de fondo detr&aacute;s del tablero, estos parches se van a notar y habr&aacute; que
 * recortar la textura de otra forma (una m&aacute;scara o un shader).
 *
 * \param rec Rect&aacute;ngulo de la carta que se acaba de dibujar.
 */
static void redondearEsquinas(Rectangle rec)
{
    // El radio se calcula igual que raylib: el lado corto por la redondez, entre
    // dos. Si no coincidiera, el parche no caeria justo sobre el arco del borde.
    float ladoCorto = (rec.width < rec.height) ? rec.width : rec.height;
    float radio     = ladoCorto * REDONDEZ / 2.0f;

    // La esquina cuadrada esta a radio * raiz(2) del centro del arco, o sea a
    // 1.414 radios. Con 1.6 el parche la rebasa con margen; lo que se pinte de mas
    // cae fuera de la carta, sobre el fondo, y ahi no se nota.
    float afuera = radio * 1.6f;

    const int SEG = 12;   // suficiente para que el arco no se vea poligonal

    Vector2 supIzq = { rec.x + radio,             rec.y + radio };
    Vector2 supDer = { rec.x + rec.width - radio, rec.y + radio };
    Vector2 infDer = { rec.x + rec.width - radio, rec.y + rec.height - radio };
    Vector2 infIzq = { rec.x + radio,             rec.y + rec.height - radio };

    // Los angulos van en grados, con el cero apuntando a la derecha y creciendo
    // hacia abajo (la Y de la pantalla crece hacia abajo). Cada esquina es un
    // cuarto de vuelta.
    DrawRing(supIzq, radio, afuera, 180.0f, 270.0f, SEG, COLOR_FONDO);
    DrawRing(supDer, radio, afuera, 270.0f, 360.0f, SEG, COLOR_FONDO);
    DrawRing(infDer, radio, afuera,   0.0f,  90.0f, SEG, COLOR_FONDO);
    DrawRing(infIzq, radio, afuera,  90.0f, 180.0f, SEG, COLOR_FONDO);
}

void dibujarCaraCarta(Rectangle rec, bool resaltada, int indiceIlustracion)
{
    float grosor = resaltada ? 4.0f : 2.0f;

    if(numTexturasCargadas == 0){
        // Sin arte el juego se sigue pudiendo probar: relleno liso y ya.
        DrawRectangleRounded(rec, REDONDEZ, SEGMENTOS, COLOR_TEXTO);
        DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
        return;
    }

    // El residuo deja el indice dentro del arreglo aunque pidan el gato numero 40.
    // Se le suma el total antes por si llega un negativo: en C++ el residuo de un
    // negativo es negativo, igual que en el menu circular.
    int cual = ((indiceIlustracion % numTexturasCargadas) + numTexturasCargadas)
             % numTexturasCargadas;

    Texture2D tex = texturasGato[cual];

    // DrawTexturePro toma un pedazo de la imagen (origen) y lo ajusta a un
    // rectangulo de pantalla (rec). Aqui el origen es la imagen completa. Es la
    // version que sirve para dibujar a un tamano distinto al del archivo; las
    // otras DrawTexture solo saben pintarla a escala 1:1.
    Rectangle origen  = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    Vector2   desfase = { 0.0f, 0.0f };

    DrawTexturePro(tex, origen, rec, desfase, 0.0f, WHITE);

    // Primero se recortan las esquinas cuadradas de la imagen y luego se pinta el
    // borde encima, para que el borde quede limpio y no lo tape el parche.
    redondearEsquinas(rec);
    DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
}

//***********************************************
// BANCO DE PRUEBAS (TEMPORAL)
//***********************************************

// Formas de carta que se pueden probar. Es una tabla y no una constante porque
// justamente lo que se quiere es compararlas en pantalla antes de decidir.
const int NUM_ASPECTOS = 3;

struct InfoAspecto {
    const char* nombre;
    float relacion;         // ancho entre alto
};

static const InfoAspecto ASPECTOS[NUM_ASPECTOS] = {
    { "2:3  (carta de baraja)", 2.0f / 3.0f },
    { "3:4  (foto vertical)",   3.0f / 4.0f },
    { "1:1  (cuadrada)",        1.0f        }
};

// Estado del banco de pruebas. Es static: solo existe dentro de este archivo,
// igual que la opcion resaltada del menu vive dentro de Menu.cpp.
static Dificultad dificultadActual = Dificultad_facil;
static int        aspectoActual    = 0;
static int        cartaResaltada   = -1;
static bool       mostrarCara      = false;   // false = todas boca abajo

/**
 * \brief Rearma el acomodo con la dificultad y la forma que est&aacute;n elegidas.
 *
 * Actualizar y dibujar lo necesitan por igual, y recalcularlo cuesta unas cuantas
 * multiplicaciones: sale m&aacute;s barato que guardarlo y arriesgarse a que los dos
 * lados queden con cuentas distintas.
 */
static DisenoTablero disenoActual()
{
    const InfoDificultad& nivel = DIFICULTADES[dificultadActual];

    return calcularDiseno(nivel.filas, nivel.columnas,
                          areaDelTablero(), ASPECTOS[aspectoActual].relacion);
}

Escena_Estado ActualizarPruebaTablero()
{
    if(IsKeyPressed(KEY_ONE))   dificultadActual = Dificultad_facil;
    if(IsKeyPressed(KEY_TWO))   dificultadActual = Dificultad_normal;
    if(IsKeyPressed(KEY_THREE)) dificultadActual = Dificultad_dificil;

    if(IsKeyPressed(KEY_A)){
        aspectoActual = (aspectoActual + 1) % NUM_ASPECTOS;
    }

    if(IsKeyPressed(KEY_V)) mostrarCara = !mostrarCara;

    // Que carta esta bajo el puntero se decide aqui, junto con el resto de la
    // entrada, y no al dibujar. Dibujar tiene que poder correr sin cambiar nada.
    cartaResaltada = indiceCartaEnPunto(disenoActual(), GetMousePosition());

    if(IsKeyPressed(KEY_ESCAPE)) return Escena_menu;

    return Escena_juego;
}

void DibujarPruebaTablero()
{
    DisenoTablero diseno = disenoActual();

    const InfoDificultad& nivel   = DIFICULTADES[dificultadActual];
    const InfoAspecto&    aspecto = ASPECTOS[aspectoActual];

    // Contorno del area disponible, para ver cuanta pantalla queda sin usar.
    DrawRectangleLinesEx(areaDelTablero(), 1.0f, COLOR_TENUE);

    // Las cartas. Todavia no hay modelo, asi que se dibujan todas boca abajo y
    // numeradas; el numero solo sirve para contarlas de un vistazo.
    for(int fila = 0; fila < diseno.filas; fila++){
        for(int columna = 0; columna < diseno.columnas; columna++){

            int       indice = fila * diseno.columnas + columna;
            Rectangle rec    = rectanguloDeCarta(diseno, fila, columna);

            if(mostrarCara){
                // Las cartas van de dos en dos: la 0 y la 1 son una pareja, la 2 y
                // la 3 la siguiente. Con eso el banco de pruebas ya se ve como un
                // memorama de verdad, aunque todavia no haya barajado ni modelo.
                dibujarCaraCarta(rec, indice == cartaResaltada, indice / 2);
                continue;   // sobre la ilustracion no se pinta el numero
            }

            dibujarDorsoCarta(rec, indice == cartaResaltada);

            // El numero se escala con la carta para que se lea igual en los tres
            // tableros. MeasureText mide con ese mismo tamano; si no, no centra.
            int         tamano = (int)(diseno.altoCarta * 0.30f);
            const char* texto  = TextFormat("%d", indice);
            int         ancho  = MeasureText(texto, tamano);

            DrawText(texto,
                     (int)(rec.x + (rec.width  - ancho ) / 2.0f),
                     (int)(rec.y + (rec.height - tamano) / 2.0f),
                     tamano, COLOR_TENUE);
        }
    }

    // ---- Las medidas, que es para lo que existe esta pantalla ----
    DrawText("BANCO DE PRUEBAS DEL TABLERO", 40, 16, 22, COLOR_TITULO);

    DrawText(TextFormat("Dificultad:  %s   %dx%d   (%d pares, %d cartas)",
                        nivel.nombre, nivel.filas, nivel.columnas,
                        numeroDePares(dificultadActual),
                        numeroDeCartas(dificultadActual)),
             40, 46, 18, COLOR_TEXTO);

    DrawText(TextFormat("Forma de carta:  %s", aspecto.nombre),
             40, 68, 18, COLOR_TEXTO);

    DrawText(TextFormat("Carta en pantalla:  %d x %d px          Arte al doble:  %d x %d px",
                        (int)diseno.anchoCarta, (int)diseno.altoCarta,
                        (int)(diseno.anchoCarta * 2.0f), (int)(diseno.altoCarta * 2.0f)),
             40, 90, 18, COLOR_SELECCION);

    // Diagnostico de la imagen: comparar el tamano del archivo contra el tamano al
    // que se esta dibujando dice si se esta encogiendo -que se ve bien- o estirando
    // -que se ve borroso-. Es el dato que decide a que resolucion pintar los gatos.
    if(numTexturasCargadas == 0){
        DrawText(TextFormat("Imagenes: no se encontro ninguna  (se corre desde: %s)",
                            GetWorkingDirectory()),
                 40, 112, 18, COLOR_TITULO);
    } else {
        DrawText("Imagenes:", 40, 112, 18, COLOR_TEXTO);

        for(int i = 0; i < numTexturasCargadas; i++){
            float escala = diseno.anchoCarta / (float)texturasGato[i].width * 100.0f;

            // Arriba del 100% la imagen se agranda: ya no hay pixeles que mostrar y
            // se ve borrosa. Se marca en otro color para que salte a la vista.
            Color aviso = (escala > 100.0f) ? COLOR_TITULO : COLOR_SELECCION;

            DrawText(TextFormat("%s  %dx%d  al %d%%%s",
                                GetFileNameWithoutExt(rutasCargadas[i]),
                                texturasGato[i].width, texturasGato[i].height,
                                (int)escala, (escala > 100.0f) ? " (agrandando)" : ""),
                     140 + i * 300, 112, 18, aviso);
        }
    }

    dibujarTextoCentrado("1 2 3 dificultad     A forma de carta     V voltear     ESC menu",
                         GetScreenHeight() - 38, 20, COLOR_TENUE);
}
