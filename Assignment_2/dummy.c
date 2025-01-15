#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handle_sigint(int sig) {
    // printf("Dummy: Received SIGINT, exiting\n");
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);
    while (1) {
        pause();
    }
    return 0;
}