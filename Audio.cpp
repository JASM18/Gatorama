/**
 * \file Audio.cpp
 * \brief Implementaci&oacute;n de la m&uacute;sica de fondo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

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

// Arranca en 60% y no en 100%: en un stand ruidoso la musica no debe taparle la
// voz a quien esta explicando el juego. Subirla es un deslizador; bajarla cuando
// ya esta gritando es tarde.
static float volumen = 0.6f;

void IniciarAudio()
{
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

        SetMusicVolume(musica, volumen);
        PlayMusicStream(musica);
    }

    // ---- Efectos ----
    numEventos = 0;

    for(int i = 0; i < NUM_EVENTOS; i++){
        if(!FileExists(RUTAS_EVENTOS[i])) continue;

        Sound s = LoadSound(RUTAS_EVENTOS[i]);
        if(!IsSoundValid(s)) continue;

        SetSoundVolume(s, volumen);

        eventos[numEventos] = s;
        numEventos++;
    }

    if(FileExists(RUTA_VICTORIA)){
        victoria    = LoadSound(RUTA_VICTORIA);
        hayVictoria = IsSoundValid(victoria);

        if(hayVictoria) SetSoundVolume(victoria, volumen);
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
}

void CerrarAudio()
{
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

float VolumenGeneral()
{
    return volumen;
}

void FijarVolumenGeneral(float nuevo)
{
    // Se recorta aqui, en un solo lugar, en vez de confiar en que todos los que
    // llamen manden un valor sano. El deslizador ya lo limita, pero si manana
    // alguien fija el volumen desde un archivo de configuracion mal escrito, esta
    // guarda es la que evita el ruido.
    if(nuevo < 0.0f) nuevo = 0.0f;
    if(nuevo > 1.0f) nuevo = 1.0f;

    volumen = nuevo;

    if(hayMusica) SetMusicVolume(musica, volumen);

    // Los efectos siguen el mismo volumen. Si no, bajarle al deslizador dejaria
    // la musica muda y los clics a todo lo que dan.
    for(int i = 0; i < numEventos; i++) SetSoundVolume(eventos[i], volumen);

    if(hayVictoria) SetSoundVolume(victoria, volumen);
}
