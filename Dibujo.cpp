/**
 * \file Dibujo.cpp
 * \brief Implementaci&oacute;n de las utilidades de dibujo compartidas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \date 29/07/2026
 */

#include "raylib.h"

#include "Dibujo.hpp"
#include "Tema.hpp"

void dibujarTextoCentrado(const char* texto, int y, int tamano, Color color)
{
    // MeasureText devuelve cuantos pixeles de ancho ocuparia ese texto con ese
    // tamano de fuente. Restarle la mitad al centro de la ventana lo centra.
    int ancho = MeasureText(texto, tamano);

    // GetScreenWidth se consulta aqui en vez de recibir el ancho por parametro:
    // asi este archivo no necesita saber que tamano de ventana eligio main.cpp,
    // y seguiria funcionando si algun dia la ventana se puede redimensionar.
    DrawText(texto, (GetScreenWidth() - ancho) / 2, y, tamano, color);
}

void dibujarPantallaPendiente(const char* nombre)
{
    dibujarTextoCentrado(nombre, 280, 50, COLOR_TITULO);
    dibujarTextoCentrado("Esta pantalla todavia no existe", 350, 22, COLOR_TEXTO);
    dibujarTextoCentrado("ESC para volver al menu", 620, 20, COLOR_TENUE);
}

// El fondo de todas las pantallas.
//
// Tiene que ser PNG: la libraylib.a que viene en el repositorio esta compilada
// SIN soporte de JPEG -SUPPORT_FILEFORMAT_JPG viene apagado de fabrica en
// raylib-, asi que un .jpg no carga y el fondo se queda en el color liso. Los
// formatos que si lee son PNG y BMP.
static const char* RUTA_FONDO_GENERAL = "recursos/fondodefault.png";

static Texture2D texturaFondoGeneral;
static bool      hayFondoGeneral = false;

void cargarFondoGeneral()
{
    hayFondoGeneral = cargarTexturaSiEsta(RUTA_FONDO_GENERAL, &texturaFondoGeneral);
}

void descargarFondoGeneral()
{
    if(hayFondoGeneral){
        UnloadTexture(texturaFondoGeneral);
        hayFondoGeneral = false;
    }
}

void dibujarFondoGeneral()
{
    if(!hayFondoGeneral) return;

    Rectangle origen = { 0.0f, 0.0f,
                         (float)texturaFondoGeneral.width,
                         (float)texturaFondoGeneral.height };

    Rectangle pantalla = { 0.0f, 0.0f,
                           (float)GetScreenWidth(), (float)GetScreenHeight() };

    Vector2 desfase = { 0.0f, 0.0f };

    // Se ajusta a la ventana en vez de dibujarse a su tamano real. Hoy dan lo
    // mismo -la imagen mide 1280x720, igual que la ventana-, pero asi nunca queda
    // una franja sin cubrir.
    DrawTexturePro(texturaFondoGeneral, origen, pantalla, desfase, 0.0f, WHITE);
}

bool cargarTexturaSiEsta(const char* ruta, Texture2D* destino)
{
    if(!FileExists(ruta)) return false;

    *destino = LoadTexture(ruta);

    if(!IsTextureValid(*destino)) return false;

    // Filtro suave: aunque casi todas las imagenes se dibujan a su tamano exacto,
    // esto evita que se vean dentadas si el rectangulo destino cambia de medida.
    SetTextureFilter(*destino, TEXTURE_FILTER_BILINEAR);

    return true;
}

//***********************************************
// TEXTO GENERADO
//***********************************************

static const char* RUTA_FUENTE = "recursos/Coconut Island.ttf";

// La fuente se hornea a 72 px y despues se reduce al dibujar. Hornearla al tamano
// exacto de cada texto obligaria a cargarla una vez por tamano; hornearla grande y
// encogerla se ve bien y ocupa una sola textura. Al reves -horneada chica y
// estirada- se veria borrosa.
static const int TAMANO_HORNEADO = 72;

// Los caracteres que se cargan: el ASCII imprimible mas lo que necesita el
// espanol. La fuente los trae todos, asi que un dia se pueden escribir acentos en
// los nombres cambiando el filtro de Configuracion.cpp.
static const int ACENTOS[] = {
    0xA1, 0xBF,                                     //  !invertido  ?invertido
    0xC1, 0xC9, 0xCD, 0xD1, 0xD3, 0xDA, 0xDC,       //  A E I N O U U con tilde
    0xE1, 0xE9, 0xED, 0xF1, 0xF3, 0xFA, 0xFC        //  a e i n o u u con tilde
};

static const int NUM_ACENTOS = sizeof(ACENTOS) / sizeof(ACENTOS[0]);
static const int NUM_ASCII   = 95;                  //  del 32 al 126

static Font fuente;
static bool hayFuente = false;

/**
 * \brief Separaci&oacute;n entre letras para un tama&ntilde;o dado.
 *
 * Va proporcional al tama&ntilde;o para que un t&iacute;tulo grande no se vea apretado ni un
 * marcador chico despegado. Es la &uacute;nica perilla del interletrado.
 *
 * \param tamano Alto de la letra en p&iacute;xeles.
 * \return Separaci&oacute;n en p&iacute;xeles.
 */
static float separacionDe(int tamano)
{
    return tamano / 16.0f;
}

void cargarFuenteDatos()
{
    hayFuente = false;

    if(!FileExists(RUTA_FUENTE)) return;

    int codigos[NUM_ASCII + NUM_ACENTOS];
    int n = 0;

    for(int c = 32; c <= 126; c++) codigos[n++] = c;
    for(int i = 0; i < NUM_ACENTOS; i++) codigos[n++] = ACENTOS[i];

    fuente    = LoadFontEx(RUTA_FUENTE, TAMANO_HORNEADO, codigos, n);
    hayFuente = IsFontValid(fuente);

    if(hayFuente){
        // Sin esto la letra se ve dentada al reducirla de 72 a los 16 o 22 px de
        // un marcador.
        SetTextureFilter(fuente.texture, TEXTURE_FILTER_BILINEAR);
    }
}

void descargarFuenteDatos()
{
    if(hayFuente){
        UnloadFont(fuente);
        hayFuente = false;
    }
}

void dibujarDato(const char* texto, int x, int y, int tamano, Color color)
{
    // Sin fuente propia se dibuja con la de fabrica. El juego se ve distinto pero
    // se puede jugar, que es lo que importa si alguien clona el repo sin recursos.
    if(!hayFuente){
        DrawText(texto, x, y, tamano, color);
        return;
    }

    Vector2 donde = { (float)x, (float)y };

    DrawTextEx(fuente, texto, donde, (float)tamano, separacionDe(tamano), color);
}

void dibujarDatoCentrado(const char* texto, int y, int tamano, Color color)
{
    dibujarDato(texto, (GetScreenWidth() - anchoDato(texto, tamano)) / 2, y, tamano, color);
}

int anchoDato(const char* texto, int tamano)
{
    if(!hayFuente) return MeasureText(texto, tamano);

    return (int)MeasureTextEx(fuente, texto, (float)tamano, separacionDe(tamano)).x;
}
