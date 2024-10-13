#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char *argv[]) {
    int num;
    pid_t nuevo;

    for (num = 0; num<2; num++) {
        nuevo = fork();
        if (nuevo == 0) {
            break;
        }
    
    }
    nuevo = fork();
    nuevo = fork();
    printf("Soy el proceso %d con padre %d\n", getpid(), getppid());
    return 0;
}
