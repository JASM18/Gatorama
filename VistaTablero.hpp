/**
 * \file VistaTablero.hpp
 * \brief Acomodo y dibujo del tablero de cartas en pantalla.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \author Helleon Cardenas, Alba Rosa
 * \author Chenoweth Galaz, Ivana Lin
 * \author Dur&oacute;n Urbina, &Aacute;ngel Daniel
 * \date 06/09/2026
 *
 * Este m&oacute;dulo es **vista**: piensa en p&iacute;xeles, no en reglas. No sabe qu&eacute; gato
 * trae cada carta ni de qui&eacute;n es el turno; solo sabe cu&aacute;ntas casillas hay y
 * d&oacute;nde cae cada una. El modelo del juego se le pasar&aacute; m&aacute;s adelante.
 */

#ifndef VISTATABLERO_HPP_INCLUDED
#define VISTATABLERO_HPP_INCLUDED

#include "raylib.h"

/**
 * \brief Forma de la carta: ancho entre alto.
 *
 * El equipo eligi&oacute; 2:3 -proporci&oacute;n de carta de baraja- despu&eacute;s de
 * compararla en pantalla contra 3:4 y 1:1. Todo el arte se dibuja con esta
 * relaci&oacute;n, as&iacute; que cambiarla obligar&iacute;a a rehacer las ilustraciones.
 */
const float ANCHO_ARTE = 347.0f;   ///< Ancho de las ilustraciones, en p&iacute;xeles
const float ALTO_ARTE  = 480.0f;   ///< Alto de las ilustraciones, en p&iacute;xeles

const float RELACION_CARTA = ANCHO_ARTE / ALTO_ARTE;

/**
 * \brief Alto m&aacute;ximo de una carta en pantalla, en p&iacute;xeles.
 *
 * Sin este tope, en F&aacute;cil las diez cartas se reparten un &aacute;rea enorme y salen
 * gigantes. El tope solo muerde ah&iacute;: en Normal y en Dif&iacute;cil las cartas ya son m&aacute;s
 * chicas porque tienen que caber m&aacute;s.
 */
const float ALTO_MAXIMO_CARTA = 200.0f;

/**
 * \brief Resultado de acomodar una cuadr&iacute;cula de cartas dentro de un &aacute;rea.
 *
 * Se calcula una vez por fotograma y de &eacute;l salen todas las posiciones. Guardarlo
 * en vez de recalcular carta por carta evita que dos partes del c&oacute;digo lleguen a
 * cuentas distintas.
 */
struct DisenoTablero {
    int   filas;        ///< Renglones de la cuadr&iacute;cula
    int   columnas;     ///< Cartas por renglon
    float anchoCarta;   ///< Ancho de una carta en p&iacute;xeles
    float altoCarta;    ///< Alto de una carta en p&iacute;xeles
    float separacion;   ///< Hueco entre dos cartas vecinas, en p&iacute;xeles
    float pasoX;        ///< Cu&aacute;nto avanzar en X de una carta a la siguiente
    float pasoY;        ///< Cu&aacute;nto avanzar en Y de un rengl&oacute;n al siguiente
    float origenX;      ///< Esquina izquierda de la carta (0,0)
    float origenY;      ///< Esquina superior de la carta (0,0)
};

/**
 * \brief El &aacute;rea de la ventana donde cabe el tablero.
 *
 * Descuenta el espacio de arriba (marcador y reloj) y el de abajo (ayudas), para
 * que el tablero nunca se encime con la interfaz.
 *
 * \return Rect&aacute;ngulo disponible para las cartas.
 */
Rectangle areaDelTablero();

/**
 * \brief El &aacute;rea, ya metida hacia adentro, donde se reparten las cartas.
 *
 * El tapete trae su propio marco dibujado, as&iacute; que las cartas no pueden usarlo
 * completo: se meten un poco para no encimarse con &eacute;l. Sin este respiro, en Dif&iacute;cil
 * la primera y la &uacute;ltima columna quedaban pegadas al borde de la imagen.
 *
 * Ensanchar el tapete solo no bastaba: las cartas crecen con &eacute;l y vuelven a
 * pegarse. Lo que da aire es este margen de adentro.
 *
 * \return Rect&aacute;ngulo disponible para las cartas.
 */
Rectangle areaDeCartas();

/**
 * \brief Calcula d&oacute;nde y de qu&eacute; tama&ntilde;o van las cartas dentro de un &aacute;rea.
 *
 * Reparte el &aacute;rea en celdas iguales y mete la carta m&aacute;s grande que quepa en una
 * celda **respetando su forma**. El lado que se queda corto es el que manda: por eso
 * un tablero ancho y bajo da cartas limitadas por el alto, y uno alto y angosto las
 * limita por el ancho.
 *
 * \param filas           Renglones de la cuadr&iacute;cula (mayor que cero).
 * \param columnas        Cartas por rengl&oacute;n (mayor que cero).
 * \param area            Rect&aacute;ngulo de pantalla donde debe caber todo.
 * \param relacionAspecto Ancho dividido entre alto de la carta (0.6667 para 2:3, 1.0 para cuadrada).
 * \return El acomodo ya resuelto.
 */
DisenoTablero calcularDiseno(int filas, int columnas, Rectangle area, float relacionAspecto);

/**
 * \brief D&oacute;nde queda una carta concreta de la cuadr&iacute;cula.
 *
 * \param diseno  Acomodo devuelto por calcularDiseno().
 * \param fila    Rengl&oacute;n de la carta, empezando en cero.
 * \param columna Columna de la carta, empezando en cero.
 * \return Rect&aacute;ngulo de esa carta en coordenadas de pantalla.
 */
