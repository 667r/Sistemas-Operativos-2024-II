# Datos de los integrantes:
Diego Alejandro Muñoz Carrillo, ROL: 202273578-3
Mariano Alonso Varas Ramos, ROL: 202273611-9

# Supuestos Utilizados 

    Turno del Jugador Humano y Bots:
        Cada jugador (humano o bot) es un proceso hijo. El proceso padre coordina la creación de estos procesos y espera su finalización.
        La memoria compartida almacena el turno actual y la pila de descartes para que todos los jugadores accedan a los datos simultáneamente.
        Semáforos controlan los turnos, asegurando que solo un jugador acceda a los recursos críticos a la vez.

    Inicialización del Juego:
        Se utiliza barajado aleatorio para mezclar el mazo.
        Cada jugador comienza con 7 cartas, y se coloca una carta inicial en la pila de descartes.
        El primer turno corresponde al jugador humano.

    Pila de Descartes:
        Se almacenan las cartas jugadas mediante la memoria compartida.
        El juego verifica constantemente si las cartas jugadas son válidas según el color, número o tipo especial.

    Reglas de las Cartas Especiales:
        Salta: El siguiente jugador pierde su turno.
        +2 y +4: El siguiente jugador roba cartas del mazo (2 o 4, según corresponda) y pierde su turno.
        Cambio de Sentido: No se implementa cambio de orden de turnos ya que no afecta al flujo en un juego de 4 jugadores consecutivos.
        Comodín y +4: Permiten al jugador elegir un nuevo color.

    Finalización del Juego:
        Si un jugador (humano o bot) se queda sin cartas, se declara ganador.
        Si el mazo se vacía antes de que alguien gane, el juego finaliza automáticamente.

# Ejecución

        Al estar en el directorio de la tarea ejecuta:
            make **para compilar el main**

            ./out  **para ejecutar el compilado**

            make clean **para eliminar los binarios generados**