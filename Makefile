# =============================================================================
#  Gatorama - compilacion en macOS y Linux
# =============================================================================
#
#  En Windows NO se usa este archivo: ahi se abre Gatorama.cbp con Code::Blocks
#  y se presiona F9. Esto existe porque la raylib que viene en libs/ es un
#  binario de Windows (formato pe-x86-64) que no sirve en Mac, y porque
#  Code::Blocks en macOS esta practicamente abandonado.
#
#  ---- Preparacion, una sola vez en macOS ----
#
#      xcode-select --install            compilador de Apple
#      brew install raylib pkg-config    raylib para Mac
#
#  raylib tiene que ser 5.5 o mas nueva: el juego usa IsTextureValid y
#  DrawRectangleRoundedLinesEx, que no existen antes. Se revisa con:
#
#      pkg-config --modversion raylib
#
#  ---- Uso, parado en la carpeta del proyecto ----
#
#      make            compila
#      make ejecutar   compila y abre el juego
#      make limpiar    borra lo compilado
#
# =============================================================================

NOMBRE = Gatorama

# wildcard agarra todos los .cpp de la carpeta. Se hace asi para que agregar un
# archivo al proyecto no obligue a acordarse de editar este Makefile: en Windows
# se agrega al .cbp y del lado de Mac aparece solo.
FUENTES = $(wildcard *.cpp)

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -fexceptions -DPLATFORM_DESKTOP

# pkg-config responde donde quedaron los encabezados y la biblioteca de raylib.
# Se le pregunta a el en vez de escribir la ruta a mano porque Homebrew instala
# en lugares distintos segun el Mac: /usr/local en los Intel y /opt/homebrew en
# los Apple Silicon. Escribir la ruta fija romperia en la mitad de las maquinas.
CXXFLAGS += $(shell pkg-config --cflags raylib)
ENLACE    = $(shell pkg-config --libs raylib)

SISTEMA := $(shell uname -s)

ifeq ($(SISTEMA),Darwin)
    # macOS: raylib se apoya en estos servicios del sistema para la ventana, el
    # dibujo y la entrada. Van DESPUES de -lraylib porque el enlazador resuelve
    # de izquierda a derecha: quien usa los simbolos va antes que quien los
    # provee. Es la misma regla que el orden raylib-opengl32-gdi32-winmm de Windows.
    ENLACE += -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL
else
    # Linux
    ENLACE += -lGL -lm -lpthread -ldl -lrt -lX11
endif

# Se recompila todo de un jalon en vez de archivo por archivo. Con seis fuentes
# tarda un par de segundos y evita tener que explicar objetos intermedios.
$(NOMBRE): $(FUENTES)
	$(CXX) $(CXXFLAGS) $(FUENTES) $(ENLACE) -o $(NOMBRE)

# Se ejecuta desde la carpeta del proyecto, no desde donde quedo el binario:
# el juego busca sus imagenes en recursos/ relativo a donde se le llama.
ejecutar: $(NOMBRE)
	./$(NOMBRE)

limpiar:
	rm -f $(NOMBRE)

# Estos dos nombres son acciones, no archivos. Sin esta linea, el dia que exista
# un archivo llamado 'limpiar' make creeria que ya esta hecho y no haria nada.
.PHONY: ejecutar limpiar
