#include <iostream>
#include <vector>
#include <unistd.h>  // Para fork()
#include <sys/ipc.h> // Para memoria compartida
#include <sys/shm.h>
#include <ctime>     // Para generar aleatoriedad
#include <cstdlib>   // Para srand y rand
#include <wait.h>    // Para wait()

using namespace std;

// Estructuras básicas para las cartas y jugadores
struct Carta {
    int numero;
    string color;
    bool esEspecial; // Para las cartas especiales
};

struct Jugador {
    vector<Carta> mano;
};

void inicializarJuego(Jugador jugadores[], Carta mazo[], int &mazoIndex, int shmId) {
    // Código para inicializar el mazo y repartir las cartas a los jugadores
    cout << "Inicializando el juego y repartiendo cartas..." << endl;
    // Ejemplo básico: repartir 7 cartas a cada jugador
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            jugadores[i].mano.push_back(mazo[mazoIndex++]);
        }
    }
}

void jugarTurno(int jugadorId, Jugador jugadores[], Carta &cartaActual, int shmId) {
    // Lógica para que el jugador juegue su turno
    cout << "Jugador " << jugadorId + 1 << " jugando su turno." << endl;
    // Ejemplo: el jugador intenta jugar una carta que coincida en color o número
    for (size_t i = 0; i < jugadores[jugadorId].mano.size(); i++) {
        if (jugadores[jugadorId].mano[i].color == cartaActual.color || jugadores[jugadorId].mano[i].numero == cartaActual.numero) {
            cartaActual = jugadores[jugadorId].mano[i]; // Jugar la carta
            jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i); // Eliminar carta de la mano
            cout << "Jugador " << jugadorId + 1 << " ha jugado una carta." << endl;
            break;
        }
    }
}

int main() {
    // Crear memoria compartida
    key_t key = ftok("shmfile", 65); 
    int shmId = shmget(key, 1024, 0666 | IPC_CREAT); 
    Carta* cartaActual = (Carta*) shmat(shmId, (void*)0, 0); // Carta actual en la pila de descarte
    
    // Inicializar variables del juego
    Jugador jugadores[4];
    Carta mazo[108]; // 108 cartas del juego Uno
    int mazoIndex = 0;

    // Inicializar el juego
    inicializarJuego(jugadores, mazo, mazoIndex, shmId);

    // Crear los procesos para los jugadores
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Proceso hijo
            while (true) {
                jugarTurno(i, jugadores, *cartaActual, shmId);
                // Código para verificar si el jugador ha ganado
                if (jugadores[i].mano.empty()) {
                    cout << "Jugador " << i + 1 << " ha ganado!" << endl;
                    exit(0);
                }
                sleep(1); // Pausar un poco para simular el tiempo de juego
            }
        }
    }

    // Esperar a que todos los hijos terminen
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    // Liberar memoria compartida
    shmdt(cartaActual);
    shmctl(shmId, IPC_RMID, NULL);

    return 0;
}
