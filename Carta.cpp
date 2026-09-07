/**
 * \file Carta.cpp
 * \brief Implementaci&oacute;n de la carta del memorama.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Carta.hpp"

//***********************************************
// CONSTRUCTORES
//***********************************************

Carta::Carta()
{
    // -1 significa "sin pareja asignada". Se usa un valor imposible en vez de 0
    // porque 0 es una pareja valida: si el reparto fallara, una carta a medio
    // inicializar se veria igualita a una del primer gato y nadie se enteraria.
    idPareja       = -1;
    estaVolteada   = false;
    estaEmparejada = false;
}

Carta::Carta(int idPareja)
{
    this->idPareja = idPareja;
    estaVolteada   = false;   // toda partida empieza con las cartas tapadas
    estaEmparejada = false;
}

//***********************************************
// CONSULTAS
//***********************************************

int Carta::IdPareja() const
{
    return idPareja;
}

bool Carta::EstaVolteada() const
{
    return estaVolteada;
}

bool Carta::EstaEmparejada() const
{
    return estaEmparejada;
}

bool Carta::HacePareja(const Carta& otra) const
{
    // Una carta sin pareja asignada no hace pareja con nadie, ni con otra igual
    // de vacia. Sin esta guarda, dos cartas recien construidas -las dos con -1-
    // se declararian pareja y el tablero se "resolveria" solo.
    if(idPareja < 0) return false;

    return idPareja == otra.idPareja;
}

//***********************************************
// CAMBIOS DE ESTADO
//***********************************************

void Carta::Voltear()
{
    // Una carta ya emparejada se queda descubierta pase lo que pase. Que ella
    // misma se defienda evita tener que acordarse de preguntar antes cada vez que
    // alguien la voltea desde otra parte del codigo.
    if(estaEmparejada) return;

    estaVolteada = true;
}

void Carta::Ocultar()
{
    if(estaEmparejada) return;

    estaVolteada = false;
}

void Carta::Emparejar()
{
    estaEmparejada = true;
    estaVolteada   = true;   // al emparejarse se queda a la vista
}
