#include<stdio.h>
#include<unistd.h>

int main(){
pid_t pid = fork();

if (pid < 0) {
    // Fork failed (only in the parent process)
} else if (pid == 0) {
    // This block runs ONLY in the child process
    printf("This is the child process! PID: %d,Child PID: %d", getpid(),pid);
} else {
    // This block runs ONLY in the parent process
    printf("This is the parent process! PID: %d, Child PID: %d\n", getpid(), pid);
}
}