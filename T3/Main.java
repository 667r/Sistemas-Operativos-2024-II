import java.io.File;
import java.util.*;

public class Main {
    public static void main(String[] args) {
        String rutaBase = "./archivos_prueba";  // Ruta relativa al directorio base
        Map<String, List<Character>> resultadosPorCaso = new TreeMap<>(new Comparator<String>() {
            @Override
            public int compare(String caso1, String caso2) {
                return extraerNumero(caso1) - extraerNumero(caso2);  // Ordenar numéricamente
            }
        });

        long tiempoTotalInicio = System.currentTimeMillis();

        List<File> archivos = ArchivoUtil.listarArchivos(rutaBase);
        if (archivos.isEmpty()) {
            System.out.println("No se encontraron archivos en los subdirectorios.");
            return;
        }

        // Procesar cada archivo y agrupar las letras por caso
        for (File archivo : archivos) {
            try {
                String nombreCaso = archivo.getParentFile().getName();  // Obtener el nombre del caso
                Matriz matriz = new Matriz(archivo.getPath());

                long inicio = System.currentTimeMillis();
                char letra = buscarLetraOculta(matriz);
                long fin = System.currentTimeMillis();

                // Guardar la letra en la lista correspondiente al caso
                resultadosPorCaso.computeIfAbsent(nombreCaso, k -> new ArrayList<>()).add(letra);

                System.out.println(letra + "\nTiempo: " + (fin - inicio) + " ms\n");
            } catch (Exception e) {
                System.out.println("Error procesando el archivo: " + archivo.getName());
                e.printStackTrace();
            }
        }

        long tiempoTotalFin = System.currentTimeMillis();
        System.out.println("\n===== Mensaje Final por Caso =====\n");

        // Imprimir el mensaje de cada caso en orden
        for (Map.Entry<String, List<Character>> entry : resultadosPorCaso.entrySet()) {
            String nombreCaso = entry.getKey();
            String mensaje = construirMensaje(entry.getValue());
            System.out.println(nombreCaso + ": " + mensaje);
        }

        System.out.println("\nTiempo total: " + (tiempoTotalFin - tiempoTotalInicio) + " ms");
    }

    // Método para buscar la letra oculta en la matriz
    public static char buscarLetraOculta(Matriz matriz) {
        for (int i = 0; i < matriz.getFilas(); i++) {
            for (int j = 0; j < matriz.getColumnas(); j++) {
                if (matriz.getElemento(i, j) != '0') {
                    return matriz.getElemento(i, j);  // Letra encontrada
                }
            }
        }
        return ' ';  // Caso borde: no se encuentra letra
    }

    // Construir el mensaje final con las letras en orden
    public static String construirMensaje(List<Character> letras) {
        StringBuilder mensaje = new StringBuilder();
        for (char letra : letras) {
            mensaje.append(letra);
        }
        return mensaje.toString();
    }

    // Extraer el número del nombre del caso (e.g., "Caso10" -> 10)
    private static int extraerNumero(String nombreCaso) {
        String numero = nombreCaso.replaceAll("\\D", "");  // Eliminar todo excepto dígitos
        return numero.isEmpty() ? 0 : Integer.parseInt(numero);
    }
}
