#include <iostream>
#include <vector>
#include <stack>
#include <cctype>
#include <unistd.h>  // Para fork()
#include <sys/ipc.h> // Para memoria compartida
#include <sys/shm.h>
#include <ctime>     // Para generar aleatoriedad
#include <cstdlib>   // Para srand y rand
#include <wait.h>    // Para wait()
#include <semaphore.h> // Para semáforos
#include <atomic>    // Para control de finalización del juego
#include <cstring>

using namespace std;

struct Carta {
    int numero;
    char color[10]; // Usamos un array de caracteres en lugar de std::string
    bool esEspecial;
    char tipoEspecial[10]; // También para el tipo especial
};

struct Jugador {
    vector<Carta> mano;
};

// Estructura para la pila de descartes compartida
struct PilaCompartida {
    Carta pila[108];  // Máximo de 108 cartas en la pila
    int tope;         // Índice que apunta al tope de la pila
};

// Variables globales para la sincronización y finalización del juego
sem_t semaforoTurno;   // Semáforo para controlar los turnos
atomic<bool> juegoTerminado(false);  // Controlar la finalización del juego

struct Compartido {
    int turnoActual;  // Variable compartida para indicar el turno actual
};

void mostrarCartas(const vector<Carta>& mano) {
    cout << "Tus cartas:" << endl;
    for (size_t i = 0; i < mano.size(); i++) {
        cout << i + 1 << ": " << (mano[i].esEspecial ? mano[i].tipoEspecial : to_string(mano[i].numero)) << " " <<  mano[i].color<< endl;
    }
}

