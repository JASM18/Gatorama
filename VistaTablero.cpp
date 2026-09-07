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
#include "Aleatorio.hpp"
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
    // 30 y no 40: en Dificil las diez columnas casi se desbordaban.
    const float MARGEN_LATERAL  = 30.0f;
    const float ESPACIO_ARRIBA  = 140.0f;   // marcador, reloj y turno
    const float ESPACIO_ABAJO   = 55.0f;    // linea de ayuda

    Rectangle area;
    area.x      = MARGEN_LATERAL;
    area.y      = ESPACIO_ARRIBA;
    area.width  = GetScreenWidth()  - MARGEN_LATERAL * 2.0f;
    area.height = GetScreenHeight() - ESPACIO_ARRIBA - ESPACIO_ABAJO;

    return area;
}

Rectangle areaDeCartas()
{
    // Cuanto se meten las cartas hacia adentro del tapete, por cada lado.
    const float MARGEN_INTERIOR = 24.0f;

    Rectangle area = areaDelTablero();

    area.x      += MARGEN_INTERIOR;
    area.y      += MARGEN_INTERIOR;
    area.width  -= MARGEN_INTERIOR * 2.0f;
    area.height -= MARGEN_INTERIOR * 2.0f;

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

    // Paso 3b: aunque quepa mas grande, una carta no pasa de cierto alto. En Facil
    // hay area de sobra para diez cartas, y sin tope salian desproporcionadas.
    if(diseno.altoCarta > ALTO_MAXIMO_CARTA){
        diseno.altoCarta  = ALTO_MAXIMO_CARTA;
        diseno.anchoCarta = ALTO_MAXIMO_CARTA * relacionAspecto;
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
// Tope de cuantas ilustraciones caben. No es cuantas hay: es hasta donde se
// buscan. Subirlo no cuesta nada mientras no existan los archivos.
const int MAX_GATOS = 24;

// El tapete sobre el que se reparten las cartas. Mide lo mismo que
// areaDelTablero(), asi que hoy se dibuja sin estirarse ni un pixel.
static const char* RUTA_TABLERO = "recursos/tablero.png";

static Texture2D texturaTablero;
static bool      hayTablero = false;

// gato00 es la contraportada: la cara que se ve cuando la carta esta boca abajo.
// Va aparte de la lista de gatos porque no es una pareja, es el reverso de todas.
static const char* RUTA_DORSO = "recursos/gato00.png";

static Texture2D texturaDorso;
static bool      hayDorso = false;

// Solo se guardan las que si cargaron, apretadas al principio del arreglo. Asi
// numTexturasCargadas siempre sirve para sacar el residuo sin dejar huecos.
static Texture2D texturasGato[MAX_GATOS];
static int       numTexturasCargadas = 0;

// A que ilustracion corresponde cada pareja en la partida de ahorita. Se baraja al
// empezar cada partida: ese es el sorteo que decide CUALES gatos salen, distinto
// del barajado del tablero, que decide DONDE queda cada carta.
static int ordenIlustraciones[MAX_GATOS];

void cargarTexturasTablero()
{
    numTexturasCargadas = 0;

    hayTablero = cargarTexturaSiEsta(RUTA_TABLERO, &texturaTablero);

    // El dorso primero: es el que se ve al empezar la partida.
    hayDorso = false;

    if(FileExists(RUTA_DORSO)){
        texturaDorso = LoadTexture(RUTA_DORSO);
        hayDorso     = IsTextureValid(texturaDorso);

        if(hayDorso){
            GenTextureMipmaps(&texturaDorso);
            SetTextureFilter(texturaDorso, TEXTURE_FILTER_TRILINEAR);
        }
    }

    // Se buscan gato01, gato02, gato03... hasta que falte alguno. Antes la lista
    // estaba escrita a mano en el codigo; asi, agregar gato16 es dejar el archivo
    // en la carpeta y ya: ni recompilar ni editar nada.
    for(int n = 1; n <= MAX_GATOS; n++){

        // TextFormat arma la ruta con el cero adelante: gato01, no gato1. Por eso
        // los archivos se nombraron asi desde el principio.
        const char* ruta = TextFormat("recursos/gato%02d.png", n);

        // El primer hueco corta la busqueda. Si faltara gato07, los de despues
        // tampoco se cargan: mejor eso que una baraja con agujeros silenciosos
        // donde dos parejas distintas comparten dibujo.
        if(!FileExists(ruta)) break;

        Texture2D tex = LoadTexture(ruta);
        if(!IsTextureValid(tex)) break;

        // Una mipmap es la misma imagen guardada ya reducida a la mitad, a la
        // cuarta parte, y asi. Sin ellas, dibujar una imagen de 320 px a 102 obliga
        // a la tarjeta a saltarse pixeles del original, y el resultado tiembla y se
        // ve dentado. Con mipmaps toma la copia del tamano mas cercano.
        GenTextureMipmaps(&tex);

        // TRILINEAR es el filtro que sabe usar esas copias: mezcla las dos mas
        // cercanas. BILINEAR sin mipmaps solo promedia vecinos del original, que
        // alcanza para reducciones chicas pero no para bajar de 320 a 102.
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);

        texturasGato[numTexturasCargadas] = tex;
        numTexturasCargadas++;
    }

    // Deja un orden valido desde el arranque, por si algo dibuja antes de que
    // empiece la primera partida.
    barajarIlustraciones();
}

void barajarIlustraciones()
{
    for(int i = 0; i < numTexturasCargadas; i++) ordenIlustraciones[i] = i;

    // Fisher-Yates, el mismo que reparte las cartas en Tablero::Repartir. Como el
    // tablero toma las parejas 0, 1, 2... en orden, barajar esta lista equivale a
    // sacar al azar tantos gatos como parejas haga falta, sin repetir.
    for(int i = numTexturasCargadas - 1; i > 0; i--){
        int j = aleatorio(0, i);

        int temporal          = ordenIlustraciones[i];
        ordenIlustraciones[i] = ordenIlustraciones[j];
        ordenIlustraciones[j] = temporal;
    }
}

int ilustracionDePareja(int idPareja)
{
    if(numTexturasCargadas <= 0 || idPareja < 0) return 0;

    // El residuo protege el caso de que falten gatos: con menos ilustraciones que
    // parejas se repiten, que es feo pero no truena, y el aviso de la pantalla de
    // juego ya le dice al equipo lo que pasa.
    return ordenIlustraciones[idPareja % numTexturasCargadas];
}

int numeroDeIlustraciones()
{
    return numTexturasCargadas;
}

void descargarTexturasTablero()
{
    if(hayTablero){
        UnloadTexture(texturaTablero);
        hayTablero = false;
    }

    if(hayDorso){
        UnloadTexture(texturaDorso);
        hayDorso = false;
    }

    for(int i = 0; i < numTexturasCargadas; i++){
        UnloadTexture(texturasGato[i]);
    }

    numTexturasCargadas = 0;
}

void dibujarDorsoCarta(Rectangle rec, bool resaltada)
{
    float grosor = resaltada ? 4.0f : 2.0f;

    if(!hayDorso){
        // Sin contraportada se dibuja el color liso de antes.
        Color relleno = resaltada ? COLOR_CARTA_HOVER : COLOR_CARTA_DORSO;

        DrawRectangleRounded(rec, REDONDEZ, SEGMENTOS, relleno);
        DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
        return;
    }

    Rectangle origen  = { 0.0f, 0.0f, (float)texturaDorso.width, (float)texturaDorso.height };
    Vector2   desfase = { 0.0f, 0.0f };

    DrawTexturePro(texturaDorso, origen, rec, desfase, 0.0f, WHITE);

    // El resaltado ya no puede ser un color de relleno distinto: ahora hay una
    // imagen encima que lo taparia. Se le echa un velo blanco translucido, que
    // aclara la carta sin ocultarla. Fade solo le baja la opacidad a un color.
    if(resaltada){
        DrawRectangleRounded(rec, REDONDEZ, SEGMENTOS, Fade(WHITE, 0.20f));
    }

    DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
}

void dibujarFondoTablero()
{
    if(!hayTablero) return;

    Rectangle area   = areaDelTablero();
    Rectangle origen = { 0.0f, 0.0f, (float)texturaTablero.width, (float)texturaTablero.height };
    Vector2   desfase = { 0.0f, 0.0f };

    // Se ajusta al area en vez de dibujarse a su tamano real. Hoy dan lo mismo
    // -la imagen mide 1200x525, igual que el area-, pero si alguien cambia los
    // margenes el tapete la sigue en vez de dejar una franja sin cubrir.
    DrawTexturePro(texturaTablero, origen, area, desfase, 0.0f, WHITE);
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
    DrawRectangleRoundedLinesEx(rec, REDONDEZ, SEGMENTOS, grosor, COLOR_CARTA_BORDE);
}
