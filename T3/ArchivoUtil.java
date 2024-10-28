import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

public class ArchivoUtil {

    // Método para obtener y ordenar archivos en orden ascendente
    public static List<File> listarArchivos(String rutaBase) {
        List<File> archivos = new ArrayList<>();
        File directorioBase = new File(rutaBase);

        if (directorioBase.exists() && directorioBase.isDirectory()) {
            for (File subdirectorio : directorioBase.listFiles()) {
                if (subdirectorio.isDirectory()) {
                    File[] archivosEnSubdir = subdirectorio.listFiles(
                        (dir, name) -> name.endsWith("")
                    );

                    // Ordenar archivos numéricamente según el nombre (code1, code2, ...)
                    if (archivosEnSubdir != null) {
                        Arrays.sort(archivosEnSubdir, Comparator.comparingInt(
                            f -> extraerNumero(f.getName())
                        ));
                        archivos.addAll(Arrays.asList(archivosEnSubdir));
                    }
                }
            }
        }
        return archivos;
    }

    // Función auxiliar para extraer el número del nombre del archivo (e.g., "code10" -> 10)
    private static int extraerNumero(String nombre) {
        String numero = nombre.replaceAll("\\D", "");  // Elimina todo lo que no sea dígito
        return numero.isEmpty() ? 0 : Integer.parseInt(numero);
    }
}