void inicializarMazo(vector<Carta>& mazo) {
    const char* colores[] = {"rojo", "amarillo", "verde", "azul"};
    for (const auto& color : colores) {
        for (int num = 0; num <= 9; ++num) {
            Carta carta = {num, "", false, ""};
            strncpy(carta.color, color, sizeof(carta.color) - 1);
            mazo.push_back(carta);
            if (num != 0) mazo.push_back(carta);  // El 0 no se repite
        }
    }

    for (const auto& color : colores) {
        for (int i = 0; i < 2; i++) {
            Carta cambioSentido = {-1, "", true, "cambio"};
            Carta salta = {-1, "", true, "salta"};
            Carta robaDos = {-1, "", true, "+2"};
            strncpy(cambioSentido.color, color, sizeof(cambioSentido.color) - 1);
            strncpy(salta.color, color, sizeof(salta.color) - 1);
            strncpy(robaDos.color, color, sizeof(robaDos.color) - 1);
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

void pushCarta(PilaCompartida* pila, Carta carta) {
    if (pila->tope < 107) {
        pila->pila[++(pila->tope)] = carta;
    }
}

Carta topCarta(PilaCompartida* pila) {
    if (pila->tope >= 0) {
        return pila->pila[pila->tope];
    } else {
        cerr << "Error: Pila de descartes vacía." << endl;
        exit(1); // O maneja el error de otra manera
    }
}

void jugarTurnoHumano(Jugador& jugador, PilaCompartida* pilaDescarte, vector<Carta>& mazo, Compartido* turnoCompartido, int miTurno) {
    while (turnoCompartido->turnoActual != miTurno) {
        // Esperar a que sea el turno del jugador humano
        sleep(1);
    }

    // Turno del jugador humano
    Carta cartaActual = topCarta(pilaDescarte);  // Obtener la carta en la cima de la pila desde la memoria compartida
    cout << "Carta en la pila de descarte: " << cartaActual.color << " " 
        << (cartaActual.esEspecial ? cartaActual.tipoEspecial : to_string(cartaActual.numero)) << endl;

    bool jugada = false;
    if (strcmp(cartaActual.tipoEspecial, "salta") == 0) {
        jugada = true;
        cout << "El jugador anterior ocupó una carta 'Salta', perdiste tu turno." << endl;
    } else if (strcmp(cartaActual.tipoEspecial, "+2") == 0 || strcmp(cartaActual.tipoEspecial, "+4") == 0)
 {
        jugada = true;
        int cont = 0;

        // Robar cartas del mazo según el efecto de +2 o +4
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            jugador.mano.push_back(cartaRobada);
            cont += 1;
        }
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            jugador.mano.push_back(cartaRobada);
            cont += 1;
        }

        // Si es +4, robar dos cartas más
        if (strcmp(cartaActual.tipoEspecial, "+4") == 0) {
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                jugador.mano.push_back(cartaRobada);
                cont += 1;
            }
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                jugador.mano.push_back(cartaRobada);
                cont += 1;
            }
            cout << "El jugador anterior ocupó una carta '+4', robaste " << cont << " cartas del mazo y perdiste tu turno." << endl;
        } else {
            cout << "El jugador anterior ocupó una carta '+2', robaste " << cont << " cartas del mazo y perdiste tu turno." << endl;
        }
    }

    // Mostrar las cartas del jugador después de robar
    mostrarCartas(jugador.mano);

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
                
                if (strcmp(cartaRobada.color, cartaActual.color) == 0 || (cartaRobada.numero == cartaActual.numero && !cartaRobada.esEspecial) || (cartaRobada.esEspecial && strcmp(cartaRobada.tipoEspecial, cartaActual.tipoEspecial) == 0)){
                    pushCarta(pilaDescarte, cartaRobada);
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
            if (strcmp(cartaElegida.color, cartaActual.color) == 0 || (cartaElegida.numero == cartaActual.numero && !cartaElegida.esEspecial) || (cartaElegida.esEspecial && strcmp(cartaElegida.tipoEspecial, cartaActual.tipoEspecial) == 0)) {
                pushCarta(pilaDescarte, cartaElegida);
                jugador.mano.erase(jugador.mano.begin() + opcion - 1);
                cout << "Has jugado: " << cartaElegida.color << " " 
                     << (cartaElegida.esEspecial ? cartaElegida.tipoEspecial : to_string(cartaElegida.numero)) << endl;
                jugada = true;
            } else if (strcmp(cartaElegida.tipoEspecial, "comodin") == 0 || strcmp(cartaElegida.tipoEspecial, "+4") == 0) {
                string input_color;
                cout << endl;
                cout << "Elige el nuevo color: ";
                cin >> input_color;
                cout << endl;
                while (input_color != "rojo" && input_color != "verde" && input_color != "amarillo" && input_color != "azul") {
                    cout << "Ingrese una opción válida: "; 
                    cin >> input_color;
                    cout << endl;
                }

                strncpy(cartaElegida.color, input_color.c_str(), sizeof(cartaElegida.color) - 1);
                cartaElegida.color[sizeof(cartaElegida.color) - 1] = '\0'; // Asegurar el terminador nulo

                pushCarta(pilaDescarte, cartaElegida);
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

    // Verificar si el jugador ha ganado
    if (jugador.mano.empty()) {
        cout << "¡Has ganado!" << endl;
        juegoTerminado = true;  // Finalizar el juego
    }

    // Cambiar el turno al siguiente jugador
    turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;


    sem_post(&semaforoTurno);  // Liberar semáforo para los bots
}

void jugarTurnoBot(int jugadorId, Jugador jugadores[], PilaCompartida* pilaDescarte, vector<Carta>& mazo, Compartido* turnoCompartido, int miTurno) {
    while (turnoCompartido->turnoActual != miTurno) {
        // Esperar a que sea el turno del bot
        sleep(1);
    }

    // Esperar a que el jugador anterior (humano o bot) termine su jugada
    sem_wait(&semaforoTurno);

    if (juegoTerminado) {  // Si el juego ya ha terminado, no hacer nada
        return;
    }

    // Turno del bot
    Carta cartaActual = topCarta(pilaDescarte);  // Obtener la carta en la cima de la pila desde la memoria compartida
    bool jugada = false;
    bool saltar_turno = false;

    if (strcmp(cartaActual.tipoEspecial, "salta") == 0) {
        jugada = true;
        saltar_turno = true;
    } else if (strcmp(cartaActual.tipoEspecial, "+2") == 0 || strcmp(cartaActual.tipoEspecial, "+4") == 0) {
        jugada = true;
        saltar_turno = true;

        // Robar cartas del mazo según el efecto de +2 o +4
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            jugadores[jugadorId].mano.push_back(cartaRobada);
        }
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            jugadores[jugadorId].mano.push_back(cartaRobada);
        }

        // Si es +4, robar dos cartas más
        if (strcmp(cartaActual.tipoEspecial, "+4") == 0) {
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                jugadores[jugadorId].mano.push_back(cartaRobada);
            }
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                jugadores[jugadorId].mano.push_back(cartaRobada);
            }
        }
    }

    // Si hay que saltar turno, intentar jugar una carta
    if (saltar_turno != true){
        for (size_t i = 0; i < jugadores[jugadorId].mano.size(); i++) {
            if (strcmp(jugadores[jugadorId].mano[i].color, cartaActual.color) == 0 || (jugadores[jugadorId].mano[i].numero == cartaActual.numero && !jugadores[jugadorId].mano[i].esEspecial) || (jugadores[jugadorId].mano[i].esEspecial && strcmp(jugadores[jugadorId].mano[i].tipoEspecial, cartaActual.tipoEspecial) == 0)) {
                pushCarta(pilaDescarte, jugadores[jugadorId].mano[i]);
                jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i);
                jugada = true;
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado una carta." << endl;
                break;
            } else if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "comodin") == 0 || strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+4") == 0) {
                int int_color = rand() % 4;
                string colores[] = {"rojo", "amarillo", "verde", "azul"};
                strncpy(jugadores[jugadorId].mano[i].color, colores[int_color].c_str(), sizeof(jugadores[jugadorId].mano[i].color) - 1);
                jugadores[jugadorId].mano[i].color[sizeof(jugadores[jugadorId].mano[i].color) - 1] = '\0';  // Asegurar el terminador nulo
                pushCarta(pilaDescarte, jugadores[jugadorId].mano[i]);
                jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i);
                jugada = true;
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado una carta." << endl;
                break;
            }
        }
    }

    // Intentar jugar una carta si no se ha saltado el turno
    for (size_t i = 0; i < jugadores[jugadorId].mano.size(); i++) {
        cartaActual = topCarta(pilaDescarte);  // Obtener la carta actual desde la pila de memoria compartida
        if (strcmp(jugadores[jugadorId].mano[i].color, cartaActual.color) == 0 || jugadores[jugadorId].mano[i].numero == cartaActual.numero || jugadores[jugadorId].mano[i].esEspecial) {

            pushCarta(pilaDescarte, jugadores[jugadorId].mano[i]);  // Actualizar la pila de descartes en memoria compartida
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
            if (strcmp(cartaRobada.color, cartaActual.color) == 0 || (cartaRobada.numero == cartaActual.numero && !cartaRobada.esEspecial) || (cartaRobada.esEspecial && strcmp(cartaRobada.tipoEspecial, cartaActual.tipoEspecial) == 0)) {
                pushCarta(pilaDescarte, cartaRobada);
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado la carta robada." << endl;
            } else {
                jugadores[jugadorId].mano.push_back(cartaRobada);
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha robado una carta." << endl;
            }
        } else {
            cout << "El mazo está vacío. Jugador " << jugadorId + 1 << " no puede robar." << endl;
        }
    }

    // Verificar si el bot ha ganado
    if (jugadores[jugadorId].mano.empty()) {
        cout << "¡Jugador " << jugadorId + 1 << " (Bot) ha ganado!" << endl;
        juegoTerminado = true;  // Finalizar el juego
    }

    // Cambiar el turno al siguiente jugador
    turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;


    sem_post(&semaforoTurno);  // Liberar el semáforo para el siguiente jugador
}

