#include <iostream>
#include <vector>
#include <stack>
#include <cctype>
#include <unistd.h>  // fork()
#include <sys/ipc.h> // memoria compartida
#include <sys/shm.h>
#include <ctime>     // cartas aleatorias
#include <cstdlib>   // srand y rand
#include <wait.h>    // wait()
#include <semaphore.h> // semáforos
#include <atomic>    // control de finalización del juego
#include <cstring>

using namespace std;

struct Carta {
    int numero;
    char color[10]; // array de caracteres para el color
    bool esEspecial;
    char tipoEspecial[10]; // array de caracteres para el tipo especial
};

struct Jugador {
    vector<Carta> mano;
};

// struct para la pila de descartes 
struct PilaCompartida {
    Carta pila[108];  // máximo de 108 cartas en la pila
    int tope;         // indice que apunta al tope de la pila
};

// variables globales para la sincronización y finalización del juego
sem_t semaforoTurno;   // semáforo que controla los turnos

struct Compartido {
    int turnoActual;    // variable compartida para indicar el turno actual
    bool sentido_positivo; // variable compartida para indicar el sentido del juego
    int cartas_acumuladas; // variable compartida para indicar la cantidad de cartas acumuladas
    bool juegoTerminado;  // variable compartida para indicar si el juego ha terminado
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
            if (num != 0) mazo.push_back(carta);  // el 0 no se repite
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
        pila->pila[++(pila->tope)] = carta; // incrementar el tope y luego asignar la carta a pone en la pila
    }
}

Carta topCarta(PilaCompartida* pila) {
    if (pila->tope >= 0) {
        return pila->pila[pila->tope];  // retornar la carta en el tope de la pila
    } else {
        cerr << "Error: Pila de descartes vacía." << endl;
        exit(1); // O maneja el error de otra manera
    }
}

void cambiosentido(Compartido* turno){
    if (turno->sentido_positivo == true){
        turno->sentido_positivo = false;
    } else {
        turno->sentido_positivo = true;
    }
}

