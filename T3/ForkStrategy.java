import java.io.*;

public class ForkStrategy {
    public static char buscarConFork(Matriz matriz) throws IOException {
        ProcessBuilder pb = new ProcessBuilder("java", "ProcesoFork", 
                                                matriz.getNombreArchivo(), 
                                                "0", String.valueOf(matriz.getFilas()), 
                                                "0", String.valueOf(matriz.getColumnas()));
        Process process = pb.start();

        BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
        String resultado = reader.readLine();

        try {
            process.waitFor();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        return resultado != null ? resultado.charAt(0) : ' ';
    }
}
