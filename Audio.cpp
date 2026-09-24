/**
 * \file Audio.cpp
 * \brief Implementaci&oacute;n de la m&uacute;sica de fondo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <cstdio>
#include <cstring>

#include "raylib.h"

#include "Audio.hpp"
#include "Aleatorio.hpp"

// Por ahora es la unica pista y suena en todas las pantallas. Cuando haya una
// para la partida, esto se vuelve una tabla igual que la de dificultades.
static const char* RUTA_MUSICA = "recursos/menuLoop_Pink.mp3";

static Music musica;
static bool  hayMusica = false;

// Los efectos cortos se cargan como Sound y no como Music: un Sound se decodifica
// entero a memoria y suena de inmediato, mientras que un Music se va leyendo del
// archivo. Para medio segundo de sonido, esperar a que se lea del disco es
// justamente el retraso que hace sentir el juego lento.
const int NUM_EVENTOS = 2;

// Son .wav y no .mp3: los archivos siempre fueron WAV, solo traian la extension
// equivocada. raylib elige el decodificador por la extension, asi que un WAV
// llamado .mp3 se le pasa al decodificador de MP3 y sale ruido -o nada-.
static const char* RUTAS_EVENTOS[NUM_EVENTOS] = {
    "recursos/event1.wav",
    "recursos/event2.wav"
};

static const char* RUTA_VICTORIA = "recursos/victoria.mp3";

static Sound eventos[NUM_EVENTOS];
static int   numEventos = 0;

static Sound victoria;
static bool  hayVictoria = false;

// La musica arranca en 60% y no en 100%: en un stand ruidoso no debe taparle la
// voz a quien esta explicando el juego. Los efectos arrancan mas alto porque su
// trabajo es justamente hacerse oir por encima de ella.
static float volumenMusica  = 0.6f;
static float volumenEfectos = 0.8f;

//***********************************************
// ARCHIVO DE AJUSTES
//***********************************************

// El volumen elegido se guarda aqui para que sobreviva a cerrar el juego. Va junto
// al ejecutable, como puntajes.json, y es texto plano a proposito: si algo sale
// mal en el stand, se abre con el bloc de notas y se corrige a mano.
//
//     musica=0.60
//     efectos=0.80
//
// Si el archivo no esta o trae basura, se usan los valores de arriba y ya: nunca
// es motivo para que el juego no arranque.
static const char* RUTA_AJUSTES = "settings.txt";

// El volumen se guarda un rato despues del ultimo cambio y no en cada uno: una
// barra arrastrada cambia el valor sesenta veces por segundo, y escribir el
// archivo otras tantas no tiene sentido. Medio segundo quieto basta.
static const float ESPERA_GUARDAR = 0.5f;

static bool  hayCambioSinGuardar = false;
static float esperaGuardar       = 0.0f;

static float recortar(float v);

/**
 * \brief Busca "clave=valor" en el texto del archivo.
 *
 * \param texto Todo el contenido de settings.txt.
 * \param clave Nombre del ajuste, sin el signo de igual.
 * \param valor D&oacute;nde dejar el n&uacute;mero si se encontr&oacute;; si no, no se toca.
 */
static void leerAjuste(const char* texto, const char* clave, float* valor)
{
    const char* donde = strstr(texto, clave);
    if(donde == 0) return;

    float leido;

    // El formato arma "musica=%f": sscanf solo acepta la linea si trae el igual y
    // un numero despues. Cualquier otra cosa deja el valor como estaba.
    char formato[32];
    snprintf(formato, sizeof(formato), "%s=%%f", clave);

    if(sscanf(donde, formato, &leido) == 1) *valor = recortar(leido);
}

/**
 * \brief Lee el volumen de settings.txt, si existe.
 */
static void cargarAjustes()
{
    if(!FileExists(RUTA_AJUSTES)) return;

    char* texto = LoadFileText(RUTA_AJUSTES);
    if(texto == 0) return;

    leerAjuste(texto, "musica",  &volumenMusica);
    leerAjuste(texto, "efectos", &volumenEfectos);

    UnloadFileText(texto);
}

/**
 * \brief Escribe el volumen actual en settings.txt.
 */
static void guardarAjustes()
{
    const char* texto = TextFormat("# Gatorama - se escribe solo al cambiar el volumen\n"
                                   "musica=%.2f\n"
                                   "efectos=%.2f\n",
                                   volumenMusica, volumenEfectos);

    // SaveFileText pide un char* aunque no lo modifica.
    SaveFileText(RUTA_AJUSTES, (char*)texto);

    hayCambioSinGuardar = false;
}

