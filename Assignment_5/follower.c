#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define PATH "/"
#define ID 120000
#define MAX_FOLLOWERS 100

void follower_process() {
    // Get shared memory
    key_t key = ftok(PATH, ID);
    if (key == -1) {
        perror("ftok error");
        exit(1);
    }

    // Get shared memory ID
    int shmid = shmget(key, 0, 0666);
    if (shmid == -1) {
        perror("shmget error (leader not running?)");
        exit(1);
    }

    int *M = (int *)shmat(shmid, NULL, 0);
    if (M == (int *)-1) {
        perror("shmat error");
        exit(1);
    }

    // Atomic increment of M[1] and get my number
    int my_number;
    while (1) {
        int current = M[1];
        if (current >= M[0]) {
            printf("follower error: %d followers have already joined\n", M[0]);
            shmdt(M);
            exit(1);
        }
        
        my_number = current + 1;
        M[1] = my_number;
        if (current == M[1] - 1) {
            break;
        }
        usleep(10000);  
    }

    printf("follower %d joins\n", my_number);

    srand(time(NULL) ^ (getpid() << 16));

    while (1) {
        // Wait for my turn
        while (M[2] != my_number && M[2] != -my_number) {
            usleep(1000);
        }

        if (M[2] == -my_number) {
            printf("follower %d leaves\n", my_number);
            
            // Signal next process
            if (my_number == M[0]) {
                M[2] = 0;  // Last follower signals leader
            } else {
                M[2] = -(my_number + 1);  // Signal next follower
            }
            
            shmdt(M);
            exit(0);
        }

        M[3 + my_number] = rand() % 9 + 1;

        // Signal next process
        if (my_number == M[0]) {
            M[2] = 0;  // Last follower signals leader
        } else {
            M[2] = my_number + 1;  // Signal next follower
        }
    }
}

int main(int argc, char *argv[]) {
    int nf = 1;  // Default number of followers to create
    if (argc > 1) {
        nf = atoi(argv[1]);
        if (nf <= 0) {
            printf("Number of followers must be positive\n");
            return 1;
        }
    }

    // Fork nf children
    for (int i = 0; i < nf; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork error");
            return 1;
        }
        if (pid == 0) {
            // Child process
            follower_process();
            exit(0); 
        }
        usleep(10000);  
    }

    // Wait for all children to finish
    for (int i = 0; i < nf; i++) {
        wait(NULL);
    }

    return 0;
}