void añadircartasacumuladas(Compartido* turno, int cant_cartas){
    if (cant_cartas == 0){
        turno->cartas_acumuladas = 0;
    } else {
        turno->cartas_acumuladas += cant_cartas;
    }
}
void jugarTurnoHumano(Jugador& jugador, PilaCompartida* pilaDescarte, vector<Carta>& mazo, Compartido* turnoCompartido, int miTurno) {
    while (turnoCompartido->turnoActual != miTurno) {
        // esperar a que sea el turno del jugador humano
        sleep(1);
    }
    if (turnoCompartido->juegoTerminado == true){
        cout << "Cantidad de cartas del Jugador 1: " << jugador.mano.size() << endl;
        if (turnoCompartido->sentido_positivo == true){
            turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;
        } else {
            turnoCompartido->turnoActual = (turnoCompartido->turnoActual - 1) % 4;
        }
        return;
    }
    // turno del jugador humano
    Carta cartaActual = topCarta(pilaDescarte);  // obtiene la carta que esta arriba en la pila de descartes
    cout << "Carta en la pila de descarte: " << cartaActual.color << " " 
        << (cartaActual.esEspecial ? cartaActual.tipoEspecial : to_string(cartaActual.numero)) << endl;

    bool jugada = false;
    bool respondido = false;
    if (strcmp(cartaActual.tipoEspecial, "salta") == 0) {
        jugada = true;
        cout << "El jugador anterior ocupó una carta 'Salta', perdiste tu turno." << endl;
        strncpy(cartaActual.tipoEspecial, "salta (U)", sizeof(cartaActual.tipoEspecial) - 1);
        cartaActual.tipoEspecial[sizeof(cartaActual.tipoEspecial) - 1] = '\0';
        pushCarta(pilaDescarte, cartaActual);
    } else if (strcmp(cartaActual.tipoEspecial, "+2") == 0 || strcmp(cartaActual.tipoEspecial, "+4") == 0){
        jugada = true;
        int cont = 0;
        bool respondido = false;
        for (int i = 0; i < jugador.mano.size(); i++){
            if (strcmp(jugador.mano[i].tipoEspecial, "+2") == 0 || strcmp(jugador.mano[i].tipoEspecial, "+4") == 0){
                while (respondido == false){
                    cout << "Tiene cartas + disponibles para responder, elija si ocupar una de ellas o robar " << cartaActual.tipoEspecial[1] << " cartas." << endl;
                    mostrarCartas(jugador.mano);
                    cout << "Selecciona una carta para jugar (0 para robar): ";
                    int opcion;
                    cin >> opcion;
                    if (opcion == 0) {
                        while (cont < (turnoCompartido->cartas_acumuladas)){
                            if (!mazo.empty()) {
                                Carta cartaRobada = mazo.back();
                                mazo.pop_back();
                                jugador.mano.push_back(cartaRobada);
                                cont += 1;
                                cout << "Has robado: " << cartaRobada.color << " " 
                                << (cartaRobada.esEspecial ? cartaRobada.tipoEspecial : to_string(cartaRobada.numero)) << endl;
                            }
                        }
                        añadircartasacumuladas(turnoCompartido, 0);
                        respondido = true;
                    } else if (opcion > 0 && opcion <= jugador.mano.size()) {
                        while (true){
                            Carta cartaElegida = jugador.mano[opcion - 1];
                            if (strcmp(cartaElegida.tipoEspecial, "+2") == 0 || strcmp(cartaElegida.tipoEspecial, "+4") == 0){
                                pushCarta(pilaDescarte, cartaElegida);
                                if (strcmp(cartaElegida.tipoEspecial, "+2") == 0){
                                    añadircartasacumuladas(turnoCompartido, 2);
                                } else {
                                    string input_color;
                                    cout << "Elige el nuevo color: ";
                                    cin >> input_color;
                                    cout << endl;
                                    while (input_color != "rojo" && input_color != "verde" && input_color != "amarillo" && input_color != "azul") {
                                        cout << "Ingrese una opción válida: "; 
                                        cin >> input_color;
                                        cout << endl;
                                    }

                                    strncpy(cartaElegida.color, input_color.c_str(), sizeof(cartaElegida.color) - 1);
                                    cartaElegida.color[sizeof(cartaElegida.color) - 1] = '\0'; // asegurar el terminador nulo
                                    añadircartasacumuladas(turnoCompartido, 4);
                                }
                                pushCarta(pilaDescarte, cartaElegida);
                                jugador.mano.erase(jugador.mano.begin() + opcion - 1);
                                cout << "Has jugado: " << cartaElegida.color << " " 
                                << (cartaElegida.esEspecial ? cartaElegida.tipoEspecial : to_string(cartaElegida.numero)) << endl;
                                jugada = true;
                                respondido = true;
                                break;
                            } else {
                                cout << "La carta seleccionada debe ser del tipo +" << endl;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
        if (respondido == false){
            while (cont < turnoCompartido->cartas_acumuladas){
                if (!mazo.empty()) {
                    Carta cartaRobada = mazo.back();
                    mazo.pop_back();
                    jugador.mano.push_back(cartaRobada);
                    cont += 1;
                    cout << "Has robado: " << cartaRobada.color << " " 
                    << (cartaRobada.esEspecial ? cartaRobada.tipoEspecial : to_string(cartaRobada.numero)) << endl;
                }
            }
            añadircartasacumuladas(turnoCompartido, 0);
            cout << "Debido a la carta + y que no había ninguna carta con la que responder, perdiste tu turno." << endl;
            strncpy(cartaActual.tipoEspecial, "+ (U)", sizeof(cartaActual.tipoEspecial) - 1);
            cartaActual.tipoEspecial[sizeof(cartaActual.tipoEspecial) - 1] = '\0';
            pushCarta(pilaDescarte, cartaActual);
            respondido = true;
            jugada = true;
        }
    }

    // mostrar las cartas del jugador después de robar
    if (respondido == false){
        mostrarCartas(jugador.mano);
    }

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
                
                if (strcmp(cartaRobada.color, cartaActual.color) == 0 || (cartaRobada.numero == cartaActual.numero && !cartaRobada.esEspecial) || (cartaRobada.esEspecial && strcmp(cartaRobada.tipoEspecial, cartaActual.tipoEspecial) == 0) || (cartaRobada.tipoEspecial[0] == '+') && (cartaActual.tipoEspecial[0] == '+') || (strcmp(cartaRobada.tipoEspecial, "salta") == 0 && strcmp(cartaActual.tipoEspecial, "salta (U)") == 0)){
                    if (strcmp(cartaRobada.tipoEspecial, "cambio") == 0){
                        cambiosentido(turnoCompartido);
                    } else if (strcmp(cartaRobada.tipoEspecial, "+2") == 0){
                        añadircartasacumuladas(turnoCompartido, 2);
                    } else if (strcmp(cartaRobada.tipoEspecial, "+4") == 0 || (strcmp(cartaRobada.tipoEspecial, "comodin") == 0))  {
                        string input_color;
                        cout << "Elige el nuevo color: ";
                        cin >> input_color;
                        cout << endl;
                        while (input_color != "rojo" && input_color != "verde" && input_color != "amarillo" && input_color != "azul") {
                            cout << "Ingrese una opción válida: "; 
                            cin >> input_color;
                            cout << endl;
                        }

                        strncpy(cartaRobada.color, input_color.c_str(), sizeof(cartaRobada.color) - 1);
                        cartaRobada.color[sizeof(cartaRobada.color) - 1] = '\0'; // Asegurar el terminador nulo
                        if (strcmp(cartaRobada.tipoEspecial, "+4") == 0){
                            añadircartasacumuladas(turnoCompartido, 4);
                        }
                    }
                    pushCarta(pilaDescarte, cartaRobada);
                    cout << "Has jugado la carta robada." << endl;
                    jugada = true;
                } else {
                    jugador.mano.push_back(cartaRobada);
                    jugada = true;
                }
            } else {
                cout << "El mazo está vacío." << endl;
                jugada = true;
            }
        } else if (opcion > 0 && opcion <= jugador.mano.size()) {
            Carta cartaElegida = jugador.mano[opcion - 1];
            if (strcmp(cartaElegida.color, cartaActual.color) == 0 || (cartaElegida.numero == cartaActual.numero && !cartaElegida.esEspecial) || (cartaElegida.esEspecial && strcmp(cartaElegida.tipoEspecial, cartaActual.tipoEspecial) == 0) || (cartaElegida.tipoEspecial[0] == '+') && (cartaActual.tipoEspecial[0] == '+') || (strcmp(cartaElegida.tipoEspecial, "salta") == 0 && strcmp(cartaActual.tipoEspecial, "salta (U)") == 0)) {
                if (strcmp(cartaElegida.tipoEspecial, "cambio") == 0){
                    cambiosentido(turnoCompartido);
                } else if (strcmp(cartaElegida.tipoEspecial, "+2") == 0){
                    añadircartasacumuladas(turnoCompartido, 2);
                }
                pushCarta(pilaDescarte, cartaElegida);
                jugador.mano.erase(jugador.mano.begin() + opcion - 1);
                cout << "Has jugado: " << cartaElegida.color << " " 
                << (cartaElegida.esEspecial ? cartaElegida.tipoEspecial : to_string(cartaElegida.numero)) << endl;
                jugada = true;
            } else if (strcmp(cartaElegida.tipoEspecial, "comodin") == 0 || strcmp(cartaElegida.tipoEspecial, "+4") == 0) {
                if (strcmp(cartaElegida.tipoEspecial, "+4") == 0){
                    añadircartasacumuladas(turnoCompartido, 4);
                }
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

    // verificar si el jugador ha ganado
    if (jugador.mano.empty()) {
        cout << "¡Has ganado!" << endl;
        turnoCompartido->juegoTerminado = true;  // finalizar el juego
    }

    // cambiar el turno al siguiente jugador
    if (turnoCompartido->sentido_positivo == true){
        turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;
    } else {
        turnoCompartido->turnoActual = 3;
    }

    if (turnoCompartido->juegoTerminado == true){
        cout << "Cantidad de cartas del Jugador 1: " << jugador.mano.size() << endl;
    }


    sem_post(&semaforoTurno);  // liberar semáforo para los bots
}

void jugarTurnoBot(int jugadorId, Jugador jugadores[], PilaCompartida* pilaDescarte, vector<Carta>& mazo, Compartido* turnoCompartido, int miTurno) {
    while (turnoCompartido->turnoActual != miTurno) {
        // esperar a que sea el turno del bot
        sleep(1);
    }

    // esperar a que el jugador anterior (humano o bot) termine su jugada
    sem_wait(&semaforoTurno);

    if (turnoCompartido->juegoTerminado == true){
        cout << "Cantidad de cartas del Jugador" << jugadorId + 1 << " (Bot): " << jugadores[jugadorId].mano.size() << endl;
        if (turnoCompartido->sentido_positivo == true){
            turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;
        } else {
            turnoCompartido->turnoActual = (turnoCompartido->turnoActual - 1) % 4;
        }
        return;
    }

    // turno del bot
    Carta cartaActual = topCarta(pilaDescarte);  // obtiene la carta que esta arriba en la pila de descartes
    bool jugada = false;
    bool saltar_turno = false;
    if (strcmp(cartaActual.tipoEspecial, "salta") == 0) {
        jugada = true;
        saltar_turno = true;
        strncpy(cartaActual.tipoEspecial, "salta (U)", sizeof(cartaActual.tipoEspecial) - 1);
        cartaActual.tipoEspecial[sizeof(cartaActual.tipoEspecial) - 1] = '\0';
        pushCarta(pilaDescarte, cartaActual);
    } else if (strcmp(cartaActual.tipoEspecial, "+2") == 0 || strcmp(cartaActual.tipoEspecial, "+4") == 0) {
        jugada = true;
        saltar_turno = true;
        bool respondido = false;
        for (int i = 0; i < jugadores[jugadorId].mano.size(); i++){
            if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+2") == 0 || strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+4") == 0){
                Carta cartaElegida = jugadores[jugadorId].mano[i];
                if (strcmp(cartaElegida.tipoEspecial, "+2") == 0){
                    añadircartasacumuladas(turnoCompartido, 2);
                } else if (strcmp(cartaElegida.tipoEspecial, "+4") == 0){
                    añadircartasacumuladas(turnoCompartido, 4);
                    int int_color = rand() % 4;
                    string colores[] = {"rojo", "amarillo", "verde", "azul"};
                    strncpy(jugadores[jugadorId].mano[i].color, colores[int_color].c_str(), sizeof(jugadores[jugadorId].mano[i].color) - 1);
                    jugadores[jugadorId].mano[i].color[sizeof(jugadores[jugadorId].mano[i].color) - 1] = '\0';  // Asegurar el terminador nulo
                }
                pushCarta(pilaDescarte, jugadores[jugadorId].mano[i]);
                jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i);
                jugada = true;
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado una carta." << endl;
                respondido = true;
                break;
            }
        }
        if (respondido == false){
        for (int cont = 0; cont < turnoCompartido->cartas_acumuladas; cont++){
            if (!mazo.empty()) {
                Carta cartaRobada = mazo.back();
                mazo.pop_back();
                jugadores[jugadorId].mano.push_back(cartaRobada);
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha robado una carta." << endl;
                }        
            }
            añadircartasacumuladas(turnoCompartido, 0);
            strncpy(cartaActual.tipoEspecial, "+ (U)", sizeof(cartaActual.tipoEspecial) - 1);
            cartaActual.tipoEspecial[sizeof(cartaActual.tipoEspecial) - 1] = '\0';
            pushCarta(pilaDescarte, cartaActual); 
        }
    }

    // si hay que saltar turno, intentar jugar una carta
    if (saltar_turno != true){
        for (size_t i = 0; i < jugadores[jugadorId].mano.size(); i++) {
            if (strcmp(jugadores[jugadorId].mano[i].color, cartaActual.color) == 0 || (jugadores[jugadorId].mano[i].numero == cartaActual.numero && !jugadores[jugadorId].mano[i].esEspecial) || (jugadores[jugadorId].mano[i].esEspecial && strcmp(jugadores[jugadorId].mano[i].tipoEspecial, cartaActual.tipoEspecial) == 0) || (jugadores[jugadorId].mano[i].tipoEspecial[0] == '+' && cartaActual.tipoEspecial[0] == '+') || (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "salta") == 0 && strcmp(cartaActual.tipoEspecial, "salta (U)") == 0)) {
                if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "cambio") == 0){
                    cambiosentido(turnoCompartido);
                } else if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+2") == 0){
                    añadircartasacumuladas(turnoCompartido, 2);
                }
                pushCarta(pilaDescarte, jugadores[jugadorId].mano[i]);
                jugadores[jugadorId].mano.erase(jugadores[jugadorId].mano.begin() + i);
                jugada = true;
                cout << "Jugador " << jugadorId + 1 << " (Bot) ha jugado una carta." << endl;
                break;
            } else if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "comodin") == 0 || strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+4") == 0) {
                if (strcmp(jugadores[jugadorId].mano[i].tipoEspecial, "+4") == 0){
                    añadircartasacumuladas(turnoCompartido, 4);
                }
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

    if (!jugada) {
        if (!mazo.empty()) {
            Carta cartaRobada = mazo.back();
            mazo.pop_back();
            if (strcmp(cartaRobada.color, cartaActual.color) == 0 || (cartaRobada.numero == cartaActual.numero && !cartaRobada.esEspecial) || (cartaRobada.esEspecial && strcmp(cartaRobada.tipoEspecial, cartaActual.tipoEspecial) == 0) || (cartaRobada.tipoEspecial[0] == '+' && cartaActual.tipoEspecial[0] == '+') || (strcmp(cartaRobada.tipoEspecial, "salta") == 0 && strcmp(cartaRobada.tipoEspecial, "salta (U)") == 0)) {
                if (strcmp(cartaRobada.tipoEspecial, "+4") == 0 || (strcmp(cartaRobada.tipoEspecial, "comodin") == 0)){
                    if (strcmp(cartaRobada.tipoEspecial, "+4") == 0){    
                        añadircartasacumuladas(turnoCompartido, 4);
                    }
                    int int_color = rand() % 4;
                    string colores[] = {"rojo", "amarillo", "verde", "azul"};
                    strncpy(cartaRobada.color, colores[int_color].c_str(), sizeof(cartaRobada.color) - 1);
                    cartaRobada.color[sizeof(cartaRobada.color) - 1] = '\0';  // asegurar el terminador nulo
                } else if (strcmp(cartaRobada.tipoEspecial, "+2") == 0) {
                    añadircartasacumuladas(turnoCompartido, 2);
                } else if (strcmp(cartaRobada.tipoEspecial, "cambio") == 0){
                    cambiosentido(turnoCompartido);
                }
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

    // verificar si el bot ha ganado
    if (jugadores[jugadorId].mano.empty()) {
        cout << "¡Jugador " << jugadorId + 1 << " (Bot) ha ganado!" << endl;
        turnoCompartido->juegoTerminado = true;  // finalizar el juego
    }

    // cambiar el turno al siguiente jugador
    if (turnoCompartido->sentido_positivo == true){
        turnoCompartido->turnoActual = (turnoCompartido->turnoActual + 1) % 4;
    } else {
        turnoCompartido->turnoActual = (turnoCompartido->turnoActual - 1) % 4;
    }

    if (turnoCompartido->juegoTerminado == true){
        cout << "Cantidad de cartas del Jugador " << jugadorId + 1 << " (Bot): " << jugadores[jugadorId].mano.size() << endl;
    }


    sem_post(&semaforoTurno);  // liberar el semáforo para el siguiente jugador
}

int main() {

    // crear memoria compartida para el control de turnos y la pila de descartes
    key_t key1 = ftok("shmfile", 65);
    if (key1 == -1) {
        cerr << "Error al generar la clave 1." << endl;
        exit(1);
    }

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
    turnoCompartido->turnoActual = 0;
    turnoCompartido->sentido_positivo = true;
    turnoCompartido->cartas_acumuladas = 0;  // el jugador 0 (humano) empieza
    turnoCompartido->juegoTerminado = false;

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

    pilaDescarte->tope = -1; // inicializar la pila de descartes, el mazo, y los jugadores
    Jugador jugadores[4];
    vector<Carta> mazo;
    inicializarMazo(mazo);
    barajarMazo(mazo);
    repartirCartas(mazo, jugadores, 4);
    if (mazo.back().esEspecial){
        while (mazo.back().esEspecial){
            barajarMazo(mazo);
        }
    }
    pushCarta(pilaDescarte, mazo.back());
    mazo.pop_back();
    Carta cartaActual = topCarta(pilaDescarte);
    if (strcmp(cartaActual.tipoEspecial, "+2") == 0){
        añadircartasacumuladas(turnoCompartido, 2);
    } else if (strcmp(cartaActual.tipoEspecial, "+4") == 0){
        añadircartasacumuladas(turnoCompartido, 4);
    }
    // inicializar el semáforo
    sem_init(&semaforoTurno, 1, 1);  // semaforo en 1 para que juegue el jugador humano

    pid_t pid;
    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid == 0) {  // proceso hijo
            if (i == 0) {  // el jugador humano es el proceso hijo 0
                while (!turnoCompartido->juegoTerminado) {
                    jugarTurnoHumano(jugadores[0], pilaDescarte, mazo, turnoCompartido, 0);
                }
                exit(0);  // cuando alguien gana, el proceso hijo termina
            } else {  // los otros 3 son bots
                while (!turnoCompartido->juegoTerminado) {
                    jugarTurnoBot(i, jugadores, pilaDescarte, mazo, turnoCompartido, i);
                }
                exit(0);  // cuando alguien gana, el proceso hijo termina
            }
        }
    }

    // esperar a que terminen los procesos hijos
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    // liberar recursos
    shmdt(turnoCompartido);
    shmctl(shmId1, IPC_RMID, NULL);
    shmdt(pilaDescarte);
    shmctl(shmId2, IPC_RMID, NULL);
    sem_destroy(&semaforoTurno);  // liberar el semáforo

    return 0;
}