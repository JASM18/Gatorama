/**
 * \file Monticulo.cpp
 * \brief Implementaci&oacute;n del mont&iacute;culo de registros.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Monticulo.hpp"

//***********************************************
// REGLA DE TRES
//***********************************************

Monticulo::Monticulo(int capacidad, Comparador esMejor)
{
    // Se valida antes de reservar: si se hiciera al reves habria que liberar el
    // arreglo antes de lanzar, y ahi es donde se cuelan las fugas.
    if(capacidad <= 0) throw Lleno();

    this->capacidad = capacidad;
    this->esMejor   = esMejor;
    this->cantidad  = 0;
    this->datos     = new Puntaje[capacidad];
}

Monticulo::Monticulo(const Monticulo& otro)
{
    Copiar(otro);
}

Monticulo& Monticulo::operator=(const Monticulo& otro)
{
    // Sin esta guarda, "m = m" liberaria su propia memoria y despues copiaria
    // desde el puntero que acaba de quedar colgando.
    if(this == &otro) return *this;

    delete[] datos;
    Copiar(otro);

    return *this;
}

Monticulo::~Monticulo()
{
    delete[] datos;
}

void Monticulo::Copiar(const Monticulo& otro)
{
    capacidad = otro.capacidad;
    cantidad  = otro.cantidad;
    esMejor   = otro.esMejor;
    datos     = new Puntaje[capacidad];

    for(int i = 0; i < cantidad; i++){
        datos[i] = otro.datos[i];
    }
}

//***********************************************
// ACOMODO INTERNO
//***********************************************

// El arbol vive en el arreglo sin punteros: el hijo izquierdo del lugar i esta en
// 2i+1, el derecho en 2i+2, y el padre de i en (i-1)/2. Esa es toda la magia del
// monticulo binario, y es por lo que no hace falta reservar nodo por nodo.

void Monticulo::Subir(int posicion)
{
    while(posicion > 0){

        int padre = (posicion - 1) / 2;

        // Si el padre ya es mejor, este elemento llego a su lugar.
        if(esMejor(datos[padre], datos[posicion])) return;

        Puntaje temporal = datos[padre];
        datos[padre]     = datos[posicion];
        datos[posicion]  = temporal;

        posicion = padre;
    }
}

void Monticulo::Bajar(int posicion)
{
    while(true){

        int izquierdo = 2 * posicion + 1;
        int derecho   = 2 * posicion + 2;
        int mejor     = posicion;

        // Se busca cual de los tres -el, su hijo izquierdo y su hijo derecho- debe
        // quedar arriba. Hay que revisar los dos hijos: quedarse con el primero
        // que sea mejor rompe la propiedad del monticulo.
        if(izquierdo < cantidad && esMejor(datos[izquierdo], datos[mejor])) mejor = izquierdo;
        if(derecho   < cantidad && esMejor(datos[derecho],   datos[mejor])) mejor = derecho;

        if(mejor == posicion) return;

        Puntaje temporal = datos[mejor];
        datos[mejor]     = datos[posicion];
        datos[posicion]  = temporal;

        posicion = mejor;
    }
}

//***********************************************
// OPERACIONES
//***********************************************

void Monticulo::Insertar(const Puntaje& p)
{
    if(cantidad >= capacidad) throw Lleno();

    // Entra al final y sube hasta su lugar. Como el arbol siempre esta completo,
    // ese camino tiene a lo mucho log n pasos.
    datos[cantidad] = p;
    cantidad++;

    Subir(cantidad - 1);
}

Puntaje Monticulo::Sacar()
{
    if(cantidad == 0) throw Vacio();

    Puntaje mejor = datos[0];

    // El ultimo se sube a la raiz y baja hasta acomodarse. Es el truco que deja el
    // arbol completo sin tener que recorrer nada.
    cantidad--;
    datos[0] = datos[cantidad];

    if(cantidad > 0) Bajar(0);

    return mejor;
}

bool Monticulo::EstaVacio() const
{
    return cantidad == 0;
}

int Monticulo::Cantidad() const
{
    return cantidad;
}
