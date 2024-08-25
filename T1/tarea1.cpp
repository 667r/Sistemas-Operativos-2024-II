#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <algorithm> // Para usar std::remove_if
namespace fs = std::filesystem;

using namespace std;

void creardirectorio(const string& path) {
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
}

string limpiarEspacios(string str) {
    // Eliminar espacios en blanco al inicio y al final
    str.erase(str.begin(), find_if(str.begin(), str.end(), [](unsigned char ch) { return !isspace(ch); }));
    str.erase(find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), str.end());
    return str;
}

int main() {
    string path_carpeta = "archivos_deportes";
    string path_destino = "archivos_deportes_organizados";

    for (const auto& entrada : fs::directory_iterator(path_carpeta)) {
        ifstream archivo_atleta(entrada.path());
        string deporte, categoria, medalla;
        getline(archivo_atleta, deporte);
        getline(archivo_atleta, categoria);
        getline(archivo_atleta, medalla);
        archivo_atleta.close();

        // Limpiar espacios en blanco
        deporte = limpiarEspacios(deporte);
        categoria = limpiarEspacios(categoria);
        medalla = limpiarEspacios(medalla);

        // Normalizar el nombre de la carpeta de medallas
        if (medalla == "Sin Medalla") {
            medalla = "SinMedalla";
        } else {
            medalla = "ConMedalla";
        }

        // Construir el path destino
        string aux_dir = path_destino + "/" + deporte;
        creardirectorio(aux_dir);
        aux_dir += "/" + categoria;
        creardirectorio(aux_dir);
        aux_dir += "/" + medalla;
        creardirectorio(aux_dir);

        // Mover el archivo al directorio destino
        string carp_destino = aux_dir + "/" + entrada.path().filename().string();
        try {
            fs::rename(entrada.path(), carp_destino);
        } catch (const fs::filesystem_error& e) {
            cerr << "Error al mover el archivo: " << e.what() << endl;
        }
    }

    return 0;
}
