#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define MAX_CHILDREN 100

int my_index;
int n_children;
pid_t child_pids[MAX_CHILDREN];
int my_status = 0;  // 0: PLAYING, 1: CATCHMADE, 2: CATCHMISSED, 3: OUTOFGAME
int *out_children;  // Array to track which children are out
static int first_print = 1;  

void print_status() {
    
    if (my_index == 1 && first_print) {
        // Print child numbers
        for (int i = 1; i <= n_children; i++) {
            printf("    %d  |", i);
        }
        printf("\n ");
        
        printf(" \n   ");
        for (int i = 0; i < n_children; i++) {
            printf("      ");
        }
        printf(" \n");
    
        first_print = 0;
    }

    
    if (my_index == 1) {
        printf("|");
    }

    // Print status
    if (out_children[my_index - 1]) {
        printf("       |");  // Blank for out children
    } else if (my_status == 2) {  // Just missed
        printf(" MISS  |");
        out_children[my_index - 1] = 1;  // Mark as out
    } else if (my_status == 1) {  // Caught
        printf(" CATCH |");
    } else {  // Still playing
        printf(" ....  |");
    }

    if (my_index == n_children) {
        printf(" \n   ");
        for (int i = 0; i < n_children; i++) {
            printf("      ");
        }
        printf(" \n");
    }

    fflush(stdout);
}

void signal_handler(int signum) {
    if (signum == SIGINT) {
        if (my_status != 3) {
            printf("+++ Child %d: Yay! I am the winner!\n", my_index);
        }
        free(out_children);
        exit(0);
    }

    if (signum == SIGUSR1) {  // Print status
        print_status();
        
        if (my_status == 2) {  // Just missed
            my_status = 3;  // Set to out of game
        }
        
        if (my_status == 1) {  // Reset catch status
            my_status = 0;
        }

        if (my_index < n_children) {
            kill(child_pids[my_index], SIGUSR1);
        } else {
            FILE *fp = fopen("dummycpid.txt", "r");
            pid_t dummy_pid;
            fscanf(fp, "%d", &dummy_pid);
            fclose(fp);
            kill(dummy_pid, SIGINT);
        }
    }

    if (signum == SIGUSR2) {  // Ball throw
        double catch_prob = ((double)rand() / RAND_MAX);
        if (catch_prob <= 0.8) {  // 80% chance to catch
            my_status = 1;  // catched
            kill(getppid(), SIGUSR1);
        } else {
            my_status = 2;  // missed catch
            kill(getppid(), SIGUSR2);
        }
    }
    
    signal(signum, signal_handler);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <index>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL) ^ (getpid()<<16));
    my_index = atoi(argv[1]);

    // Wait for parent to write child database
    sleep(1);

    // Read child database
    FILE *fp = fopen("childpid.txt", "r");
    fscanf(fp, "%d", &n_children);
    
    // shared out_children array
    out_children = calloc(n_children, sizeof(int));
    
    for (int i = 0; i < n_children; i++) {
        fscanf(fp, "%d", &child_pids[i]);
    }
    fclose(fp);

    //signal handlers
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGINT, signal_handler);

    while (1) {
        pause();
    }

    return 0;
}