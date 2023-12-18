#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        printf("Fork failed!\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Hello from child!\n");
    } else {
        // Parent process
        printf("Hello from parent!\n");
    }

    return 0;
}