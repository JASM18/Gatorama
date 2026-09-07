/**
 * \file Puntaje.cpp
 * \brief Implementaci&oacute;n de las comparaciones entre registros.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include "Puntaje.hpp"

//***********************************************
// COMPARADORES
//***********************************************

// Hay uno por cada columna y direccion. Todos llevan un desempate: sin el, dos
// registros iguales en la columna elegida quedarian en el orden en que los acomodo
// el monticulo, y la tabla se veria distinta cada vez que se abre sin que nada
// haya cambiado.

static bool puntosDesc(const Puntaje& a, const Puntaje& b)
{
    if(a.puntos != b.puntos) return a.puntos > b.puntos;
    if(a.racha  != b.racha)  return a.racha  > b.racha;

    return a.tiempo < b.tiempo;
}

static bool puntosAsc(const Puntaje& a, const Puntaje& b)
{
    if(a.puntos != b.puntos) return a.puntos < b.puntos;
    if(a.racha  != b.racha)  return a.racha  < b.racha;

    return a.tiempo < b.tiempo;
}

static bool rachaDesc(const Puntaje& a, const Puntaje& b)
{
    if(a.racha != b.racha) return a.racha > b.racha;

    return a.puntos > b.puntos;
}

static bool rachaAsc(const Puntaje& a, const Puntaje& b)
{
    if(a.racha != b.racha) return a.racha < b.racha;

    return a.puntos > b.puntos;
}

static bool paresDesc(const Puntaje& a, const Puntaje& b)
{
    if(a.pares != b.pares) return a.pares > b.pares;

    return a.puntos > b.puntos;
}

static bool paresAsc(const Puntaje& a, const Puntaje& b)
{
    if(a.pares != b.pares) return a.pares < b.pares;

    return a.puntos > b.puntos;
}

static bool tiempoAsc(const Puntaje& a, const Puntaje& b)
{
    if(a.tiempo != b.tiempo) return a.tiempo < b.tiempo;

    return a.puntos > b.puntos;
}

static bool tiempoDesc(const Puntaje& a, const Puntaje& b)
{
    if(a.tiempo != b.tiempo) return a.tiempo > b.tiempo;

    return a.puntos > b.puntos;
}

//***********************************************
// TABLAS POR COLUMNA
//***********************************************

// El indice de la columna es el valor del enum, y el segundo indice es la
// direccion: [0] ascendente, [1] descendente. Agregar una columna es agregar un
// renglon aqui, otro en TITULOS y subirle uno a NUM_COLUMNAS_ORDEN.
static const ComparadorPuntaje COMPARADORES[NUM_COLUMNAS_ORDEN][2] = {
    { puntosAsc, puntosDesc },
    { rachaAsc,  rachaDesc  },
    { paresAsc,  paresDesc  },
    { tiempoAsc, tiempoDesc }
};

static const char* TITULOS[NUM_COLUMNAS_ORDEN] = {
    "Puntos",
    "Racha",
    "Pares",
    "Tiempo"
};

// Que direccion se toma al estrenar una columna. En Puntos, Racha y Pares uno
// quiere ver primero al mas alto; en Tiempo, al mas bajo. Poner lo contrario
// obligaria a hacer dos clics para ver lo obvio.
static const bool DESCENDENTE_POR_OMISION[NUM_COLUMNAS_ORDEN] = {
    true,    // Puntos
    true,    // Racha
    true,    // Pares
    false    // Tiempo
};

const char* tituloDeColumna(ColumnaPuntaje columna)
{
    if(columna < 0 || columna >= NUM_COLUMNAS_ORDEN) return TITULOS[Columna_puntos];

    return TITULOS[columna];
}

bool ordenDescendentePorOmision(ColumnaPuntaje columna)
{
    if(columna < 0 || columna >= NUM_COLUMNAS_ORDEN) return true;

    return DESCENDENTE_POR_OMISION[columna];
}

ComparadorPuntaje comparadorDe(ColumnaPuntaje columna, bool descendente)
{
    if(columna < 0 || columna >= NUM_COLUMNAS_ORDEN) columna = Columna_puntos;

    return COMPARADORES[columna][descendente ? 1 : 0];
}

Puntaje puntajeVacio()
{
    Puntaje p;

    p.nombre[0]  = '\0';
    p.modo       = Modo_solitario;
    p.dificultad = Dificultad_facil;
    p.pares      = 0;
    p.puntos     = 0;
    p.racha      = 0;
    p.intentos   = 0;
    p.tiempo     = 0.0f;
    p.gano       = false;

    return p;
}
