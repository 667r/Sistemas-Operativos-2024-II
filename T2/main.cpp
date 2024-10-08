#include <iostream>
#include <vector>
#include <stack>
#include <unistd.h>  // Para fork()
#include <sys/ipc.h> // Para memoria compartida
#include <sys/shm.h>
#include <ctime>     // Para generar aleatoriedad
#include <cstdlib>   // Para srand y rand
#include <wait.h>    // Para wait()
#include <semaphore.h> // Para semáforos

using namespace std;

struct Carta {
    int numero;
    string color;
    bool esEspecial;
    string tipoEspecial; // "cambio", "salta", "+2", "comodin", "+4"
};

struct Jugador {
    vector<Carta> mano;
};

void mostrarCartas(const vector<Carta>& mano) {
    cout << "Tus cartas:" << endl;
    for (size_t i = 0; i < mano.size(); i++) {
        cout << i + 1 << ": " << mano[i].color << " " << (mano[i].esEspecial ? mano[i].tipoEspecial : to_string(mano[i].numero)) << endl;
    }
}

void inicializarMazo(vector<Carta>& mazo) {
    string colores[] = {"rojo", "amarillo", "verde", "azul"};
    for (const auto& color : colores) {
        for (int num = 0; num <= 9; ++num) {
            Carta carta = {num, color, false, ""};
            mazo.push_back(carta);
            if (num != 0) mazo.push_back(carta);  // El 0 no se repite
        }
    }

    for (const auto& color : colores) {
        for (int i = 0; i < 2; i++) {
            Carta cambioSentido = {-1, color, true, "cambio"};
            Carta salta = {-1, color, true, "salta"};
            Carta robaDos = {-1, color, true, "+2"};
            mazo.push_back(cambioSentido);
            mazo.push_back(salta);
            mazo.push_back(robaDos);
        }
    }

    for (int i = 0; i < 4; i++) {
        Carta comodin = {-1, "", true, "comodin"};
        Carta comodinRobaCuatro = {-1, "", true, "+4"};
        mazo.push_back(comodin);
        mazo.push_back(comodinRobaCuatro);
    }
}

void barajarMazo(vector<Carta>& mazo) {
    srand(time(0));
    for (size_t i = 0; i < mazo.size(); ++i) {
        int j = rand() % mazo.size();
        swap(mazo[i], mazo[j]);
    }
}

void repartirCartas(vector<Carta>& mazo, Jugador jugadores[], int numJugadores) {
    for (int i = 0; i < numJugadores; i++) {
        for (int j = 0; j < 7; j++) {
            jugadores[i].mano.push_back(mazo.back());
            mazo.pop_back();
        }
    }
}

void jugarTurnoHumano(Jugador& jugador, stack<Carta>& pilaDescarte, vector<Carta>& mazo, sem_t* semaforo, bool* esTurno) {
    Carta cartaActual = pilaDescarte.top();
    cout << "Carta en la pila de descarte: " << cartaActual.color << " " 
         << (cartaActual.esEspecial ? cartaActual.tipoEspecial : to_string(cartaActual.numero)) << endl;

    mostrarCartas(jugador.mano);

    bool jugada = false;
    while (!jugada) {
        cout << "Selecciona una carta para jugar (0 para robar): ";
        int opcion;
        cin >> opcion;

        if (opcion == 0) {
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                cout << "Has robado: " << cartaRobada.color << " " 
                     << (cartaRobada.esEspecial ? cartaRobada.tipoEspecial : to_string(cartaRobada.numero)) << endl;
                
                if (cartaRobada.color == cartaActual.color || cartaRobada.numero == cartaActual.numero) {
                    pilaDescarte.push(cartaRobada);
                    cout << "Has jugado la carta robada." << endl;
                    jugada = true;
                } else {
                    jugador.mano.push_back(cartaRobada);
                }
            } else {
                cout << "El mazo está vacío." << endl;
            }
        } else if (opcion > 0 && opcion <= jugador.mano.size()) {
            Carta cartaElegida = jugador.mano[opcion - 1];
            if (cartaElegida.color == cartaActual.color || cartaElegida.numero == cartaActual.numero || cartaElegida.esEspecial) {
                pilaDescarte.push(cartaElegida);
                jugador.mano.erase(jugador.mano.begin() + opcion - 1);
                cout << "Has jugado: " << cartaElegida.color << " " 
                     << (cartaElegida.esEspecial ? cartaElegida.tipoEspecial : to_string(cartaElegida.numero)) << endl;
                jugada = true;
            } else {
                cout << "No puedes jugar esa carta." << endl;
            }
        } else {
            cout << "Opción inválida." << endl;
        }
    }

    // Después de jugar, liberar el semáforo para que los bots puedan jugar
    *esTurno = false;
    sem_post(semaforo); // Permitir que los bots jueguen
}

