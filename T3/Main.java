import java.io.File;
import java.util.*;

public class Main {
    public static void main(String[] args) {
        String rutaBase = "./archivos_prueba";
        Map<String, List<Character>> resultadosPorCaso = new TreeMap<>(Comparator.comparingInt(Main::extraerNumero));
        long tiempoTotalInicio = System.currentTimeMillis();

        List<File> archivos = ArchivoUtil.listarArchivos(rutaBase);
        if (archivos.isEmpty()) {
            System.out.println("No se encontraron archivos en los subdirectorios.");
            return;
        }

        // Procesar cada archivo con ambas estrategias y medir tiempos
        for (File archivo : archivos) {
            try {
                String nombreCaso = archivo.getParentFile().getName();
                Matriz matriz = new Matriz(archivo.getPath());

                // Comparación entre ThreadStrategy y ForkStrategy
                char letraThread = ejecutarThreadStrategy(matriz);
                char letraFork = ejecutarForkStrategy(matriz);

                // Guardar la letra encontrada (puedes elegir cualquiera)
                resultadosPorCaso.computeIfAbsent(nombreCaso, k -> new ArrayList<>()).add(letraThread);

            } catch (Exception e) {
                System.out.println("Error procesando el archivo: " + archivo.getName());
                e.printStackTrace();
            }
        }

        long tiempoTotalFin = System.currentTimeMillis();
        System.out.println("\n===== Mensaje Final por Caso =====\n");

        // Imprimir el mensaje final por caso
        for (Map.Entry<String, List<Character>> entry : resultadosPorCaso.entrySet()) {
            String nombreCaso = entry.getKey();
            String mensaje = construirMensaje(entry.getValue());
            System.out.println(nombreCaso + ": " + mensaje);
        }

        System.out.println("\nTiempo total: " + (tiempoTotalFin - tiempoTotalInicio) + " ms");
    }

    private static char ejecutarThreadStrategy(Matriz matriz) throws InterruptedException {
        ThreadStrategy thread = new ThreadStrategy(matriz, 0, matriz.getFilas(), 0, matriz.getColumnas());
        long inicio = System.currentTimeMillis();
        thread.start();
        thread.join();  // Esperar a que el hilo termine
        long fin = System.currentTimeMillis();

        System.out.println("ThreadStrategy - Letra: " + thread.getResultado() + " | Tiempo: " + (fin - inicio) + " ms");
        return thread.getResultado();
    }

    private static char ejecutarForkStrategy(Matriz matriz) {
        long inicio = System.currentTimeMillis();
        char letra = ForkStrategy.buscarConFork(matriz);
        long fin = System.currentTimeMillis();

        System.out.println("ForkStrategy - Letra: " + letra + " | Tiempo: " + (fin - inicio) + " ms");
        return letra;
    }

    public static String construirMensaje(List<Character> letras) {
        StringBuilder mensaje = new StringBuilder();
        for (char letra : letras) {
            mensaje.append(letra);
        }
        return mensaje.toString();
    }

    private static int extraerNumero(String nombreCaso) {
        String numero = nombreCaso.replaceAll("\\D", "");
        return numero.isEmpty() ? 0 : Integer.parseInt(numero);
    }
}
