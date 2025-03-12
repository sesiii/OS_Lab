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
pthread_cond_t *boat_exit_cond; // Condition variables to signal boats to exit
pthread_mutex_t *boat_exit_mutex; // Mutexes for boat exit conditions
int *BA, *BC, *BT; // Boat availability, capacity, visitor ID assigned
pthread_barrier_t EOS; // End-of-simulation barrier for visitors + main
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
        printf("[Time %ld] Visitor %d assigned to Boat %d\n", time(NULL), visitor_id, boat_id);

        int rtime = rand() % 5 + 1;
        printf("[Time %ld] Boat    %d start of ride for visitor %d\n", time(NULL), boat_id, visitor_id);
        sleep(rtime ); // Convert minutes to seconds (assuming shorter simulation time)

        // Signal ride completion
        printf("[Time %ld] Boat    %d end of ride for visitor %d (ride time = %d)\n", time(NULL), boat_id, visitor_id, rtime);
        pthread_barrier_wait(&BB[boat_id]);

        __sync_fetch_and_add(&visitors_served, 1);
        n = total_visitors - visitors_served;
        printf("[Time %ld] Boat    %d served visitor %d, %d visitors remaining\n", time(NULL), boat_id, visitor_id, n);
        if (n == 0) {
            printf("All visitors served\n");
            break;
        }
    }

    // Wait for simulation end signal if not already done
    pthread_mutex_lock(&boat_exit_mutex[boat_id]);
    while (!simulation_done) {
        pthread_cond_wait(&boat_exit_cond[boat_id], &boat_exit_mutex[boat_id]);
    }
    pthread_mutex_unlock(&boat_exit_mutex[boat_id]);

    printf("[Time %ld] Boat    %d finished serving visitors\n", time(NULL), boat_id);
    return NULL;
}

void *visitor_thread(void *arg) {
    int visitor_id = *((int *)arg);
    free(arg);

    srand(time(NULL) ^ (visitor_id << 2));
    int vtime = rand() % 5 + 1;
    printf("[Time %ld] Visitor %d Starts sightseeing for %d minutes\n", time(NULL), visitor_id, vtime);
    sleep(vtime); // Convert minutes to seconds

    // Find an available boat
    int boat_id = -1;
    printf("[Time %ld] Visitor %d Ready to ride a boat\n", time(NULL), visitor_id);
    pthread_mutex_lock(&bmtx);
    for (int i = 0; i < m; i++) {
        if (BA[i] == 1) {
            BA[i] = 0; // Mark boat as busy
            BT[i] = visitor_id; // Assign visitor to boat
            boat_id = i;
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
    total_visitors = n = atoi(argv[2]);
    printf("m = %d, n = %d\n", m, n);

    pthread_t bid[m], vid[n];

    pthread_mutex_init(&bmtx, NULL);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    BB = (pthread_barrier_t *)malloc(m * sizeof(pthread_barrier_t));
    boat_exit_cond = (pthread_cond_t *)malloc(m * sizeof(pthread_cond_t));
    boat_exit_mutex = (pthread_mutex_t *)malloc(m * sizeof(pthread_mutex_t));

    for (int i = 0; i < m; i++) {
        BA[i] = 1;
        BC[i] = 1;
        BT[i] = -1;
        pthread_barrier_init(&BB[i], NULL, 2);
        pthread_cond_init(&boat_exit_cond[i], NULL);
        pthread_mutex_init(&boat_exit_mutex[i], NULL);
    }

    pthread_barrier_init(&EOS, NULL, n + 1);

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
    printf("i am here now...\n");
    // Signal all boats to exit
    for (int i = 0; i < m; i++) {
        pthread_mutex_lock(&boat_exit_mutex[i]);
        pthread_cond_signal(&boat_exit_cond[i]);
        pthread_mutex_unlock(&boat_exit_mutex[i]);
    }
    printf("aaaa...\n");
    for (int i = 0; i < n; i++) {
        pthread_join(vid[i], NULL);
    }
    printf("bbbb...\n");
    for (int i = 0; i < m; i++) {
        printf("joining boat %d\n", i);
        pthread_join(bid[i], NULL);
    }

    printf("[Time %ld] Main thread terminating\n", time(NULL));

    pthread_mutex_destroy(&bmtx);
    for (int i = 0; i < m; i++) {
        pthread_barrier_destroy(&BB[i]);
        pthread_cond_destroy(&boat_exit_cond[i]);
        pthread_mutex_destroy(&boat_exit_mutex[i]);
    }
    free(BB);
    free(boat_exit_cond);
    free(boat_exit_mutex);
    free(BA);
    free(BC);
    free(BT);
    pthread_barrier_destroy(&EOS);

    return 0;
}