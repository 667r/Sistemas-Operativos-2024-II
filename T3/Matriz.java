import java.io.*;

public class Matriz {
    private char[][] matriz;
    private int filas, columnas;
    private String nombreArchivo;  // Almacena la ruta del archivo

    // Constructor que recibe la ruta del archivo
    public Matriz(String archivo) throws IOException {
        this.nombreArchivo = archivo;  // Guardar la ruta del archivo
        cargarDesdeArchivo(archivo);
    }

    // Método para cargar la matriz desde el archivo
    private void cargarDesdeArchivo(String archivo) throws IOException {
        try (BufferedReader br = new BufferedReader(new FileReader(archivo))) {
            // Leer las dimensiones desde la primera línea
            String[] dimensiones = br.readLine().trim().split("x");
            filas = Integer.parseInt(dimensiones[0]);
            columnas = Integer.parseInt(dimensiones[1]);
            matriz = new char[filas][columnas];

            // Leer y llenar la matriz
            for (int i = 0; i < filas; i++) {
                String[] elementos = br.readLine().trim().split("\\s+");
                for (int j = 0; j < columnas; j++) {
                    matriz[i][j] = elementos[j].charAt(0);
                }
            }
        }
    }

    // Método para obtener el nombre del archivo
    public String getNombreArchivo() {
        return nombreArchivo;
    }

    // Métodos de acceso
    public char getElemento(int i, int j) {
        return matriz[i][j];
    }

    public int getFilas() {
        return filas;
    }

    public int getColumnas() {
        return columnas;
    }
}
