/**
 * \file Audio.hpp
 * \brief M&uacute;sica de fondo y su volumen.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Todo lo de sonido pasa por aqu&iacute;. Ninguna pantalla habla directamente con raylib
 * para esto: piden subir o bajar el volumen y ya. As&iacute;, el d&iacute;a que se agreguen
 * efectos -el clic de una carta, el sonido de acierto- se agregan en un solo lugar.
 *
 * Si no hay tarjeta de sonido, o el archivo no est&aacute;, el juego corre igual en
 * silencio. Nadie se queda sin jugar por eso.
 */

#ifndef AUDIO_HPP_INCLUDED
#define AUDIO_HPP_INCLUDED

/**
 * \brief Abre el dispositivo de sonido y arranca la m&uacute;sica.
 *
 * Se llama una vez al inicio, despu&eacute;s de InitWindow().
 */
void IniciarAudio();

/**
 * \brief Le da de comer al reproductor de m&uacute;sica (una llamada por fotograma).
 *
 * La m&uacute;sica no se carga entera a memoria: se va leyendo del archivo por pedazos
 * mientras suena. Esta llamada es la que rellena el siguiente pedazo. **Si se deja
 * de llamar, la m&uacute;sica se corta**, aunque el archivo siga cargado.
 */
void ActualizarAudio();

/**
 * \brief Libera la m&uacute;sica y cierra el dispositivo de sonido.
 *
 * Se llama antes de CloseWindow().
 */
void CerrarAudio();

/**
 * \brief Suena uno de los efectos de evento, elegido al azar.
 *
 * Hay dos y se echa un volado entre ellos. Es un detalle chico con un efecto
 * grande: un mismo clic que siempre suena igual se vuelve monotono en un stand
 * donde el juego se usa cien veces seguidas.
 */
void ReproducirEvento();

/**
 * \brief Suena el efecto de victoria.
 */
void ReproducirVictoria();

// Hay dos volumenes y no uno porque son dos cosas con usos distintos: la musica
// acompana de fondo y los efectos avisan que algo paso. En un stand ruidoso lo
// normal es querer la musica baja y los efectos altos, y con un solo control eso
// no se puede.

/**
 * \brief El volumen de la m&uacute;sica de fondo.
 * \return Un valor de 0.0 (mudo) a 1.0 (m&aacute;ximo).
 */
float VolumenMusica();

/**
 * \brief Cambia el volumen de la m&uacute;sica de fondo.
 *
 * Valores fuera de rango se recortan, para que un deslizador mal calculado no
 * reviente el sonido.
 *
 * \param volumen De 0.0 a 1.0.
 */
void FijarVolumenMusica(float volumen);

/**
 * \brief El volumen de los efectos.
 * \return Un valor de 0.0 (mudo) a 1.0 (m&aacute;ximo).
 */
float VolumenEfectos();

/**
 * \brief Cambia el volumen de los efectos.
 * \param volumen De 0.0 a 1.0.
 */
void FijarVolumenEfectos(float volumen);

#endif // AUDIO_HPP_INCLUDED
