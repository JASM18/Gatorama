/**
 * \file Tablero.cpp
 * \brief Implementaci&oacute;n de la matriz de cartas.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Tablero.hpp"
#include "Aleatorio.hpp"

//***********************************************
// REGLA DE TRES
//***********************************************

Tablero::Tablero(int filas, int columnas)
{
    // Se valida ANTES de reservar nada. Si se hiciera al reves habria que liberar
    // el bloque antes de lanzar, y ese es justo el descuido que deja fugas.
    if(filas <= 0 || columnas <= 0)          throw TableroInvalido();
    if((filas * columnas) % 2 != 0)          throw TableroInvalido();

    this->filas    = filas;
    this->columnas = columnas;
    this->cartas   = new Carta[filas * columnas];

    Repartir();
}

Tablero::Tablero(const Tablero& otro)
{
    // Un tablero recien construido no tiene nada que liberar, asi que se copia
    // directo. El operador de asignacion si tiene que limpiar antes; esa es toda
    // la diferencia entre los dos.
    Copiar(otro);
}

Tablero& Tablero::operator=(const Tablero& otro)
{
    // Sin esta guarda, "tablero = tablero" liberaria su propia memoria y despues
    // intentaria copiar desde el puntero que acaba de quedar colgando.
    if(this == &otro) return *this;

    delete[] cartas;
    Copiar(otro);

    return *this;
}

Tablero::~Tablero()
{
    // delete[] y no delete: se reservo con new[]. Mezclarlos es comportamiento
    // indefinido aunque en la practica a veces "funcione".
    delete[] cartas;
}

void Tablero::Copiar(const Tablero& otro)
{
    filas    = otro.filas;
    columnas = otro.columnas;
    cartas   = new Carta[filas * columnas];

    // Copia carta por carta. Como Carta no maneja memoria dinamica, su operador
    // de asignacion generado por el compilador ya hace lo correcto.
    for(int i = 0; i < filas * columnas; i++){
        cartas[i] = otro.cartas[i];
    }
}

//***********************************************
// REPARTO Y BARAJADO
//***********************************************

void Tablero::Repartir()
{
    int total = filas * columnas;

    // Paso 1: acomodar las parejas en orden. La division entera hace el trabajo:
    // 0/2=0, 1/2=0, 2/2=1, 3/2=1... o sea 0,0,1,1,2,2 y asi. Cada numero aparece
    // exactamente dos veces, que es lo que define un memorama.
    for(int i = 0; i < total; i++){
        cartas[i] = Carta(i / 2);
    }

    // Paso 2: barajar con Fisher-Yates. Se recorre de atras hacia adelante y cada
    // carta se intercambia con una elegida al azar de entre las que quedan a su
    // izquierda, ella incluida.
    //
    // La razon de hacerlo asi y no "intercambiar dos cartas al azar muchas veces":
    // Fisher-Yates garantiza que los N! acomodos posibles salen todos con la misma
    // probabilidad, y lo logra en una sola pasada. El metodo de sacudir al azar no
    // reparte parejo -algunos acomodos salen mas seguido- y ademas nunca se sabe
    // cuantas sacudidas son suficientes.
    for(int i = total - 1; i > 0; i--){
        int j = aleatorio(0, i);

        Carta temporal = cartas[i];
        cartas[i]      = cartas[j];
        cartas[j]      = temporal;
    }
}

//***********************************************
// CONSULTAS
//***********************************************

int Tablero::Filas() const
{
    return filas;
}

int Tablero::Columnas() const
{
    return columnas;
}

int Tablero::NumeroDeCartas() const
{
    return filas * columnas;
}

int Tablero::NumeroDePares() const
{
    // Siempre es exacta: el constructor rechaza los tableros de total impar.
    return (filas * columnas) / 2;
}

int Tablero::Indice(int fila, int columna) const
{
    if(fila < 0 || fila >= filas)          throw FueraDeRango();
    if(columna < 0 || columna >= columnas) throw FueraDeRango();

    return fila * columnas + columna;
}

Carta& Tablero::En(int fila, int columna)
{
    return cartas[Indice(fila, columna)];
}

const Carta& Tablero::En(int fila, int columna) const
{
    return cartas[Indice(fila, columna)];
}

Carta& Tablero::EnIndice(int indice)
{
    if(indice < 0 || indice >= NumeroDeCartas()) throw FueraDeRango();

    return cartas[indice];
}

const Carta& Tablero::EnIndice(int indice) const
{
    if(indice < 0 || indice >= NumeroDeCartas()) throw FueraDeRango();

    return cartas[indice];
}

int Tablero::ParesEncontrados() const
{
    int emparejadas = 0;

    for(int i = 0; i < NumeroDeCartas(); i++){
        if(cartas[i].EstaEmparejada()) emparejadas++;
    }

    // Las cartas emparejadas siempre van de dos en dos, asi que la division es
    // exacta mientras nadie empareje una carta suelta.
    return emparejadas / 2;
}

bool Tablero::EstaResuelto() const
{
    for(int i = 0; i < NumeroDeCartas(); i++){
        if(!cartas[i].EstaEmparejada()) return false;
    }

    return true;
}
