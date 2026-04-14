#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// =======================
// ESTRUCTURASSSS
// =======================

// Aquí guardo la config de la liga (todo viene del archivo)
struct ConfigLiga {
    string nombreLiga;
    int puntosVictoria;
    int puntosEmpate;
    int puntosDerrota;
    vector<string> equipos; // obviamente solo millos
};

// Partido normal
struct Partido {
    string fecha;
    string local;
    string visitante;
    int golesLocal;
    int golesVisitante;
};

// Stats de cada equipo
struct Equipo {
    string nombre;
    int PJ = 0, PG = 0, PE = 0, PP = 0;
    int GF = 0, GC = 0;
    int puntos = 0;
};

// =======================
// ARCHIVOSSSSS
// =======================

// Leo el config.txt (si falla, muere el programa, no hay liga sin config)
ConfigLiga leerConfig() {
    ConfigLiga config;
    ifstream file("data/config.txt");

    if (!file.is_open()) {
        cout << "ERROR: no se pudo abrir config.txt\n";
        exit(1);
    }

    string linea;

    while (getline(file, linea)) {

        // Ignoro basura o comentarios
        if (linea.empty() || linea[0] == '#') continue;

        size_t pos = linea.find('=');

        if (pos == string::npos) {
            cout << "Formato invalido en config.txt\n";
            exit(1);
        }

        string clave = linea.substr(0, pos);
        string valor = linea.substr(pos + 1);

        // Cargo los datos
        if (clave == "liga") config.nombreLiga = valor;
        else if (clave == "puntos_victoria") config.puntosVictoria = stoi(valor);
        else if (clave == "puntos_empate") config.puntosEmpate = stoi(valor);
        else if (clave == "puntos_derrota") config.puntosDerrota = stoi(valor);
        else if (clave == "equipo") config.equipos.push_back(valor);
    }

    file.close();
    return config;
}

// Guardo el partido en el archivo (modo append, o sea no borro nada)
void guardarPartido(const Partido &p) {
    ofstream file("data/partidos.txt", ios::app);

    if (!file.is_open()) {
        cout << "Error al abrir partidos.txt\n";
        return;
    }

    // Formato tipo CSV
    file << p.fecha << ","
         << p.local << ","
         << p.visitante << ","
         << p.golesLocal << ","
         << p.golesVisitante << "\n";

    file.close();
}

// Leo todos los partidos guardados
vector<Partido> leerPartidos() {
    vector<Partido> partidos;
    ifstream file("data/partidos.txt");

    if (!file.is_open()) return partidos;

    string linea;

    while (getline(file, linea)) {
        stringstream ss(linea);
        Partido p;
        string temp;

        // Separo todo por comas
        getline(ss, p.fecha, ',');
        getline(ss, p.local, ',');
        getline(ss, p.visitante, ',');
        getline(ss, temp, ','); p.golesLocal = stoi(temp);
        getline(ss, temp, ','); p.golesVisitante = stoi(temp);

        partidos.push_back(p);
    }

    return partidos;
}

// =======================
// LOGICAAAaaaAAAAaaAAAA 
// =======================

// Función clave (punteros) - aquí es donde pasa la magia
void actualizarEquipo(Equipo *eq, int gf, int gc, ConfigLiga config) {

    eq->PJ++;
    eq->GF += gf;
    eq->GC += gc;

    // Si gana → felicidad :) (sobre todo si es Millos)
    if (gf > gc) {
        eq->PG++;
        eq->puntos += config.puntosVictoria;


    }
    else if (gf == gc) {
        eq->PE++;
        eq->puntos += config.puntosEmpate;
    }
    else {
        eq->PP++;
        eq->puntos += config.puntosDerrota;

    }
}

// Construyo la tabla completa
vector<Equipo> construirTabla(vector<Partido> partidos, ConfigLiga config) {

    vector<Equipo> tabla;

    // Creo los equipos
    for (string nombre : config.equipos) {
        Equipo e;
        e.nombre = nombre;
        tabla.push_back(e);
    }

    // Recorro partidos
    for (Partido p : partidos) {

        Equipo *local = nullptr;
        Equipo *visitante = nullptr;

        for (auto &eq : tabla) {
            if (eq.nombre == p.local) local = &eq;
            if (eq.nombre == p.visitante) visitante = &eq;
        }

        if (local && visitante) {
            actualizarEquipo(local, p.golesLocal, p.golesVisitante, config);
            actualizarEquipo(visitante, p.golesVisitante, p.golesLocal, config);
        }
    }

    return tabla;
}