int main() {
    // Crear memoria compartida para el control de turnos y la pila de descartes
    key_t key1 = ftok("shmfile", 65); 
    int shmId1 = shmget(key1, sizeof(Compartido), 0666 | IPC_CREAT); 
    if (shmId1 < 0) {
        cerr << "Error al crear memoria compartida para turno." << endl;
        exit(1);
    }

    Compartido* turnoCompartido = (Compartido*) shmat(shmId1, nullptr, 0);
    if (turnoCompartido == (Compartido*)(-1)) {
        cerr << "Error al adjuntar la memoria compartida para turno." << endl;
        exit(1);
    }
    turnoCompartido->turnoActual = 0;  // El jugador 0 (humano) empieza

    key_t key2 = ftok("shmfile2", 66); 
    if (key2 == -1) {
        cerr << "Error al generar la clave: " << strerror(errno) << endl;
        return 1;
    }

    int shmId2 = shmget(key2, sizeof(PilaCompartida), 0660 | IPC_CREAT);

    if (shmId2 < 0) {
        cerr << "Error al crear memoria compartida para la pila: " << strerror(errno) << endl;
        exit(1);
    }

    PilaCompartida* pilaDescarte = (PilaCompartida*) shmat(shmId2, nullptr, 0);
    if (pilaDescarte == (PilaCompartida*)(-1)) {
        cerr << "Error al adjuntar la memoria compartida para la pila." << endl;
        exit(1);
    }
    
    memset(pilaDescarte, 0, sizeof(PilaCompartida));

    pilaDescarte->tope = -1; // Inicializar la pila vacía
    // Inicializar variables del juego
    Jugador jugadores[4];
    vector<Carta> mazo;
    // Inicializar y barajar el mazo
    inicializarMazo(mazo);
    barajarMazo(mazo);
    repartirCartas(mazo, jugadores, 4);

    pushCarta(pilaDescarte, mazo.back());
    mazo.pop_back();

    // Inicializar el semáforo
    sem_init(&semaforoTurno, 1, 1);  // El semáforo empieza en 1

    pid_t pid;
    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid == 0) {  // Proceso hijo
            if (i == 0) {  // El jugador humano es el proceso hijo 0
                while (!juegoTerminado) {
                    jugarTurnoHumano(jugadores[0], pilaDescarte, mazo, turnoCompartido, 0);
                }
                exit(0);  // Salir del proceso cuando el juego termine
            } else {  // Los otros 3 son bots
                while (!juegoTerminado) {
                    jugarTurnoBot(i, jugadores, pilaDescarte, mazo, turnoCompartido, i);
                }
                exit(0);  // Salir del proceso cuando el juego termine
            }
        }
    }

    // El proceso padre solo espera a que los hijos terminen
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    // Liberar recursos
    shmdt(turnoCompartido);
    shmctl(shmId1, IPC_RMID, NULL);
    shmdt(pilaDescarte);
    shmctl(shmId2, IPC_RMID, NULL);
    sem_destroy(&semaforoTurno);  // Destruir el semáforo

    return 0;
}
