public class ThreadStrategy extends Thread {
    private Matriz matriz;
    private int inicioX, finX, inicioY, finY;
    private char resultado;

    public ThreadStrategy(Matriz matriz, int inicioX, int finX, int inicioY, int finY) {
        this.matriz = matriz;
        this.inicioX = inicioX;
        this.finX = finX;
        this.inicioY = inicioY;
        this.finY = finY;
    }

    @Override
    public void run() {
        buscarLetra();
    }

    private void buscarLetra() {
        for (int i = inicioX; i < finX; i++) {
            for (int j = inicioY; j < finY; j++) {
                if (matriz.getElemento(i, j) != '0') {
                    resultado = matriz.getElemento(i, j);
                    return;
                }
            }
        }
    }

    public char getResultado() {
        return resultado;
    }
}
