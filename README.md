# Gatorama

![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![raylib](https://img.shields.io/badge/raylib-6.0-orange.svg)
![Windows](https://img.shields.io/badge/Windows-probado-brightgreen.svg)
![macOS](https://img.shields.io/badge/macOS-por%20confirmar-yellow.svg)
![Linux](https://img.shields.io/badge/Linux-sin%20probar-yellow.svg)
![Licencia](https://img.shields.io/badge/raylib-zlib%2Flibpng-lightgrey.svg)

Este proyecto es un videojuego que consiste en el típico juego del “Memorama”, pero con temática de gatos, desarrollado en **C++ con raylib** para el
**Rally de la Niñez Científica 2026**.

A esa base le agrega un **sistema de puntajes clasificatorio** y dos modos de juego:
**solitario** y **multijugador (1 vs 1)**.

---

## Índice

**El juego**

- [En qué consiste](#en-qué-consiste)
- [Cómo se juega](#cómo-se-juega)
- [La racha y el puntaje](#la-racha-y-el-puntaje)
- [Controles](#controles)
- [Diseño](#diseño)

**Lo técnico**

- [Cómo ejecutar el juego](#cómo-ejecutar-el-juego)
- [Modo desarrollador](#modo-desarrollador)
- [Cómo está armado el proyecto](#cómo-está-armado-el-proyecto)
- [Empaquetar para el evento](#empaquetar-para-el-evento)
- [Solución de problemas](#solución-de-problemas)
- [Conclusiones](#conclusiones)
- [Créditos](#créditos)

---

# El juego

## En qué consiste

Un mazo de cartas boca abajo, cada dibujo repetido dos veces. Se voltean de dos en dos: si
coinciden se quedan descubiertas, si no, se vuelven a ocultar. El objetivo es descubrir
todas las parejas del tablero.

Sobre esa base hay tres cosas que hacen a Gatorama distinto de un memorama de siempre:

- **Un sistema de racha** que premia encadenar aciertos, sin decidir quién gana.
- **Un reloj independiente por jugador** en 1 vs 1: solo corre el de quien está jugando.
- **Una tabla de mejores puntajes** que guarda las partidas en disco, separadas por modo y
  dificultad, con un apartado para las resueltas en menos tiempo.

El programa usa una interfaz gráfica hecha con raylib para renderizar el juego: el jugador
interactúa con los botones del menú y con las cartas, y ve en pantalla su puntaje y su
reloj. Al terminar, el sistema determina el ganador y clasifica la partida en una tabla
que se consulta desde su propia opción del menú.

---

## Cómo se juega

### Antes de empezar

En la pantalla de configuración se eligen tres cosas:

1. **El modo** — solitario o 1 vs 1.
2. **La dificultad** — fácil, normal o difícil.
3. **Los nombres** de quienes van a jugar. Si se dejan en blanco, quedan como
   *Player 1* y *Player 2*.

La configuración se reinicia sola al terminar cada partida, para que el siguiente niño no
herede el nombre ni la dificultad del anterior.

### Las tres dificultades

| Nivel | Acomodo | Cartas | Parejas |
|---|---|---|---|
| Fácil | 2 × 5 | 10 | 5 |
| Normal | 2 × 9 | 18 | 9 |
| Difícil | 3 × 10 | 30 | 15 |

Las cartas se reparten con un barajado **Fisher-Yates**, y además hay un **segundo
sorteo** que decide *cuáles* gatos entran a la partida. Sin ese segundo sorteo, en fácil
siempre se jugaría con los mismos cinco gatos aunque cambiaran de lugar.

### Modo solitario

El jugador voltea dos cartas. Si forman pareja, se quedan descubiertas; si no, se vuelven
a ocultar y lo intenta otra vez. Sigue así hasta descubrir todas las parejas del tablero.

El reloj corre desde que empieza la partida y **se detiene en pausa**. El objetivo es
terminar el mazo; el objetivo secundario es hacerlo en el menor tiempo posible, porque eso
es lo que se registra en el apartado de mejores tiempos.

### Modo 1 vs 1

Por turnos, y el turno **no cambia con cada tirada**: cambia cuando alguien falla.

- Si el jugador en turno voltea dos cartas que **coinciden**, se anota la pareja y
  **sigue jugando**.
- Si **no coinciden**, las cartas se vuelven a ocultar, se le corta la racha y
  **pasa el turno** al otro.

Así hasta que se acaben las cartas del tablero.

**Gana quien haya juntado más parejas**, no quien tenga más puntos. Como las tres
dificultades tienen un número **impar** de parejas —5, 9 y 15— el empate es imposible.

Cada jugador tiene **su propio cronómetro**, y solo avanza mientras es su turno. Quedarse
pensando le cuesta tiempo a quien está pensando, no a los dos.

### Cuándo termina

La partida termina cuando **todas las cartas del tablero están emparejadas**. Ahí suena la
victoria, se guarda el resultado en la tabla de puntajes y aparece la pantalla que dice
quién ganó.

---

## La racha y el puntaje

La **racha** cuenta cuántas parejas seguidas se aciertan sin fallar. El puntaje de cada
pareja **es el largo de la racha en ese momento**:

| Pareja dentro de la racha | Puntos que da |
|---|---|
| la primera | 1 |
| la segunda seguida | 2 |
| la tercera seguida | 3 |
| … | … |

De ahí sale toda la gracia del sistema:

$$
\underbrace{1 + 2 + 3}_{\text{tres seguidas}} = 6
\qquad\text{contra}\qquad
\underbrace{1 + 1 + 1}_{\text{tres sueltas}} = 3
$$

Las mismas tres parejas valen el doble si se encadenan. Al fallar, la racha vuelve a cero
y la siguiente pareja arranca otra vez en 1.

> **El puntaje no decide quién gana.** Es una medalla para la tabla de mejores puntajes.
> La partida la gana quien junte más parejas, y punto. Eso es a propósito: así un niño que
> ganó limpio no pierde en la pantalla final por una cuenta que no entiende.

---

## Controles

El **ratón es la entrada principal**: es un juego de cartas contra reloj, y apuntar y
hacer clic es más rápido que recorrer el tablero con flechas. El teclado hace exactamente
lo mismo, no cosas distintas.

| Acción | Ratón | Teclado |
|---|---|---|
| Moverse por el menú | pasar el puntero | ↑ ↓ |
| Elegir | clic | Enter |
| Voltear una carta | clic en la carta | flechas para moverse, Enter o Espacio |
| Pausar | botón *Pausa* | Esc |
| Ver las instrucciones | botón **?** | — |
| Regresar | botón *Volver* | Esc |
| Ordenar la tabla de puntajes | clic en el encabezado de la columna | — |

En el tablero, un **anillo morado** marca en qué carta está parado el teclado. Si el
anillo sale **gris**, esa carta ya está resuelta y darle Enter no hace nada.

---

## Diseño

> **[IMAGEN FALTANTE]** — hoja de representación del diseño.

---

# Lo técnico

## Cómo ejecutar el juego

Esta sección es para **jugar**, no para programar. Si vas a tocar el código, salta a
[Modo desarrollador](#modo-desarrollador).

### En Windows

Es el sistema con el que se presenta el juego en el rally.

1. Consigue la carpeta del juego. Adentro hay exactamente dos cosas:

```
Gatorama/
├── Gatorama.exe
└── recursos/
```

2. Doble clic en `Gatorama.exe`.

No hay que instalar nada: ni Code::Blocks, ni raylib, ni bibliotecas de Microsoft. El
ejecutable se vale por sí mismo.

> **La carpeta `recursos/` tiene que quedar AL LADO del `.exe`.** El juego busca sus
> imágenes en `recursos/…` relativo a donde se le llama; si mueves el `.exe` solo a otra
> parte, abre pero sin gatos, sin fondo y sin sonido. Si vas a hacer un acceso directo,
> haz el acceso directo — no muevas el archivo.

### En macOS y Linux

Todavía **no hay un ejecutable listo para repartir** en estos sistemas: hay que
compilarlo. Son cuatro líneas en la terminal, y están en
[Modo desarrollador](#modo-desarrollador).

La razón es que un binario de Mac tendría que ir firmado para que macOS lo deje abrir sin
pelear, y como el rally se presenta en Windows, eso no se ha hecho.

---

## Modo desarrollador

### Qué tan probado está cada sistema

Vale la pena decirlo antes de que alguien pierda una tarde:

| Sistema | Estado |
|---|---|
| **Windows** | Probado de punta a punta: compila, corre, el depurador se detiene en los puntos de interrupción y el ejecutable de Release funciona en una máquina sin Code::Blocks. |
| **macOS** | Compiló bien en la máquina de un integrante. El Makefile cambió después de esa prueba, así que falta correr `make` una vez para volver a confirmarlo. |
| **Linux** | Sin probar. La rama del Makefile existe y es la estándar, pero nadie la ha ejecutado. |

### Lo que hace falta, por sistema

| | Windows | macOS | Linux |
|---|---|---|---|
| Compilador | Code::Blocks **25.03** (trae MinGW-W64 14.2.0 UCRT) | Herramientas de línea de comandos de Xcode | `build-essential` |
| raylib | **incluida en el repositorio** | Homebrew (5.5 o más nueva) | del gestor de paquetes (5.5 o más nueva) |
| Git | cualquiera reciente | cualquiera reciente | cualquiera reciente |

raylib tiene que ser **5.5 o más nueva** en Mac y Linux: el juego usa `IsTextureValid` y
`DrawRectangleRoundedLinesEx`, que no existen antes. Se revisa con
`pkg-config --modversion raylib`.

En Windows no aplica, porque la biblioteca viene dentro del repositorio. Ver
[¿por qué raylib está adentro?](#por-qué-raylib-está-dentro-del-repositorio).

### Clonar

```bash
git clone https://github.com/JASM18/Gatorama.git
cd Gatorama
```

---

### Windows

Hay **tres caminos y son el mismo compilador**. Escoge uno; no hay que hacer los tres.

#### A. Code::Blocks — el camino corto

Abre `Gatorama.cbp` y presiona **F9**. Se abre la ventana del juego.

Es todo. El `.cbp` referencia raylib con rutas relativas, así que funciona sin importar
en qué carpeta hayas clonado.

> Tiene que ser el instalador **con MinGW** y la versión **25.03**. Otra versión trae otro
> compilador y la `libraylib.a` del repositorio podría no enlazar; si eso pasa, ve a
> [Solución de problemas](#undefined-reference-to-stat64i32).

#### B. Visual Studio Code — con botones

VS Code no trae compilador: sigue usando el MinGW de Code::Blocks, así que **primero
instala Code::Blocks 25.03** aunque no lo vayas a abrir.

Dos extensiones:

| Extensión | Para qué |
|---|---|
| `ms-vscode.cpptools` | autocompletado y depurador |
| `actboy168.tasks` | los botones de abajo a la izquierda |

```bash
code --install-extension ms-vscode.cpptools
code --install-extension actboy168.tasks
```

La carpeta `.vscode/` **no está en el repositorio** (ver `.gitignore`): trae rutas que
dependen de dónde instaló cada quien sus cosas. Son cuatro archivos:

| Archivo | Qué resuelve | ¿Compila algo? |
|---|---|---|
| `tasks.json` | los cuatro botones | sí, llama al Makefile |
| `launch.json` | F5 con puntos de interrupción, vía `gdb` | ejecuta |
| `c_cpp_properties.json` | que el editor encuentre `raylib.h` | **no**, solo autocompletado |
| `settings.json` | el PATH de la terminal integrada | no |

Ya con eso, abajo a la izquierda aparecen:

```
🔧 Compilar     ▶ Compilar y ejecutar     ⏵ Ejecutar     🗑 Limpiar
```

Y **F5** compila y arranca el depurador: pones un punto rojo a la izquierda del número de
línea y el juego se congela ahí, con todas las variables a la vista.

> **Por qué esos archivos traen la ruta de MinGW escrita a mano.** Code::Blocks instala
> su compilador pero **no lo publica en el PATH de Windows**: lo usa por dentro y no se lo
> dice a nadie más. Por eso cada archivo tiene que decirle a VS Code dónde vive el
> `mingw32-make`, el `gdb` y el `g++`. En Mac no pasa porque Homebrew sí publica lo suyo.
>
> Se podría arreglar agregando esa carpeta al PATH del sistema, pero entonces ese
> compilador taparía a cualquier otro que instales después, en todos tus proyectos. Cuatro
> líneas feas dentro de `.vscode/` salen más baratas.

#### C. Terminal

En Windows GNU Make **se llama `mingw32-make`**, y su carpeta no está en el PATH. Desde la
terminal integrada de VS Code ya funciona; desde una consola nueva hay que agregarla:

```powershell
$env:PATH = "C:\Program Files\CodeBlocks\MinGW\bin;$env:PATH"
```

```powershell
mingw32-make            # compila
mingw32-make ejecutar   # compila y abre el juego
mingw32-make limpiar    # borra el ejecutable
```

---

### macOS

Una sola vez:

```bash
xcode-select --install            # compilador de Apple
brew install raylib pkg-config    # raylib para Mac
```

Después, parado en la carpeta del proyecto:

```bash
make            # compila
make ejecutar   # compila y abre el juego
make limpiar    # borra lo compilado
```

`Gatorama.cbp` **no sirve en Mac**: la `libraylib.a` del repositorio es un binario de
Windows, y Code::Blocks en macOS está prácticamente abandonado. Por eso existe el
Makefile.

Para trabajar con VS Code, lo mismo que en Windows pero con `make` en vez de
`mingw32-make`, `lldb` en vez de `gdb` y `./Gatorama` en vez de `.\Gatorama.exe`. Para el
autocompletado, apunta a **`/opt/homebrew/opt/raylib/include`** y no a la ruta de
`Cellar`: la de `opt` es un enlace estable que sobrevive a las actualizaciones, mientras
que `Cellar/raylib/6.0/include` deja de existir el día que raylib pase a 6.1.

---

### Linux

> **Sin probar.** La rama de Linux del Makefile existe y es la estándar, pero ningún
> integrante del equipo la ha corrido todavía. Si la usas y algo falla, abre un issue —
> es información que al proyecto le falta.

```bash
sudo apt install build-essential pkg-config libraylib-dev   # Debian y Ubuntu
```

Si el paquete de tu distribución trae raylib más vieja que 5.5, hay que compilarla desde
el [código fuente](https://github.com/raysan5/raylib).

Después, los mismos tres comandos que en macOS.

---

## Cómo está armado el proyecto

### Estructura

```
Gatorama/
├── libs/raylib/          raylib 6.0 para Windows (incluida a proposito)
│   ├── include/          raylib.h, raymath.h, rlgl.h
│   └── lib/              libraylib.a
├── recursos/             imagenes, fuente y sonidos
├── Carta · Tablero · Partida · Dificultad         el modelo, sin raylib
├── Monticulo · Puntaje · TablaPuntajes            los puntajes
├── Menu · Juego · Configuracion · Puntajes …      las pantallas
├── main.cpp              punto de entrada y bucle principal
├── Gatorama.cbp          proyecto de Code::Blocks (SI se sube)
├── Makefile              compilacion por terminal, los tres sistemas
└── README.md
```

### Modelo y Vista, separados

La lógica del juego —cartas, parejas, turnos, puntaje— **no sabe nada de raylib**. Se
puede probar en una consola, sin abrir una ventana. El módulo que dibuja le pregunta al
modelo, nunca al revés.

No es adorno académico: es lo que deja que cuatro personas trabajen en paralelo sin
pisarse en Git, y lo que permitió pasar el juego de tema oscuro a tema claro tocando un
solo archivo.

### Estructuras de datos propias

Varias piezas reutilizan implementaciones que el equipo escribió desde cero en Estructura
de Datos, en vez de la biblioteca estándar:

| Estructura | Dónde se usa |
|---|---|
| Montículo binario | la tabla de mejores puntajes, para sacar los primeros N |
| Barajado Fisher-Yates | repartir las cartas, y sortear qué gatos entran |
| Matriz en un solo bloque | el tablero |
| Archivos estructurados | `puntajes.json`, con lector y escritor propios |

### Dos formas de compilar, un solo compilador

| | Windows | macOS y Linux |
|---|---|---|
| `Gatorama.cbp` | sí, F9 | no |
| `Makefile` | sí, `mingw32-make` | sí, `make` |

No son dos formas distintas de construir el juego: es el mismo `g++` con las mismas
banderas, llamado por dos caminos. El `.cbp` se sube al repositorio porque es la
configuración del proyecto; los `.layout` y `.depend` no, porque son estado personal de
cada quien.

### ¿Por qué raylib está dentro del repositorio?

Para que clonar y compilar sea todo lo que haya que hacer en Windows. La alternativa —que
cada integrante la descargue e instale por su cuenta— multiplica los puntos de falla:
versiones distintas, arquitecturas equivocadas, runtimes incompatibles. Son 2.1 MB en el
repositorio a cambio de que nadie pierda una tarde configurando el entorno.

En Mac y Linux sí se instala aparte, porque la incluida es un binario de Windows.

### El orden de las bibliotecas importa

```
raylib → opengl32 → gdi32 → winmm
```

GNU `ld` resuelve símbolos de izquierda a derecha: quien los **usa** va antes que quien
los **provee**. Invertirlo produce `undefined reference` con todo bien instalado. La misma
regla aplica a los `-framework` de macOS, que van después de `-lraylib`.

---

## Empaquetar para el evento

El día del rally el juego se copia a otra computadora, y esa computadora **no tiene
Code::Blocks**. Compilado en Debug, el `.exe` pide `libgcc_s_seh-1.dll` y
`libstdc++-6.dll`, que solo existen donde hay MinGW instalado; en una Windows recién
instalada no abre y lo único que sale es un cuadro de error.

El target **Release** del `.cbp` ya lleva las banderas que lo arreglan:

| Bandera | Qué hace |
|---|---|
| `-static` | mete esas bibliotecas dentro del `.exe` |
| `-s` | quita los símbolos de depuración |
| tipo *aplicación de ventana* | no abre una consola negra atrás del juego |

Los pasos:

1. En Code::Blocks, cambia el target de *Debug* a **Release** y compila.
2. Crea una carpeta nueva.
3. Copia adentro `bin/Release/Gatorama.exe` y la carpeta `recursos/` completa.
4. **Pruébala en una computadora que no tenga Code::Blocks.** Este paso no se salta: es el
   único que de verdad comprueba que no falta nada.

---

## Solución de problemas

### `undefined reference to 'stat64i32'`

Tu compilador usa un runtime de C distinto (**UCRT** contra **MSVCRT**) al que se usó para
construir la `libraylib.a` del repositorio. Pasa si instalaste una versión de Code::Blocks
distinta a la 25.03.

**A) Instalar Code::Blocks 25.03** — recomendada, y la más rápida.

**B) Reconstruir raylib con tu propio compilador.** Descarga el
[código fuente de raylib 6.0](https://github.com/raysan5/raylib/archive/refs/tags/6.0.zip)
y en una terminal:

```bash
cd raylib-6.0/src
mingw32-make PLATFORM=PLATFORM_DESKTOP
```

Copia el `libraylib.a` resultante sobre `libs/raylib/lib/libraylib.a`.

### `skipping incompatible … when searching for -lraylib`

La biblioteca es de la arquitectura equivocada. Ambos deben decir 64 bits:

```bash
objdump -f libs/raylib/lib/libraylib.a | grep -m1 architecture   # pe-x86-64
g++ -dumpmachine                                                 # x86_64-…
```

### `make` no se reconoce como… (Windows)

En Windows GNU Make se llama **`mingw32-make`**, y vive en
`C:\Program Files\CodeBlocks\MinGW\bin`, que no está en el PATH. Ver
[Terminal](#c-terminal).

### El juego abre pero sale sin gatos, sin fondo o sin sonido

La carpeta `recursos/` no está donde el juego la busca. Las rutas son **relativas a la
carpeta desde donde se ejecuta el programa**, no a donde está el `.exe`. Code::Blocks lo
corre desde la carpeta del proyecto, así que con F9 funciona; abriendo el `.exe` a mano
desde `bin/Debug`, no.

El juego no truena por esto a propósito: dibuja formas de colores en lugar de las imágenes
que falten, para que se pueda seguir trabajando con el arte a medias.

### Una imagen o un sonido no carga aunque el archivo esté ahí

raylib elige cómo leer un archivo **por su extensión, no por su contenido**. Ya pasó dos
veces en este proyecto: un `.jpg` renombrado a `.png`, y dos `.wav` llamados `.mp3`. El
segundo caso es el peligroso, porque uno de ellos *parecía* cargar y lo que sonaba era
ruido.

Además, la `libraylib.a` incluida viene **sin soporte de JPEG**: solo lee PNG y BMP.

Si algo no aparece o suena raro, revisa los primeros bytes del archivo, no su nombre.

### El ejecutable compila pero no abre ventana

Revisa el orden de las bibliotecas en el enlazador:
[el orden importa](#el-orden-de-las-bibliotecas-importa).

---

## Conclusiones

Dos decisiones cargaron con casi todo el peso del proyecto.

**Separar el modelo de la vista** permitió que las reglas del juego se probaran en una
consola, sin ventana, y que cuatro personas trabajaran en paralelo sin pisarse. Cuando el
evento pidió cambiar de tema oscuro a tema claro, el cambio fue en un solo archivo:
ninguna pantalla necesitó una línea.

**Definir cada control una sola vez** —una función que devuelve su rectángulo, usada tanto
para dibujarlo como para detectar el clic— hizo imposible el error clásico de la interfaz:
que algo se vea en un lugar y responda en otro. Mover un botón es cambiar cuatro números
en un solo lugar.

Lo que costó caro fue lo que no se puede deducir leyendo código: que una biblioteca
compilada con un runtime distinto no enlaza, que un archivo miente sobre su formato, y que
un `.exe` que corre en la máquina de quien programa puede no abrir en ninguna otra. Nada
de eso lo dice el compilador. Por eso está escrito aquí.

---

## Créditos

Videojuego desarrollado para el **Rally de la Niñez Científica 2026** por estudiantes de
la Licenciatura en Ciencias de la Computación de la **Universidad de Sonora**.

Desarrolladores:

- Jesús Axel Sánchez Montoy
- Alba Rosa Helleon Cárdenas
- Ivana Lin Chenoweth Galaz
- Ángel Daniel Durón Urbina

raylib es obra de Ramon Santamaria ([raysan5](https://github.com/raysan5)) y se distribuye
bajo licencia zlib/libpng. Ver `libs/raylib/LICENSE`.
