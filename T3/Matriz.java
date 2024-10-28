import java.io.*;

public class Matriz {
    private char[][] matriz;
    private int filas, columnas;

    // Constructor que recibe la ruta del archivo
    public Matriz(String archivo) throws IOException {
        cargarDesdeArchivo(archivo);
    }

    // Método para cargar la matriz desde el archivo
    private void cargarDesdeArchivo(String archivo) throws IOException {
        try (BufferedReader br = new BufferedReader(new FileReader(archivo))) {
            // Leer la primera línea y obtener las dimensiones
            String[] dimensiones = br.readLine().trim().split("x");
            filas = Integer.parseInt(dimensiones[0]);
            columnas = Integer.parseInt(dimensiones[1]);
            matriz = new char[filas][columnas];

            // Leer cada línea y llenar la matriz
            for (int i = 0; i < filas; i++) {
                String linea = br.readLine().trim();  // Eliminar espacios extra
                String[] elementos = linea.split("\\s+");  // Separar por espacios

                for (int j = 0; j < columnas; j++) {
                    matriz[i][j] = elementos[j].charAt(0);  // Almacenar el carácter
                }
            }
        }
    }

    // Métodos de acceso y para imprimir la matriz
    public char getElemento(int i, int j) {
        return matriz[i][j];
    }

    public int getFilas() {
        return filas;
    }

    public int getColumnas() {
        return columnas;
    }

    public void imprimirMatriz() {
        for (int i = 0; i < filas; i++) {
            for (int j = 0; j < columnas; j++) {
                System.out.print(matriz[i][j] + " ");
            }
            System.out.println();
        }
    }
}
