import java.io.IOException;

public class ForkStrategy {
    public static char buscarConFork(Matriz matriz) throws IOException {
        // Simulación del fork: Proceso hijo que realiza la búsqueda
        ProcessBuilder pb = new ProcessBuilder("java", "ProcesoFork");
        Process proceso = pb.start();

        // Esperar la finalización del proceso (opcionalmente leer la salida)
        try {
            proceso.waitFor();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // Supongamos que la letra encontrada se imprime en la salida estándar
        // Puedes leer esa letra de la salida del proceso si es necesario.
        return 'A';  // Ejemplo, ajustar según tu implementación.
    }
}
