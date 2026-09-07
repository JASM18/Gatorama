/**
 * \file Partida.cpp
 * \brief Implementaci&oacute;n de las reglas del juego.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Partida.hpp"

//***********************************************
// ARMAR Y REINICIAR
//***********************************************

Partida::Partida(Dificultad nivel, int numJugadores)
    // El tablero se construye en la lista de inicializacion y no dentro del
    // cuerpo. No es capricho: Tablero no tiene constructor por omision -siempre
    // necesita medidas-, asi que no existe un momento en que este a medio armar.
    : tablero(DIFICULTADES[nivel].filas, DIFICULTADES[nivel].columnas)
{
    if(numJugadores < 1 || numJugadores > MAX_JUGADORES) throw JugadoresInvalidos();

    this->numJugadores = numJugadores;

    LimpiarMarcadores();
}

void Partida::Reiniciar()
{
    tablero.Repartir();
    LimpiarMarcadores();
}

void Partida::LimpiarMarcadores()
{
    turno    = 0;
    primera  = -1;
    segunda  = -1;
    intentos = 0;
    tiempo   = 0.0f;

    // Se limpian los dos casilleros aunque juegue uno solo. Dejar basura en el
    // del jugador que no existe es la clase de detalle que despues aparece en la
    // pantalla de resultados sin que nadie sepa de donde salio.
    for(int i = 0; i < MAX_JUGADORES; i++){
        pares[i]         = 0;
        puntos[i]        = 0;
        racha[i]         = 0;
        rachaMaxima[i]   = 0;
        tiempoJugador[i] = 0.0f;
    }
}

//***********************************************
// LAS REGLAS
//***********************************************

ResultadoVolteo Partida::Voltear(int indice)
{
    // Con un fallo sin resolver no se acepta nada: si no, se podria destapar una
    // tercera carta y quedarian tres a la vista.
    if(HayFalloPendiente())          return Volteo_ignorado;
    if(Terminada())                  return Volteo_ignorado;

    if(indice < 0 || indice >= tablero.NumeroDeCartas()) return Volteo_ignorado;

    Carta& elegida = tablero.EnIndice(indice);

    // Volver a picarle a una carta destapada no hace nada. Sin esta guarda se
    // podria "emparejar" una carta consigo misma picandole dos veces seguidas.
    if(elegida.EstaVolteada() || elegida.EstaEmparejada()) return Volteo_ignorado;

    elegida.Voltear();

    if(primera == -1){
        primera = indice;
        return Volteo_primera;
    }

    segunda = indice;
    intentos++;

    if(!tablero.EnIndice(primera).HacePareja(elegida)){
        // Las cartas se quedan destapadas. Quien las tapa es ResolverFallo, cuando
        // la pantalla decida que ya pasaron los milisegundos que tenian que pasar.
        return Volteo_fallo;
    }

    // ---- Acierto ----
    tablero.EnIndice(primera).Emparejar();
    tablero.EnIndice(segunda).Emparejar();

    pares[turno]++;
    racha[turno]++;

    // El puntaje ES el largo de la racha: la primera pareja vale 1, la segunda
    // seguida vale 2, la tercera vale 3. Por eso una racha de tres da 1+2+3 = 6
    // puntos y tres parejas sueltas dan 1+1+1 = 3. Esa diferencia es todo el
    // chiste del sistema.
    puntos[turno] += racha[turno];

    if(racha[turno] > rachaMaxima[turno]) rachaMaxima[turno] = racha[turno];

    primera = -1;
    segunda = -1;

    // El turno NO cambia: quien acierta sigue jugando. Eso es lo que hace que las
    // rachas existan.
    return Volteo_acierto;
}

bool Partida::HayFalloPendiente() const
{
    // Solo queda un par a medias cuando las dos estan destapadas. Si hubieran
    // hecho pareja, Voltear ya las habria emparejado y puesto los indices en -1.
    return primera != -1 && segunda != -1;
}

void Partida::ResolverFallo()
{
    if(!HayFalloPendiente()) return;

    tablero.EnIndice(primera).Ocultar();
    tablero.EnIndice(segunda).Ocultar();

    primera = -1;
    segunda = -1;

    // La racha se rompe aqui y no en el momento de fallar, para que el jugador
    // alcance a ver en pantalla la racha que traia justo cuando se le cae.
    racha[turno] = 0;

    // Y pasa el turno. En solitario el residuo lo deja en el mismo jugador, asi
    // que no hace falta un caso aparte para un jugador.
    turno = (turno + 1) % numJugadores;
}

void Partida::CorrerReloj(float segundos)
{
    // Con la partida terminada el reloj se detiene: el tiempo final ya es el bueno
    // y es el que va a la tabla de mejores tiempos.
    if(Terminada()) return;

    tiempo += segundos;

    // Y ademas al reloj de quien tiene el turno. Son dos cuentas y no una porque
    // responden preguntas distintas: cuanto duro la partida, y cuanto se tardo
    // cada quien. En solitario las dos dan lo mismo.
    tiempoJugador[turno] += segundos;
}

float Partida::TiempoDe(int jugador) const
{
    return JugadorValido(jugador) ? tiempoJugador[jugador] : 0.0f;
}

//***********************************************
// CONSULTAS
//***********************************************

const Tablero& Partida::ElTablero() const
{
    return tablero;
}

bool Partida::JugadorValido(int jugador) const
{
    return jugador >= 0 && jugador < numJugadores;
}

int Partida::NumJugadores() const
{
    return numJugadores;
}

int Partida::TurnoActual() const
{
    return turno;
}

int Partida::ParesDe(int jugador) const
{
    return JugadorValido(jugador) ? pares[jugador] : 0;
}

int Partida::PuntajeDe(int jugador) const
{
    return JugadorValido(jugador) ? puntos[jugador] : 0;
}

int Partida::RachaDe(int jugador) const
{
    return JugadorValido(jugador) ? racha[jugador] : 0;
}

int Partida::RachaMaximaDe(int jugador) const
{
    return JugadorValido(jugador) ? rachaMaxima[jugador] : 0;
}

int Partida::Intentos() const
{
    return intentos;
}

float Partida::Tiempo() const
{
    return tiempo;
}

bool Partida::Terminada() const
{
    return tablero.EstaResuelto();
}

int Partida::Ganador() const
{
    if(!Terminada()) return -1;

    // En solitario el unico jugador es el ganador por definicion.
    if(numJugadores == 1) return 0;

    // Gana quien junte MAS PAREJAS. El puntaje no entra en esta cuenta: es una
    // medalla para la tabla de mejores puntajes, no la condicion de victoria.
    if(pares[0] > pares[1]) return 0;
    if(pares[1] > pares[0]) return 1;

    // Con los tres tableros actuales -5, 9 y 15 parejas, todos impares- esto es
    // inalcanzable. Se deja por si algun dia se agrega un tablero de parejas
    // pares: mejor devolver -1 que declarar ganador a cualquiera.
    return -1;
}