// Ordeno la tabla
void ordenarTabla(vector<Equipo> &tabla) {
    sort(tabla.begin(), tabla.end(), [](Equipo a, Equipo b) {

        if (a.puntos != b.puntos) return a.puntos > b.puntos;

        int dgA = a.GF - a.GC;
        int dgB = b.GF - b.GC;
        if (dgA != dgB) return dgA > dgB;

        return a.GF > b.GF;
    });
}

// Muestro la tabla
void mostrarTabla(vector<Equipo> tabla) {

    cout << "\n#\tEquipo\tPJ\tPG\tPE\tPP\tGF\tGC\tDG\tPTS\n";

    int pos = 1;

    for (auto e : tabla) {
        int dg = e.GF - e.GC;

        cout << pos++ << "\t"
             << e.nombre << "\t"
             << e.PJ << "\t"
             << e.PG << "\t"
             << e.PE << "\t"
             << e.PP << "\t"
             << e.GF << "\t"
             << e.GC << "\t"
             << dg << "\t"
             << e.puntos << "\n";
    }

    // comentario muy clave 
    cout << "\n(solo millos loka)\n";
}

// Guardar tabla
void guardarTabla(vector<Equipo> tabla) {
    ofstream file("data/tabla.txt");

    if (!file.is_open()) {
        cout << "Error al guardar tabla\n";
        return;
    }

    for (auto e : tabla) {
        file << e.nombre << " " << e.puntos << "\n";
    }

    file.close();
}

// =======================
// INTERFAaaAAAaaZ
// =======================

int menu(string liga) {
    int op;

    cout << "\n===== " << liga << " =====\n";
    cout << "1. Ver tabla\n";
    cout << "2. Registrar partido\n";
    cout << "3. Ver partidos\n";
    cout << "4. Guardar tabla\n";
    cout << "5. Salir\n";
    cout << "Opcion: ";
    cin >> op;

    return op;
}

// Verifico si el equipo existe
bool existeEquipo(string nombre, vector<string> equipos) {
    for (string e : equipos) {
        if (e == nombre) return true;
    }
    return false;
}

// Registro partido
void registrarPartido(ConfigLiga config) {
    Partido p;

    cout << "Fecha: ";
    cin >> p.fecha;

    cout << "Equipo local: ";
    cin >> p.local;

    cout << "Equipo visitante: ";
    cin >> p.visitante;

    if (p.local == p.visitante) {
        cout << "Error: no pueden ser iguales\n";
        return;
    }

    if (!existeEquipo(p.local, config.equipos) ||
        !existeEquipo(p.visitante, config.equipos)) {
        cout << "Error: equipo no existe\n";
        return;
    }

    cout << "Goles local: ";
    cin >> p.golesLocal;

    cout << "Goles visitante: ";
    cin >> p.golesVisitante;

    guardarPartido(p);
}

// Mostrar partidos
void mostrarPartidos(vector<Partido> partidos) {
    for (auto p : partidos) {
        cout << p.fecha << " | "
             << p.local << " "
             << p.golesLocal << " - "
             << p.golesVisitante << " "
             << p.visitante << "\n";
    }
}

// =======================
// MAINNNN
// =======================

int main() {

    // arranca todo desde config
    ConfigLiga config = leerConfig();

    int op;

    do {
        op = menu(config.nombreLiga);

        if (op == 1) {
            auto partidos = leerPartidos();
            auto tabla = construirTabla(partidos, config);
            ordenarTabla(tabla);
            mostrarTabla(tabla);
        }

        else if (op == 2) {
            registrarPartido(config);
        }

        else if (op == 3) {
            auto partidos = leerPartidos();
            mostrarPartidos(partidos);
        }

        else if (op == 4) {
            auto partidos = leerPartidos();
            auto tabla = construirTabla(partidos, config);
            ordenarTabla(tabla);
            guardarTabla(tabla);
        }

    } while (op != 5);

    return 0;
}
