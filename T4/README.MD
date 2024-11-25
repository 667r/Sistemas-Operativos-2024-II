# Datos de los integrantes:
Diego Alejandro Muñoz Carrillo, ROL: 202273578-3;
Mariano Alonso Varas Ramos, ROL: 202273611-9;   

# Nombres de archivos de registro:
Todas las validaciones de los jugadores se registrarán en el archivo: "Validacion.txt".

Para la fase de eliminación y repechaje, se recurre a la función enfrentamiento, la cual registra en archivos los resultados de la siguiente forma:
    archivo = f"Ganadores Ronda{ronda}.txt" if fase == "eliminacion" else f"Perdedores Ronda{ronda}.txt"
    registrar_resultado(archivo, f"Hebra{jugador1} vs Hebra{jugador2} {timestamp}, Ganador Hebra{ganador}")
Donde fase corresponde a "eliminacion" o "repechaje"; "jugadorx" con x = {1,2} al jugador respectivo que participó en la ronda, se añade el ganador y perdedor de la ronda a cada jugador que cumpla con tal característica a ganadores y perdedores. 

Mientras que para la fase final, se registra el resultado respectivo en "Final.txt".

# Instrucciones de ejecución:
Para poder ejecutar el código, se necesitan tener todos los archivos de prueba necesarios en la misma carpeta que el archivo "t4.py". Se necesita tener un compilador del lenguaje Python instalado en la máquina donde se ejecutará el archivo. Luego, ejecutar el archivo como cualquier otro escrito en lenguaje Python.