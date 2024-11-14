import threading
import time
from datetime import datetime
import random
import os

# Constantes
NUMERO_JUGADORES = 256
DURACION_VALIDACION = 15
DURACION_ENFRENTAMIENTO = 10

# Ruta del directorio actual del script
directorio_actual = os.path.dirname(os.path.abspath(__file__))

def registrar_resultado(archivo, contenido):
    # Construir la ruta completa del archivo en el mismo directorio del script
    ruta_completa = os.path.join(directorio_actual, archivo)
    with open(ruta_completa, "a") as f:
        f.write(contenido + "\n")


# Fase de Validación
def fase_validacion(jugadores):
    print("Iniciando fase de validación...")
    for i in range(0, NUMERO_JUGADORES, 32):
        grupo = jugadores[i:i+32]
        threads = []
        for jugador in grupo:
            t = threading.Thread(target=validacion, args=(jugador,))
            threads.append(t)
            t.start()
        for t in threads:
            t.join()

def validacion(jugador_id):
    entrada = datetime.now().strftime("%H:%M:%S.%f")
    time.sleep(DURACION_VALIDACION)
    resultado = f"Hebra{jugador_id} {entrada}, Validación Completa"
    registrar_resultado("Validacion.txt", resultado)

# Fase de Eliminación Directa
def fase_eliminacion(jugadores):
    print("Iniciando fase de eliminación directa...")
    ronda = 1
    perdedores = []
    while len(jugadores) > 1:
        ganadores = []
        threads = []
        for i in range(0, len(jugadores), 2):
            jugador1, jugador2 = jugadores[i], jugadores[i+1]
            t = threading.Thread(target=enfrentamiento, args=(jugador1, jugador2, ganadores, perdedores, ronda, "eliminacion"))
            threads.append(t)
            t.start()
        for t in threads:
            t.join()
        jugadores = ganadores
        ronda += 1
    return jugadores[0], perdedores  # Retorna el ganador final y los perdedores para el repechaje

# Enfrentamiento entre dos jugadores
def enfrentamiento(jugador1, jugador2, ganadores, perdedores, ronda, fase):
    time.sleep(DURACION_ENFRENTAMIENTO)
    ganador = random.choice([jugador1, jugador2])
    perdedor = jugador1 if ganador == jugador2 else jugador2
    ganadores.append(ganador)
    perdedores.append(perdedor)
    timestamp = datetime.now().strftime("%H:%M:%S.%f")
    archivo = f"Ganadores Ronda{ronda}.txt" if fase == "eliminacion" else f"Perdedores Ronda{ronda}.txt"
    registrar_resultado(archivo, f"Hebra{jugador1} vs Hebra{jugador2} {timestamp}, Ganador Hebra{ganador}")

# Fase de Repechaje
# Fase de Repechaje
def fase_repechaje(perdedores):
    print("Iniciando fase de repechaje...")
    ronda = 1
    while len(perdedores) > 1:
        ganadores = []
        threads = []
        i = 0
        while i < len(perdedores) - 1:  # Asegurarse de tener pares
            jugador1, jugador2 = perdedores[i], perdedores[i+1]
            t = threading.Thread(target=enfrentamiento, args=(jugador1, jugador2, ganadores, perdedores, ronda, "repechaje"))
            threads.append(t)
            t.start()
            i += 2
        for t in threads:
            t.join()
        
        # Si hay un jugador impar, avanza automáticamente a la siguiente ronda
        if len(perdedores) % 2 == 1:
            ganadores.append(perdedores[-1])
        
        perdedores = ganadores
        ronda += 1
    return perdedores[0]  # Retorna el ganador del repechaje


# Fase Final
def fase_final(ganador_eliminacion, ganador_repechaje):
    print("Iniciando la fase final...")
    time.sleep(DURACION_ENFRENTAMIENTO)
    ganador = random.choice([ganador_eliminacion, ganador_repechaje])
    timestamp = datetime.now().strftime("%H:%M:%S.%f")
    registrar_resultado("Final.txt", f"Hebra{ganador_eliminacion} vs Hebra{ganador_repechaje} {timestamp}, Ganador Hebra{ganador}")
    print(f"¡El ganador del torneo es Hebra{ganador}!")

# Ejecución principal
if __name__ == "__main__":
    jugadores = list(range(1, NUMERO_JUGADORES + 1))
    fase_validacion(jugadores)
    
    # Fase de eliminación directa
    ganador_eliminacion, perdedores = fase_eliminacion(jugadores)
    
    # Fase de repechaje
    ganador_repechaje = fase_repechaje(perdedores)
    
    # Fase final
    fase_final(ganador_eliminacion, ganador_repechaje)