void jugarTurnoBot(int jugadorId, Jugador jugadores[], stack<Carta>& pilaDescarte, vector<Carta>& mazo, sem_t* semaforo, bool* esTurno) {
    // Esperar a que sea el turno del bot
    sem_wait(semaforo); // Esperar a que el jugador humano termine

    Carta cartaActual = pilaDescarte.top();
    bool jugada = false;

    for (size_t i = 0; i < jugadores[jugadorId].mano.size(); i++) {
        if (jugadores[jugadorId].mano[i].color == cartaActual.color || jugadores[jugadorId].mano[i].numero == cartaActual.numero || jugadores[jugadorId].mano[i].esEspecial) {
            pilaDescarte.push(jugadores[jugadorId].mano[i]);
            jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i);
            jugada = true;
            cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado una carta." << endl;
            break;
        }
    }

    if (!jugada) {
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            if (cartaRobada.color == cartaActual.color || cartaRobada.numero == cartaActual.numero) {
                pilaDescarte.push(cartaRobada);
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado la carta robada." << endl;
            } else {
                jugadores[jugadorId].mano.push_back(cartaRobada);
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha robado una carta." << endl;
            }
        } else {
            cout << "El mazo está vacío. Jugador " << jugadorId + 1 << " no puede robar." << endl;
        }
    }

    // Indicar que el bot ha terminado su turno
    *esTurno = true; // Permitir que el jugador humano juegue de nuevo
    sem_post(semaforo); // Liberar el semáforo para que el jugador humano pueda jugar
}

int main() {
    // Crear memoria compartida
    key_t key = ftok("shmfile", 65); 
    int shmId = shmget(key, 1024, 0666 | IPC_CREAT); 
    Carta* cartaActual = (Carta*) shmat(shmId, (void*)0, 0);
    
    // Inicializar variables del juego
    Jugador jugadores[4];
    vector<Carta> mazo;
    stack<Carta> pilaDescarte;

    // Inicializar y barajar el mazo
    inicializarMazo(mazo);
    barajarMazo(mazo);
    repartirCartas(mazo, jugadores, 4);

    // Colocar una primera carta en la pila de descarte
    pilaDescarte.push(mazo.back());
    mazo.pop_back();

    // Inicializar semáforos
    sem_t semaforo;
    sem_init(&semaforo, 1, 1); // Inicializa el semáforo en 1 (disponible)
    bool esTurno = true; // Variable para controlar los turnos

    pid_t pid;
    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid == 0) {  // Proceso hijo
            if (i == 0) { // El jugador humano será el hijo 0
                while (true) {
                    jugarTurnoHumano(jugadores[0], pilaDescarte, mazo, &semaforo, &esTurno);
                    if (jugadores[0].mano.empty()) {
                        cout << "¡Has ganado, Jugador 1!" << endl;
                        exit(0);
                    }
                }
            } else { // Los otros 3 serán bots
                while (true) {
                    jugarTurnoBot(i, jugadores, pilaDescarte, mazo, &semaforo, &esTurno);
                    if (jugadores[i].mano.empty()) {
                        cout << "¡Jugador " << i + 1 << " (Bot) ha ganado!" << endl;
                        exit(0);
                    }
                }
            }
        }
    }

    // El proceso padre solo espera que los hijos terminen
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    // Liberar memoria compartida y destruir semáforo
    shmdt(cartaActual);
    shmctl(shmId, IPC_RMID, NULL);
    sem_destroy(&semaforo); // Destruir el semáforo

    return 0;
}
