#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid, dip, cpid;

    pid = fork();  // First fork
    if (pid == 0) {
        printf("First Child process\n");
    } else {
        dip = fork();  // Second fork
        if (dip == 0) {
            printf("Second child process\n");
        } else {
            cpid = wait(0);  // Wait for the first terminated child
            printf("Child died: %d\n", cpid);

            cpid = wait(0);  // Wait for the second terminated child
            printf("Child died: %d\n", cpid);

            printf("Parent process\n");
        }
    }
    return 0;
}