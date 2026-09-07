/**
 * \file TablaPuntajes.cpp
 * \brief Implementaci&oacute;n de la tabla de puntajes y su archivo JSON.
 * \author S&aacute;nchez Montoy, Jes&uacute;s Axel
 * \date 06/09/2026
 */

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "TablaPuntajes.hpp"
#include "Dificultad.hpp"

const char* const RUTA_PUNTAJES = "puntajes.json";

//***********************************************
// TRADUCIR ENUMS A TEXTO Y DE REGRESO
//***********************************************

// El archivo guarda "Facil" y "solitario" en vez de 0 y 1. Cuesta unas lineas mas
// de codigo y a cambio el JSON se puede abrir con el bloc de notas y entender sin
// consultar el codigo fuente. Para un archivo que el equipo va a revisar a mano
// durante el rally, vale la pena.

static const char* textoDeModo(ModoJuego modo)
{
    return (modo == Modo_multijugador) ? "multijugador" : "solitario";
}

static ModoJuego modoDeTexto(const std::string& texto)
{
    return (texto == "multijugador") ? Modo_multijugador : Modo_solitario;
}

static const char* textoDeDificultad(Dificultad nivel)
{
    if(nivel < 0 || nivel >= NUM_DIFICULTADES) nivel = Dificultad_facil;

    return DIFICULTADES[nivel].nombre;
}

static Dificultad dificultadDeTexto(const std::string& texto)
{
    for(int i = 0; i < NUM_DIFICULTADES; i++){
        if(texto == DIFICULTADES[i].nombre) return (Dificultad)i;
    }

    // Un nivel desconocido -alguien edito el archivo, o se quito una dificultad-
    // no debe tirar el juego. Se cae de pie al mas chico.
    return Dificultad_facil;
}

//***********************************************
// ESCRITURA DEL JSON
//***********************************************

/**
 * \brief Escapa una cadena para meterla entre comillas en el JSON.
 *
 * El jugador puede teclear comillas y diagonales invertidas en su nombre -el campo
 * acepta todo el ASCII imprimible-, y sin escaparlas el archivo quedar&iacute;a roto y no
 * se podr&iacute;a volver a leer.
 *
 * \param texto Cadena original.
 * \return La cadena lista para el archivo.
 */
static std::string escapar(const char* texto)
{
    std::string salida;

    for(int i = 0; texto[i] != '\0'; i++){
        char c = texto[i];

        if(c == '"' || c == '\\') salida += '\\';

        salida += c;
    }

    return salida;
}

bool TablaPuntajes::Guardar(const char* ruta) const
{
    std::ofstream archivo(ruta);

    if(!archivo) return false;

    archivo << "{\n";
    archivo << "  \"puntajes\": [\n";

    for(int i = 0; i < cantidad; i++){
        const Puntaje& p = lista[i];

        archivo << "    {\n";
        archivo << "      \"nombre\": \""     << escapar(p.nombre)            << "\",\n";
        archivo << "      \"modo\": \""       << textoDeModo(p.modo)          << "\",\n";
        archivo << "      \"dificultad\": \"" << textoDeDificultad(p.dificultad) << "\",\n";
        archivo << "      \"pares\": "        << p.pares                      << ",\n";
        archivo << "      \"puntos\": "       << p.puntos                     << ",\n";
        archivo << "      \"racha\": "        << p.racha                      << ",\n";
        archivo << "      \"intentos\": "     << p.intentos                   << ",\n";
        archivo << "      \"tiempo\": "       << p.tiempo                     << ",\n";
        archivo << "      \"gano\": "         << (p.gano ? "true" : "false")  << "\n";

        // La ultima entrada no lleva coma. Un JSON con coma de mas es invalido, y
        // es el error mas comun al escribirlo a mano.
        archivo << "    }" << ((i < cantidad - 1) ? "," : "") << "\n";
    }

    archivo << "  ]\n";
    archivo << "}\n";

    return archivo.good();
}

//***********************************************
// LECTURA DEL JSON
//***********************************************

/**
 * \brief Lee una cadena entre comillas y deja la posici&oacute;n despu&eacute;s de ella.
 *
 * \param s      Texto completo.
 * \param pos    Entra apuntando a la comilla de apertura; sale despu&eacute;s de la de cierre.
 * \param salida D&oacute;nde dejar el contenido, ya sin escapes.
 * \return Verdadero si la cadena estaba bien formada.
 */
static bool leerCadena(const std::string& s, size_t& pos, std::string& salida)
{
    salida.clear();

    if(pos >= s.size() || s[pos] != '"') return false;

    pos++;   // se salta la comilla de apertura

    while(pos < s.size()){

        char c = s[pos];

        if(c == '\\'){
            // Una diagonal invertida significa "el siguiente caracter es literal".
            pos++;
            if(pos >= s.size()) return false;

            salida += s[pos];
            pos++;
            continue;
        }

        if(c == '"'){
            pos++;   // se salta la comilla de cierre
            return true;
        }

        salida += c;
        pos++;
    }

    return false;   // se acabo el texto sin cerrar la cadena
}

