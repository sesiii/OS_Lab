#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

// Global variables for the simulation
int m, n; // m boats, n visitors remaining
pthread_mutex_t bmtx; // Mutex for shared arrays
int *BA, *BC, *BT; // Boat availability, capacity, visitor ID assigned
pthread_barrier_t EOS; // End-of-simulation barrier
pthread_barrier_t *BB; // Per-boat barriers
int total_visitors; // Initial number of visitors
int visitors_served = 0; // Track completed rides
int simulation_done = 0; // Flag to signal boats to exit

void *boat_thread(void *arg) {
    int boat_id = *((int *)arg);
    free(arg);

    while (visitors_served < total_visitors && !simulation_done) {
        // Mark boat as available
        pthread_mutex_lock(&bmtx);
        BA[boat_id] = 1;
        BT[boat_id] = -1;
        printf("[Time %ld] Boat    %d ready\n", time(NULL), boat_id);
        pthread_mutex_unlock(&bmtx);

        // Wait for a visitor to pair and start the ride
        pthread_barrier_wait(&BB[boat_id]);

        // Check assigned visitor
        pthread_mutex_lock(&bmtx);
        int visitor_id = BT[boat_id];
        if (visitor_id == -1) {
            printf("[Time %ld] Boat    %d error: No visitor assigned\n", time(NULL), boat_id);
            pthread_mutex_unlock(&bmtx);
            continue; // Shouldn’t happen with proper pairing
        }
        pthread_mutex_unlock(&bmtx);
        printf("[Time %ld] Visitor %d assigned to Boat %d\n", time(NULL), visitor_id,boat_id );
        
        int rtime = rand() % 50 + 1;
        printf("[Time %ld] Boat    %d start of ride for visitor %d\n", time(NULL), boat_id, visitor_id);
        sleep(rtime);

        // Signal ride completion
        printf("[Time %ld] Boat    %d end of ride for visitor %d (ride time = %d)\n", time(NULL), boat_id, visitor_id, rtime);
        pthread_barrier_wait(&BB[boat_id]);

        __sync_fetch_and_add(&visitors_served, 1);
        n = total_visitors - visitors_served; // Update remaining visitors
        printf("[Time %ld] Boat    %d served visitor %d, %d visitors remaining\n", time(NULL), boat_id, visitor_id, n);
        if(n == 0) {
            printf("All visitors served\n");
            break;
        }
    }
    // Boat 5 End of ride for visitor 2 (
    pthread_barrier_wait(&EOS);
    // printf("[Time %ld] Boat       %d finished serving visitors\n", time(NULL), boat_id);
    return NULL;
}

void *visitor_thread(void *arg) {
    int visitor_id = *((int *)arg);
    free(arg);

    srand(time(NULL) ^ (visitor_id << 2));
    int vtime = rand() % 50 + 1;
    printf("[Time %ld] Visitor %d Starts sightseeing for %d minutes\n", time(NULL), visitor_id, vtime);
    sleep(vtime);

    // Find an available boat
    int boat_id = -1;
    printf("[Time %ld] Visitor %d Ready to ride a boat\n", time(NULL), visitor_id);
    pthread_mutex_lock(&bmtx);
    for (int i = 0; i < m; i++) {
        if (BA[i] == 1) {
            BA[i] = 0; // Mark boat as busy
            BT[i] = visitor_id; // Assign visitor to boat
            boat_id = i;
            // printf("[Time %ld] Visitor %d signaling Boat       %d\n", time(NULL), visitor_id, boat_id);
            break;
        }
    }
    pthread_mutex_unlock(&bmtx);

    if (boat_id == -1) {
        printf("[Time %ld] Visitor %d found no available boat\n", time(NULL), visitor_id);
        pthread_barrier_wait(&EOS);
        return NULL;
    }

    // Pair with boat and start ride
    pthread_barrier_wait(&BB[boat_id]);
    printf("[Time %ld] Visitor %d Finds Boat %d\n", time(NULL), visitor_id, boat_id);

    // Complete ride
    pthread_barrier_wait(&BB[boat_id]);
    printf("[Time %ld] Visitor %d leaving\n", time(NULL), visitor_id);

    pthread_barrier_wait(&EOS);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <m> <n>\n", argv[0]);
        return 1;
    }

    m = atoi(argv[1]);
    total_visitors = n = atoi(argv[2]); // n is remaining, total_visitors is initial
    printf("m = %d, n = %d\n", m, n);

    pthread_t bid[m], vid[n];

    pthread_mutex_init(&bmtx, NULL);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    BB = (pthread_barrier_t *)malloc(m * sizeof(pthread_barrier_t));

    for (int i = 0; i < m; i++) {
        BA[i] = 1;
        BC[i] = 1;
        BT[i] = -1;
        pthread_barrier_init(&BB[i], NULL, 2); // Boat + visitor, no reinitialization
    }

    pthread_barrier_init(&EOS, NULL, n + 1); // n visitors + main

    printf("[Time %ld] Main thread initialized\n", time(NULL));

    for (int i = 0; i < m; i++) {
        int *boat_id = malloc(sizeof(int));
        *boat_id = i;
        pthread_create(&bid[i], NULL, boat_thread, boat_id);
    }

    for (int i = 0; i < n; i++) {
        int *visitor_id = malloc(sizeof(int));
        *visitor_id = i;
        pthread_create(&vid[i], NULL, visitor_thread, visitor_id);
    }

    pthread_barrier_wait(&EOS);
    simulation_done = 1;
    for (int i = 0; i < m; i++) {
        pthread_join(bid[i], NULL);
    }
    for (int i = 0; i < n; i++) {
        pthread_join(vid[i], NULL);
    }

    

    printf("[Time %ld] Main thread terminating\n", time(NULL));

    pthread_mutex_destroy(&bmtx);
    for (int i = 0; i < m; i++) {
        pthread_barrier_destroy(&BB[i]);
    }
    free(BB);
    free(BA);
    free(BC);
    free(BT);
    pthread_barrier_destroy(&EOS);

    return 0;
}