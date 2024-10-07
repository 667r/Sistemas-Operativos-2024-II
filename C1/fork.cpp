#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Código ejecutado por el proceso hijo
        printf("Soy el proceso hijo\n");
    } else {
        // Código ejecutado por el proceso padre
        printf("Soy el proceso padre\n");
    }
    
    return 0;
}
