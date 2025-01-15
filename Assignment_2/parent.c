#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_CHILDREN 100

pid_t child_pids[MAX_CHILDREN];
int n_children;
int playing_children;
int *child_status; // 0: playing, 1: missed and out
int last_throw_index = -1;
volatile sig_atomic_t signal_received = 0;
volatile sig_atomic_t signal_type = 0;

void signal_handler(int signum) {
    signal_received = 1;
    signal_type = signum;
    signal(signum, signal_handler); 
}

int find_next_player(int current_index) {
    int next = (current_index + 1) % n_children;
    while (child_status[next] == 1 && playing_children > 1) {
        next = (next + 1) % n_children;
    }
    return next;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_children>\n", argv[0]);
        exit(1);
    }

    n_children = atoi(argv[1]);
    if (n_children <= 0 || n_children > MAX_CHILDREN) {
        fprintf(stderr, "Invalid number of children (1-%d)\n", MAX_CHILDREN);
        exit(1);
    }

    child_status = calloc(n_children, sizeof(int));
    playing_children = n_children;

    //signal handlers
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    // Create child processes
    FILE *fp = fopen("childpid.txt", "w");
    fprintf(fp, "%d\n", n_children);

    for (int i = 0; i < n_children; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }
        if (pid == 0) {  // Child process
            char index_str[10];
            sprintf(index_str, "%d", i + 1);
            execl("./child", "child", index_str, NULL);
            perror("Exec failed");
            exit(1);
        }
        child_pids[i] = pid;
        fprintf(fp, "%d\n", pid);
    }
    fclose(fp);

    printf("Parent: %d child processes created\n", n_children);
    printf("Parent: Waiting for child processes to read child database\n");
    sleep(2); 
    printf("\n");

    // Main game loop
    while (playing_children > 1) {
        int current_player = find_next_player(last_throw_index);
        last_throw_index = current_player;

        
        // printf("\nParent: Initiating a new throw (Child %d)\n", current_player + 1);
        kill(child_pids[current_player], SIGUSR2);

        // Wait for response
        signal_received = 0;
        while (!signal_received) {
            pause();
        }

        // Process response
        if (signal_type == SIGUSR2) {  // Miss
            // printf("Parent: Child %d missed the ball\n", current_player + 1);
            child_status[current_player] = 1;
            playing_children--;
        } else if (signal_type == SIGUSR1) {  // Catch
            // printf("Parent: Child %d caught the ball\n", current_player + 1);
        }

        // dummy process for syncing
        pid_t dummy_pid = fork();
        if (dummy_pid == 0) {
            execl("./dummy", "dummy", NULL);
            exit(1);
        }

        // Write dummy PID to file
        fp = fopen("dummycpid.txt", "w");
        fprintf(fp, "%d\n", dummy_pid);
        fclose(fp);

        // signal to first child
        kill(child_pids[0], SIGUSR1);

        // Wait for dummy to finish
        waitpid(dummy_pid, NULL, 0);
    }

    // Find winner and send SIGINT to all children
    int winner = -1;
    for (int i = 0; i < n_children; i++) {
        if (child_status[i] == 0) {
            winner = i;
        }
        kill(child_pids[i], SIGINT);
    }

    printf("\n");
    for (int i = 0; i < n_children; i++) {
        wait(NULL);
    }

    free(child_status);
    return 0;
}