/**
 * \brief Anota que el volumen cambi&oacute;, para guardarlo en cuanto se quede quieto.
 */
static void avisarCambio()
{
    hayCambioSinGuardar = true;
    esperaGuardar       = ESPERA_GUARDAR;
}

void IniciarAudio()
{
    // Primero el archivo: los volumenes leidos son los que se les ponen a la
    // musica y a los efectos al cargarlos, aqui abajo.
    cargarAjustes();

    InitAudioDevice();

    // Una maquina prestada puede no tener salida de sonido. No es motivo para no
    // poder jugar, asi que se sale en silencio y el resto del juego ni se entera.
    if(!IsAudioDeviceReady()) return;

    if(!FileExists(RUTA_MUSICA)) return;

    musica    = LoadMusicStream(RUTA_MUSICA);
    hayMusica = IsMusicValid(musica);

    if(hayMusica){
        // Sin esto la pista suena una vez y se acaba. El archivo se llama "Loop"
        // justamente porque esta pensado para repetirse sin costura.
        musica.looping = true;

        SetMusicVolume(musica, volumenMusica);
        PlayMusicStream(musica);
    }

    // ---- Efectos ----
    numEventos = 0;

    for(int i = 0; i < NUM_EVENTOS; i++){
        if(!FileExists(RUTAS_EVENTOS[i])) continue;

        Sound s = LoadSound(RUTAS_EVENTOS[i]);
        if(!IsSoundValid(s)) continue;

        SetSoundVolume(s, volumenEfectos);

        eventos[numEventos] = s;
        numEventos++;
    }

    if(FileExists(RUTA_VICTORIA)){
        victoria    = LoadSound(RUTA_VICTORIA);
        hayVictoria = IsSoundValid(victoria);

        if(hayVictoria) SetSoundVolume(victoria, volumenEfectos);
    }
}

void ReproducirEvento()
{
    if(numEventos <= 0) return;

    // Se reusa el aleatorio del equipo, el mismo que baraja las cartas.
    PlaySound(eventos[aleatorio(0, numEventos - 1)]);
}

void ReproducirVictoria()
{
    if(hayVictoria) PlaySound(victoria);
}

void ActualizarAudio()
{
    if(hayMusica) UpdateMusicStream(musica);

    // Se espera a que la barra se suelte y pase medio segundo sin cambios.
    if(hayCambioSinGuardar && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        esperaGuardar -= GetFrameTime();

        if(esperaGuardar <= 0.0f) guardarAjustes();
    }
}

void CerrarAudio()
{
    // Si se cierra el juego justo despues de mover una barra, el cambio no se pierde.
    if(hayCambioSinGuardar) guardarAjustes();

    for(int i = 0; i < numEventos; i++) UnloadSound(eventos[i]);
    numEventos = 0;

    if(hayVictoria){
        UnloadSound(victoria);
        hayVictoria = false;
    }

    if(hayMusica){
        UnloadMusicStream(musica);
        hayMusica = false;
    }

    // Se pregunta antes: cerrar un dispositivo que nunca abrio no tiene sentido.
    if(IsAudioDeviceReady()) CloseAudioDevice();
}

/**
 * \brief Recorta un volumen al rango que acepta raylib.
 *
 * Se hace aqui, en un solo lugar, en vez de confiar en que todos los que llamen
 * manden un valor sano. Los deslizadores ya lo limitan, pero si manana alguien
 * fija el volumen desde un archivo de configuracion mal escrito, esta guarda es la
 * que evita el ruido.
 *
 * \param v Valor pedido.
 * \return El mismo valor, ya dentro de 0.0 a 1.0.
 */
static float recortar(float v)
{
    if(v < 0.0f) return 0.0f;
    if(v > 1.0f) return 1.0f;

    return v;
}

float VolumenMusica()
{
    return volumenMusica;
}

void FijarVolumenMusica(float nuevo)
{
    nuevo = recortar(nuevo);

    // Las barras llaman aqui en cada fotograma aunque nadie las toque; solo un
    // cambio de verdad cuenta para guardar.
    if(nuevo == volumenMusica) return;

    volumenMusica = nuevo;
    avisarCambio();

    if(hayMusica) SetMusicVolume(musica, volumenMusica);
}

float VolumenEfectos()
{
    return volumenEfectos;
}

void FijarVolumenEfectos(float nuevo)
{
    nuevo = recortar(nuevo);

    if(nuevo == volumenEfectos) return;

    volumenEfectos = nuevo;
    avisarCambio();

    for(int i = 0; i < numEventos; i++) SetSoundVolume(eventos[i], volumenEfectos);

    if(hayVictoria) SetSoundVolume(victoria, volumenEfectos);
}
