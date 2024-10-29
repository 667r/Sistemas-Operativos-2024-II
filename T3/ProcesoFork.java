import java.io.*;

public class ProcesoFork {
    public static void main(String[] args) {
        if (args.length < 5) {
            System.err.println("Parámetros insuficientes.");
            System.exit(1);
        }

        String rutaArchivo = args[0];
        int inicioX = Integer.parseInt(args[1]);
        int finX = Integer.parseInt(args[2]);
        int inicioY = Integer.parseInt(args[3]);
        int finY = Integer.parseInt(args[4]);

        try {
            Matriz matriz = new Matriz(rutaArchivo);
            char letra = buscarLetra(matriz, inicioX, finX, inicioY, finY);
            System.out.println(letra);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static char buscarLetra(Matriz matriz, int inicioX, int finX, int inicioY, int finY) {
        if (inicioX == finX - 1 && inicioY == finY - 1) {
            return matriz.getElemento(inicioX, inicioY);
        }

        int midX = (inicioX + finX) / 2;
        int midY = (inicioY + finY) / 2;

        try {
            ProcessBuilder pb1 = new ProcessBuilder("java", "ProcesoFork", matriz.getNombreArchivo(), 
                                                    String.valueOf(inicioX), String.valueOf(midX), 
                                                    String.valueOf(inicioY), String.valueOf(midY));
            Process p1 = pb1.start();

            ProcessBuilder pb2 = new ProcessBuilder("java", "ProcesoFork", matriz.getNombreArchivo(), 
                                                    String.valueOf(inicioX), String.valueOf(midX), 
                                                    String.valueOf(midY), String.valueOf(finY));
            Process p2 = pb2.start();

            ProcessBuilder pb3 = new ProcessBuilder("java", "ProcesoFork", matriz.getNombreArchivo(), 
                                                    String.valueOf(midX), String.valueOf(finX), 
                                                    String.valueOf(inicioY), String.valueOf(midY));
            Process p3 = pb3.start();

            ProcessBuilder pb4 = new ProcessBuilder("java", "ProcesoFork", matriz.getNombreArchivo(), 
                                                    String.valueOf(midX), String.valueOf(finX), 
                                                    String.valueOf(midY), String.valueOf(finY));
            Process p4 = pb4.start();

            BufferedReader r1 = new BufferedReader(new InputStreamReader(p1.getInputStream()));
            BufferedReader r2 = new BufferedReader(new InputStreamReader(p2.getInputStream()));
            BufferedReader r3 = new BufferedReader(new InputStreamReader(p3.getInputStream()));
            BufferedReader r4 = new BufferedReader(new InputStreamReader(p4.getInputStream()));

            char letra1 = r1.readLine().charAt(0);
            char letra2 = r2.readLine().charAt(0);
            char letra3 = r3.readLine().charAt(0);
            char letra4 = r4.readLine().charAt(0);

            if (letra1 != '0') return letra1;
            if (letra2 != '0') return letra2;
            if (letra3 != '0') return letra3;
            return letra4;
        } catch (IOException e) {
            e.printStackTrace();
            return ' ';
        }
    }
}
