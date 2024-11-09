# Datos de los integrantes:
Diego Alejandro Muñoz Carrillo, ROL: 202273578-3;
Mariano Alonso Varas Ramos, ROL: 202273611-9;   

# Método del approach al problema
    Búsqueda:

        La búsqueda de la letra oculta en las matrices se realiza utilizando dos estrategias de concurrencia: Forks y Threads.
        Cada método se implementa en una clase dedicada:
            ForkStrategy crea procesos independientes para cada cuadrante de la matriz, facilitando la búsqueda en paralelo.
            ThreadStrategy utiliza hilos dentro del mismo proceso para dividir la matriz en cuadrantes y optimizar la búsqueda mediante concurrencia.
        La simulación se asegura de que ambos métodos realicen la búsqueda de manera recursiva dividiendo la matriz en cuadrantes cada vez más pequeños hasta encontrar la letra oculta.

    Inicialización y Organización del programa:

        Se utiliza una clase Matriz para cargar los datos desde archivos en el directorio archivos_prueba, donde cada archivo representa una matriz con una letra oculta.
        Cada matriz se carga y se procesa secuencialmente, asegurando que cada método de búsqueda (Forks y Threads) actúe sobre los mismos datos para una comparación precisa.
        Los archivos se procesan en orden secuencial, y los resultados se organizan por caso en un archivo Excel (Resultados_Simulacion.xlsx) generado automáticamente.

    Pila de Resultados:

        Los resultados de cada búsqueda (letras encontradas) y los tiempos de ejecución se almacenan en estructuras de datos en Main para analizar el rendimiento.

    Estrategias de Búsqueda:

        Búsqueda en Forks: Utiliza procesos independientes, donde cada proceso busca en un cuadrante de la matriz. El proceso continúa dividiendo recursivamente la matriz en cuadrantes más pequeños, aislando la búsqueda de cada cuadrante en su propio proceso hasta localizar la letra oculta.

        Búsqueda en Threads: Usa hilos dentro del mismo proceso para dividir la matriz en cuadrantes. Cada cuadrante se asigna a un hilo que realiza una búsqueda recursiva, dividiendo el cuadrante en fragmentos cada vez más pequeños. Esto permite optimizar la búsqueda mediante concurrencia en memoria compartida.

    Finalización del Programa:

        El programa finaliza después de procesar todos los archivos en archivos_prueba.

# Ejecución

        Al estar en el directorio de la tarea ejecuta:
            make all **para compilar y ejecutar el programa**

            make build  **para solo compilar**

            make run **para ejecutar despues de compilar**

            make clean **para limpiar los .class generados al compilar**
