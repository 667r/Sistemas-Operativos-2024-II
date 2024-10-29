public class ThreadStrategy extends Thread {
    private Matriz matriz;
    private int inicioX, finX, inicioY, finY;
    private char resultado = ' ';

    public ThreadStrategy(Matriz matriz, int inicioX, int finX, int inicioY, int finY) {
        this.matriz = matriz;
        this.inicioX = inicioX;
        this.finX = finX;
        this.inicioY = inicioY;
        this.finY = finY;
    }

    @Override
    public void run() {
        resultado = buscarLetra(inicioX, finX, inicioY, finY);
    }

    private char buscarLetra(int inicioX, int finX, int inicioY, int finY) {
        // Caso base: Si es una celda única
        if (inicioX == finX - 1 && inicioY == finY - 1) {
            return matriz.getElemento(inicioX, inicioY);
        }

        // Dividir en cuadrantes
        int midX = (inicioX + finX) / 2;
        int midY = (inicioY + finY) / 2;

        // Crear hilos para los 4 cuadrantes
        ThreadStrategy q1 = new ThreadStrategy(matriz, inicioX, midX, inicioY, midY);
        ThreadStrategy q2 = new ThreadStrategy(matriz, inicioX, midX, midY, finY);
        ThreadStrategy q3 = new ThreadStrategy(matriz, midX, finX, inicioY, midY);
        ThreadStrategy q4 = new ThreadStrategy(matriz, midX, finX, midY, finY);

        q1.start(); q2.start(); q3.start(); q4.start();

        try {
            q1.join(); q2.join(); q3.join(); q4.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        // Retornar el primer resultado no vacío
        if (q1.getResultado() != '0') return q1.getResultado();
        if (q2.getResultado() != '0') return q2.getResultado();
        if (q3.getResultado() != '0') return q3.getResultado();
        return q4.getResultado();
    }

    public char getResultado() {
        return resultado;
    }
}
