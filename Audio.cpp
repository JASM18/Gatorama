/**
 * \file Audio.cpp
 * \brief Implementaci&oacute;n de la m&uacute;sica de fondo.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "raylib.h"

#include "Audio.hpp"

// Por ahora es la unica pista y suena en todas las pantallas. Cuando haya una
// para la partida, esto se vuelve una tabla igual que la de dificultades.
static const char* RUTA_MUSICA = "recursos/menuLoop_Pink.mp3";

static Music musica;
static bool  hayMusica = false;

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
}

void ActualizarAudio()
{
    if(hayMusica) UpdateMusicStream(musica);
}

void CerrarAudio()
{
    if(hayMusica){
        UnloadMusicStream(musica);
        hayMusica = false;
    }

    // Se pregunta antes: cerrar un dispositivo que nunca abrio no tiene sentido.
    if(IsAudioDeviceReady()) CloseAudioDevice();
}

float VolumenMusica()
{
    return volumen;
}

void FijarVolumenMusica(float nuevo)
{
    // Se recorta aqui, en un solo lugar, en vez de confiar en que todos los que
    // llamen manden un valor sano. El deslizador ya lo limita, pero si manana
    // alguien fija el volumen desde un archivo de configuracion mal escrito, esta
    // guarda es la que evita el ruido.
    if(nuevo < 0.0f) nuevo = 0.0f;
    if(nuevo > 1.0f) nuevo = 1.0f;

    volumen = nuevo;

    if(hayMusica) SetMusicVolume(musica, volumen);
}