Rectangle rectanguloDeCarta(const DisenoTablero& diseno, int fila, int columna);

/**
 * \brief Qu&eacute; carta est&aacute; debajo de un punto de la pantalla.
 *
 * Es lo que convierte un clic del rat&oacute;n en una carta del juego. Se hace aqu&iacute;, en
 * la vista, porque es una pregunta de p&iacute;xeles: el modelo no sabe d&oacute;nde se dibuj&oacute;
 * nada.
 *
 * \param diseno Acomodo devuelto por calcularDiseno().
 * \param punto  Posici&oacute;n en pantalla, normalmente GetMousePosition().
 * \return &Iacute;ndice de la carta (fila * columnas + columna), o -1 si el punto cay&oacute; en un hueco.
 */
int indiceCartaEnPunto(const DisenoTablero& diseno, Vector2 punto);

/**
 * \brief Dibuja el dorso de una carta (la cara oculta).
 *
 * \param rec       D&oacute;nde va, normalmente de rectanguloDeCarta().
 * \param resaltada Verdadero si el rat&oacute;n est&aacute; encima; se dibuja distinto para que
 *                  el jugador vea qu&eacute; va a escoger antes de hacer clic.
 */
void dibujarDorsoCarta(Rectangle rec, bool resaltada);

/**
 * \brief Dibuja el anillo que marca en qu&eacute; carta est&aacute; parado el teclado.
 *
 * Va aparte del resaltado de dibujarDorsoCarta a prop&oacute;sito: aquel aclara la carta
 * y por eso desaparece cuando la carta ya est&aacute; destapada, que es justo cuando el
 * jugador se perd&iacute;a. El anillo se dibuja siempre; lo que cambia es el color.
 *
 * \param rec       D&oacute;nde va, normalmente de rectanguloDeCarta().
 * \param volteable Verdadero si darle Enter ah&iacute; hace algo. En morado se ve que la
 *                  carta responde; en gris, que el cursor solo est&aacute; de paso.
 */
void dibujarCursorCarta(Rectangle rec, bool volteable);

/**
 * \brief Carga las ilustraciones de los gatos.
 *
 * **Tiene que llamarse despu&eacute;s de InitWindow().** Una textura vive en la memoria de
 * la tarjeta de video, y esa memoria no existe hasta que hay una ventana con su
 * contexto de OpenGL. Llamarla antes devuelve una textura vac&iacute;a sin avisar.
 *
 * Los archivos que falten simplemente se saltan: el juego se puede seguir probando
 * con las ilustraciones que s&iacute; existan, o sin ninguna.
 */
void cargarTexturasTablero();

/**
 * \brief Dibuja el tapete del tablero, si hay imagen.
 *
 * Va **antes** que las cartas, porque es el fondo sobre el que se reparten. Cubre
 * exactamente areaDelTablero(); si no hay imagen no dibuja nada y el tablero se ve
 * sobre el fondo de la pantalla, como antes.
 */
void dibujarFondoTablero();

/**
 * \brief Sortea qu&eacute; gatos le tocan a las parejas de esta partida.
 *
 * El modelo reparte parejas numeradas 0, 1, 2&hellip; y nunca sabe de dibujos. Sin este
 * sorteo, la pareja 0 siempre mostraba el gato01 y la 1 el gato02, as&iacute; que en
 * F&aacute;cil se jugaba **siempre con los mismos cinco gatos** aunque cambiaran de
 * lugar. Barajar la correspondencia hace que cada partida traiga una selecci&oacute;n
 * distinta de la baraja completa.
 *
 * Se llama al empezar y al reiniciar una partida.
 */
void barajarIlustraciones();

/**
 * \brief Qu&eacute; gato le toca a una pareja en esta partida.
 *
 * \param idPareja N&uacute;mero de pareja que da el modelo.
 * \return &Iacute;ndice de la ilustraci&oacute;n que le corresponde.
 */
int ilustracionDePareja(int idPareja);

/**
 * \brief Cu&aacute;ntas ilustraciones se lograron cargar.
 * \return N&uacute;mero de im&aacute;genes disponibles; cero si no se carg&oacute; ninguna.
 */
int numeroDeIlustraciones();

/**
 * \brief Libera las ilustraciones.
 *
 * **Tiene que llamarse antes de CloseWindow()**, por la misma raz&oacute;n: despu&eacute;s de
 * cerrar la ventana ya no hay a qui&eacute;n devolverle esa memoria.
 */
void descargarTexturasTablero();

/**
 * \brief Dibuja la cara descubierta de una carta, con su ilustraci&oacute;n.
 *
 * La imagen se ajusta al rect&aacute;ngulo que se le d&eacute;. Si no hay ilustraciones cargadas
 * dibuja un relleno liso, para que el juego se pueda probar sin arte.
 *
 * \param rec                D&oacute;nde va, normalmente de rectanguloDeCarta().
 * \param resaltada          Verdadero si el rat&oacute;n est&aacute; encima.
 * \param indiceIlustracion  Qu&eacute; gato dibujar. Se toma el residuo entre el n&uacute;mero de
 *                           ilustraciones cargadas, as&iacute; que cualquier entero es v&aacute;lido.
 */
void dibujarCaraCarta(Rectangle rec, bool resaltada, int indiceIlustracion);

#endif // VISTATABLERO_HPP_INCLUDED
