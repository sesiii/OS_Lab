#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

typedef struct {
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} semaphore;

void V(semaphore *s) {
    pthread_mutex_lock(&s->mutex);
    s->value++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->mutex);
}

void P(semaphore *s) {
    pthread_mutex_lock(&s->mutex);
    while (s->value == 0) {
        pthread_cond_wait(&s->cond, &s->mutex);
    }
    s->value--;
    pthread_mutex_unlock(&s->mutex);
}

// Global variables for the simulation
int m, n; // m boats, n visitors
semaphore *boat_sem; // Array of semaphores, one per boat
pthread_mutex_t bmtx; // Mutex for shared arrays
int *BA, *BC, *BT; // Boat availability, capacity, visitor ID assigned
pthread_barrier_t EOS; // End-of-simulation barrier
pthread_barrier_t *BB; // Per-boat barriers
int visitors_served = 0; // Track completed rides

void *boat_thread(void *arg) {
    int boat_id = *((int *)arg);
    free(arg);

    while (visitors_served < n) {
        // Mark boat as available
        pthread_mutex_lock(&bmtx);
        BA[boat_id] = 1;
        BT[boat_id] = -1; // No visitor assigned yet
        pthread_mutex_unlock(&bmtx);

        printf("Boat %d waiting for visitor\n", boat_id);
        P(&boat_sem[boat_id]); // Wait for a specific visitor

        // Get the assigned visitor ID
        pthread_mutex_lock(&bmtx);
        int visitor_id = BT[boat_id];
        if (visitor_id == -1) {
            printf("Boat %d error: No visitor assigned\n", boat_id);
            pthread_mutex_unlock(&bmtx);
            continue;
        }
        printf("Boat %d assigned to visitor %d\n", boat_id, visitor_id);
        pthread_mutex_unlock(&bmtx);

        // Handshake with visitor to start ride
        pthread_barrier_wait(&BB[boat_id]);

        int rtime = 2; // Hardcoded ride time
        printf("Boat %d taking visitor %d for a %d-second ride\n", boat_id, visitor_id, rtime);
        sleep(rtime);

        // Handshake to end ride
        pthread_barrier_wait(&BB[boat_id]);

        printf("Boat %d ride with visitor %d completed\n", boat_id, visitor_id);
        __sync_fetch_and_add(&visitors_served, 1);
    }

    printf("Boat %d finished serving visitors\n", boat_id);
    return NULL;
}

void *visitor_thread(void *arg) {
    int visitor_id = *((int *)arg);
    free(arg);

    srand(time(NULL) ^ (visitor_id << 2));
    int vtime = rand() % 5 + 1;
    printf("Visitor %d arriving, waiting %d seconds\n", visitor_id, vtime);
    sleep(vtime);

    // Find an available boat
    int boat_id = -1;
    while (boat_id == -1) {
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
            usleep(100000); // Sleep 0.1s if no boat available
        }
    }

    printf("Visitor %d signaling boat %d\n", visitor_id, boat_id);
    V(&boat_sem[boat_id]); // Signal the specific boat

    // Handshake with boat to start ride
    pthread_barrier_wait(&BB[boat_id]);

    printf("Visitor %d riding with boat %d\n", visitor_id, boat_id);

    // Handshake to end ride
    pthread_barrier_wait(&BB[boat_id]);

    printf("Visitor %d finished\n", visitor_id);
    pthread_barrier_wait(&EOS); // Sync with main

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <m> <n>\n", argv[0]);
        return 1;
    }

    m = atoi(argv[1]);
    n = atoi(argv[2]);
    printf("m = %d, n = %d\n", m, n);

    pthread_t bid[m], vid[n];

    // Initialize semaphores and arrays
    boat_sem = (semaphore *)malloc(m * sizeof(semaphore));
    for (int i = 0; i < m; i++) {
        boat_sem[i].value = 0;
        pthread_mutex_init(&boat_sem[i].mutex, NULL);
        pthread_cond_init(&boat_sem[i].cond, NULL);
    }
    pthread_mutex_init(&bmtx, NULL);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    BB = (pthread_barrier_t *)malloc(m * sizeof(pthread_barrier_t));
    for (int i = 0; i < m; i++) {
        BA[i] = 1; // All boats initially available
        BC[i] = 1; // Capacity of 1
        BT[i] = -1; // No visitor assigned
        pthread_barrier_init(&BB[i], NULL, 2); // Boat + visitor
    }

    pthread_barrier_init(&EOS, NULL, n + 1); // n visitors + main

    printf("Main thread initialized\n");

    // Create m boats
    for (int i = 0; i < m; i++) {
        int *boat_id = malloc(sizeof(int));
        *boat_id = i;
        pthread_create(&bid[i], NULL, boat_thread, boat_id);
    }

    // Create n visitors
    for (int i = 0; i < n; i++) {
        int *visitor_id = malloc(sizeof(int));
        *visitor_id = i;
        pthread_create(&vid[i], NULL, visitor_thread, visitor_id);
    }

    pthread_barrier_wait(&EOS); // Wait for all visitors
    for (int i = 0; i < m; i++) {
        pthread_join(bid[i], NULL);
    }
    for (int i = 0; i < n; i++) {
        pthread_join(vid[i], NULL);
    }

    printf("Main thread terminating\n");

    // Clean up
    for (int i = 0; i < m; i++) {
        pthread_mutex_destroy(&boat_sem[i].mutex);
        pthread_cond_destroy(&boat_sem[i].cond);
        pthread_barrier_destroy(&BB[i]);
    }
    free(boat_sem);
    pthread_mutex_destroy(&bmtx);
    free(BB);
    free(BA);
    free(BC);
    free(BT);
    pthread_barrier_destroy(&EOS);

    return 0;
}