/**
 * \brief Lee el valor que sigue a los dos puntos y lo mete en el registro.
 *
 * \param s     Texto completo.
 * \param pos   Entra en el primer caracter del valor; sale despu&eacute;s de &eacute;l.
 * \param clave Nombre del campo.
 * \param p     Registro a llenar.
 * \return Verdadero si se pudo leer el valor.
 */
static bool leerValor(const std::string& s, size_t& pos, const std::string& clave, Puntaje& p)
{
    if(pos >= s.size()) return false;

    if(s[pos] == '"'){
        std::string texto;
        if(!leerCadena(s, pos, texto)) return false;

        if(clave == "nombre"){
            strncpy(p.nombre, texto.c_str(), LARGO_NOMBRE);
            p.nombre[LARGO_NOMBRE] = '\0';
        }
        else if(clave == "modo")       p.modo       = modoDeTexto(texto);
        else if(clave == "dificultad") p.dificultad = dificultadDeTexto(texto);

        return true;
    }

    // Numeros y booleanos: se toma todo hasta la coma o el cierre del objeto.
    size_t inicio = pos;

    while(pos < s.size() && s[pos] != ',' && s[pos] != '}' && s[pos] != '\n') pos++;

    std::string texto = s.substr(inicio, pos - inicio);

    if(clave == "gano"){
        p.gano = (texto.find("true") != std::string::npos);
        return true;
    }

    double numero = atof(texto.c_str());

    if(clave == "pares")         p.pares    = (int)numero;
    else if(clave == "puntos")   p.puntos   = (int)numero;
    else if(clave == "racha")    p.racha    = (int)numero;
    else if(clave == "intentos") p.intentos = (int)numero;
    else if(clave == "tiempo")   p.tiempo   = (float)numero;

    return true;
}

int TablaPuntajes::Cargar(const char* ruta)
{
    cantidad = 0;

    std::ifstream archivo(ruta);

    // Que no exista no es un error: es la primera vez que se abre el juego.
    if(!archivo) return 0;

    std::stringstream buffer;
    buffer << archivo.rdbuf();

    std::string s = buffer.str();
    size_t      i = 0;

    // Se recorre el texto de llave en llave. Cada '{' que no este dentro de una
    // cadena empieza un registro; se leen sus campos hasta el '}' que lo cierra.
    while(i < s.size()){

        if(s[i] == '"'){
            // Una cadena suelta -como "puntajes"- se salta entera, para que un
            // nombre de jugador que traiga llaves no confunda al lector.
            std::string basura;
            if(!leerCadena(s, i, basura)) break;
            continue;
        }

        if(s[i] != '{'){ i++; continue; }

        // ---- Empieza un registro ----
        i++;

        Puntaje p = puntajeVacio();
        bool    completo = false;

        while(i < s.size()){

            if(s[i] == '}'){ i++; completo = true; break; }

            if(s[i] != '"'){ i++; continue; }

            std::string clave;
            if(!leerCadena(s, i, clave)) break;

            // Se salta lo que haya entre la clave y su valor.
            while(i < s.size() && (s[i] == ' ' || s[i] == ':' || s[i] == '\t')) i++;

            if(!leerValor(s, i, clave, p)) break;
        }

        // Un registro a medias se tira, pero el archivo se sigue leyendo: un
        // renglon corrupto no debe costar toda la tabla.
        if(completo && cantidad < MAX_PUNTAJES){
            lista[cantidad] = p;
            cantidad++;
        }
    }

    return cantidad;
}

//***********************************************
// LA TABLA
//***********************************************

TablaPuntajes::TablaPuntajes()
{
    cantidad = 0;
}

void TablaPuntajes::Agregar(const Puntaje& p)
{
    if(cantidad >= MAX_PUNTAJES){
        // Llena: se recorre todo un lugar y se tira la mas vieja. Es caro -mueve
        // todo el arreglo- pero pasa una vez cada 500 partidas y deja el
        // comportamiento claro: se conservan las mas recientes.
        for(int i = 1; i < MAX_PUNTAJES; i++){
            lista[i - 1] = lista[i];
        }

        cantidad = MAX_PUNTAJES - 1;
    }

    lista[cantidad] = p;
    cantidad++;
}

int TablaPuntajes::Cantidad() const
{
    return cantidad;
}

const Puntaje& TablaPuntajes::En(int indice) const
{
    if(indice < 0 || indice >= cantidad) return lista[0];

    return lista[indice];
}

int TablaPuntajes::Mejores(Puntaje* destino, int cuantos,
                           Monticulo::Comparador esMejor, bool soloSolitario) const
{
    if(cuantos <= 0 || cantidad == 0) return 0;

    Monticulo monticulo(cantidad, esMejor);

    for(int i = 0; i < cantidad; i++){
        if(soloSolitario && lista[i].modo != Modo_solitario) continue;

        monticulo.Insertar(lista[i]);
    }

    int llenados = 0;

    while(llenados < cuantos && !monticulo.EstaVacio()){
        destino[llenados] = monticulo.Sacar();
        llenados++;
    }

    return llenados;
}
