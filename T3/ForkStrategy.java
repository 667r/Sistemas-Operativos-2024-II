import java.util.concurrent.ForkJoinPool;

public class ForkStrategy {
    private static final ForkJoinPool pool = new ForkJoinPool();

    public static char buscarConFork(Matriz matriz) {
        ForkJoinStrategy task = new ForkJoinStrategy(matriz, 0, matriz.getFilas(), 0, matriz.getColumnas());
        return pool.invoke(task);  // Ejecutar la búsqueda en el ForkJoinPool
    }
}
