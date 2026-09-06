# Gatorama

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![raylib](https://img.shields.io/badge/raylib-6.0-orange.svg)
![Windows](https://img.shields.io/badge/Windows-Supported-blue.svg)
![Code::Blocks](https://img.shields.io/badge/Code%3A%3ABlocks-25.03-green.svg)

Juego de memorama con temática de gatitos, desarrollado en **C++ con raylib**.

## Requisitos del sistema

| Componente | Versión | Para qué se necesita |
|---|---|---|
| Code::Blocks | **25.03** | IDE y compilador (trae MinGW-W64 14.2.0 UCRT) |
| Git | Cualquiera reciente | Clonar el repositorio |

## Instalación

### 1. Instalar Code::Blocks 25.03

Descarga desde [codeblocks.org](https://www.codeblocks.org/downloads/binaries/) el
instalador que **incluye MinGW**:

```
codeblocks-25.03mingw-setup.exe
```

> **Importante:** tiene que ser la versión **con MinGW** y la **25.03**. Una versión
> distinta trae otro compilador, y la biblioteca de raylib incluida en el repositorio
> podría no enlazar. Si eso pasa, ve a [Solución de problemas](#solución-de-problemas).

### 2. Clonar y abrir

```bash
git clone https://github.com/JASM18/Gatorama.git
cd Gatorama
```

Abre `Gatorama.cbp` con Code::Blocks y presiona **F9** (Build and run). Debería abrirse
una ventana del juego.

## Estructura del proyecto

```
gatorama/
├── libs/raylib/          Biblioteca raylib 6.0 (incluida a proposito)
│   ├── include/          raylib.h, raymath.h, rlgl.h
│   └── lib/              libraylib.a
├── recursos/             Ilustraciones de las cartas (gato01.png, gato02.png...)
├── main.cpp              Punto de entrada
├── Gatorama.cbp          Proyecto de Code::Blocks (SI se sube al repo)
└── README.md
```

### ¿Por qué raylib está dentro del repositorio?

Para que clonar y compilar sea todo lo que haya que hacer. La alternativa —que cada
integrante descargue e instale raylib por su cuenta— multiplica los puntos de falla:
versiones distintas, arquitecturas equivocadas (32 vs 64 bits) e incompatibilidades de
runtime. Son 2.1 MB en el repositorio a cambio de que nadie pierda una tarde
configurando el entorno.

El `.cbp` usa **rutas relativas** (`libs/raylib/...`), así que funciona sin importar en
qué carpeta se clone el proyecto.

## Solución de problemas

### `undefined reference to 'stat64i32'`

Tu compilador usa un runtime de C distinto (**UCRT** vs **MSVCRT**) al que se usó para
construir la `libraylib.a` del repositorio. Pasa si instalaste una versión de
Code::Blocks distinta a la 25.03.

Dos salidas:

**A) Instalar Code::Blocks 25.03** (recomendada, y la más rápida).

**B) Reconstruir raylib con tu propio compilador.** Descarga el
[código fuente de raylib 6.0](https://github.com/raysan5/raylib/archive/refs/tags/6.0.zip),
descomprímelo y en una terminal:

```bash
cd raylib-6.0/src
mingw32-make PLATFORM=PLATFORM_DESKTOP
```

Copia el `libraylib.a` resultante sobre `libs/raylib/lib/libraylib.a`. Tarda unos
minutos y no requiere instalar nada más.

### `skipping incompatible ... when searching for -lraylib`

La biblioteca es de arquitectura equivocada (32 bits contra un compilador de 64, o al
revés). Verifica con:

```bash
objdump -f libs/raylib/lib/libraylib.a | grep -m1 architecture   # pe-x86-64 = 64 bits
g++ -dumpmachine                                                  # x86_64-... = 64 bits
```

Ambos deben ser de 64 bits.

### El ejecutable compila pero no abre ventana

Revisa que el orden de las bibliotecas en el enlazador sea
`raylib → opengl32 → gdi32 → winmm`. GNU `ld` resuelve símbolos de izquierda a derecha:
quien *usa* los símbolos debe ir antes que quien los *provee*.

## Créditos

raylib es obra de Ramon Santamaria ([raysan5](https://github.com/raysan5)) y se
distribuye bajo licencia zlib/libpng. Ver `libs/raylib/LICENSE`.
