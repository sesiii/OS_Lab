#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define MAX_FOLLOWERS 100
#define MAX_SUMS 1000
#define PATH "/"
#define ID 120000

int main(int argc, char *argv[])
{
    int n = 10;
    if (argc > 1)
    {
        n = atoi(argv[1]);
        if (n <= 0 || n > MAX_FOLLOWERS)
        {
            printf("Number of followers must be between 1 and %d\n", MAX_FOLLOWERS);
            return 1;
        }
    }

    // Create shared memory
    key_t key = ftok(PATH, ID);
    if (key == -1)
    {
        perror("ftok error");
        return 1;
    }

    // Size calculation: M[0] to M[3+n] = (n+4) integers
    size_t shm_size = (n + 4) * sizeof(int);
    int shmid = shmget(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1)
    {
        perror("shmget error (leader might already be running)");
        return 1;
    }
    

    printf("\n");
    // Attach shared memory
    int *M = (int *)shmat(shmid, NULL, 0);
    if (M == (int *)-1)
    {
        perror("shmat error");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    // Initialize shared memory
    memset(M, 0, shm_size); // Clear all memory first
    M[0] = n;               // Number of followers
    M[1] = 0;               // Number of joined followers
    M[2] = 0;               // Turn leader)
    // printf("Array M:\n");
    // {
    //     for (int i = 0; i < n + 4; i++)
    //     {
    //         printf("%d ", M[i]);
    //     }
    // }
    printf("\n");
    printf("Leader started. Waiting for %d followers to join...\n", n);

    // Waits for all followers to join
    while (M[1] < n)
    {
        printf("\rWaiting for all followers to join");
        fflush(stdout);
        usleep(1000000);
    }
    printf("\nAll followers have joined!\n\n");

    // Initialize random number generator
    srand(time(NULL));

    // Array to store previous sums
    int *sums = calloc(MAX_SUMS, sizeof(int));
    int sum_count = 0;
    bool sum_repeated = false;

    while (!sum_repeated)
    {
        // Wait for my turn (0)
        while (M[2] != 0)
        {
            usleep(10000);
        }

        // rand numer to M[3]
        M[3] = rand() % 99 + 1;

        // Give turn to first follower
        M[2] = 1;

        // Waits for the last follower to complete
        while (M[2] != 0)
        {
            usleep(1000);
        }

        int sum = 0;
        for (int i = 3; i < n + 4; i++)
        {
            sum += M[i];
        }

        //sum calculation
        printf("%d", M[3]);
        for (int i = 4; i < n + 4; i++)
        {
            printf(" + %d", M[i]);
        }
        printf(" = %d\n", sum);

        // Check if sum exists in previous sums
        for (int i = 0; i < sum_count; i++)
        {
            if (sums[i] == sum)
            {
                sum_repeated = true;
                break;
            }
        }

        if (!sum_repeated && sum_count < MAX_SUMS)
        {
            // Store sum in array
            sums[sum_count++] = sum;
            M[2] = 1; 
        }
        else
        {
            M[2] = -1; // Signal termination
        }
    }
    // printf("Updated Array M:\n");
    // {
    //     for (int i = 0; i < n + 4; i++)
    //     {
    //         printf("%d ", M[i]);
    //     }
    // }

    // Wait for the last follower to signal completion
    while (M[2] != 0)
    {
        usleep(10000);
    }

    
    free(sums);
    shmdt(M);
    shmctl(shmid, IPC_RMID, NULL);

    printf("Leader terminated.\n");
    return 0;
}