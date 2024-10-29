import java.util.concurrent.RecursiveTask;

public class ForkJoinStrategy extends RecursiveTask<Character> {
    private Matriz matriz;
    private int inicioX, finX, inicioY, finY;

    public ForkJoinStrategy(Matriz matriz, int inicioX, int finX, int inicioY, int finY) {
        this.matriz = matriz;
        this.inicioX = inicioX;
        this.finX = finX;
        this.inicioY = inicioY;
        this.finY = finY;
    }

    @Override
    protected Character compute() {
        // Caso base: si es una celda única, devolvemos su contenido
        if (inicioX == finX - 1 && inicioY == finY - 1) {
            return matriz.getElemento(inicioX, inicioY);
        }

        // Dividimos en cuadrantes
        int midX = (inicioX + finX) / 2;
        int midY = (inicioY + finY) / 2;

        // Crear tareas recursivas para los cuadrantes
        ForkJoinStrategy q1 = new ForkJoinStrategy(matriz, inicioX, midX, inicioY, midY);
        ForkJoinStrategy q2 = new ForkJoinStrategy(matriz, inicioX, midX, midY, finY);
        ForkJoinStrategy q3 = new ForkJoinStrategy(matriz, midX, finX, inicioY, midY);
        ForkJoinStrategy q4 = new ForkJoinStrategy(matriz, midX, finX, midY, finY);

        // Ejecutamos las tareas en paralelo
        q1.fork();
        q2.fork();
        q3.fork();
        Character resultadoQ4 = q4.compute();  // Ejecutamos el cuarto cuadrante en el hilo actual

        // Recogemos los resultados
        Character resultadoQ1 = q1.join();
        Character resultadoQ2 = q2.join();
        Character resultadoQ3 = q3.join();

        // Devolvemos el primer resultado válido (que no sea '0')
        if (resultadoQ1 != '0') return resultadoQ1;
        if (resultadoQ2 != '0') return resultadoQ2;
        if (resultadoQ3 != '0') return resultadoQ3;
        return resultadoQ4;
    }
}
