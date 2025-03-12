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
semaphore boat, visitor; // Semaphores for synchronization
pthread_mutex_t bmtx; // Mutex for shared arrays
int *BA, *BC, *BT; // Arrays for boat availability, capacity, and type
pthread_barrier_t EOS; // End-of-simulation barrier
int visitors_served = 0; // Track completed rides
int current_visitor_id = -1; // Global to track which visitor is signaling
pthread_mutex_t visitor_id_mutex = PTHREAD_MUTEX_INITIALIZER; // Protect current_visitor_id

void *boat_thread(void *arg) {
    int boat_id = *((int *)arg);
    free(arg);

    while (visitors_served < n) { // Loop until all visitors are served
        printf("Boat %d waiting for visitor\n", boat_id);
        P(&boat); // Wait for a visitor to signal readiness
        
        // Get the visitor ID that signaled (locked until ride starts)
        pthread_mutex_lock(&visitor_id_mutex);
        int visitor_id = current_visitor_id;
        
        printf("Boat %d signaling visitor %d to ride\n", boat_id, visitor_id);
        V(&visitor); // Signal visitor that the ride can start
        
        int rtime = 2; // Hardcoded ride time
        printf("Boat %d taking visitor %d for a %d-second ride\n", boat_id, visitor_id, rtime);
        sleep(rtime); // Simulate the ride
        
        printf("Boat %d ride with visitor %d completed\n", boat_id, visitor_id);
        __sync_fetch_and_add(&visitors_served, 1); // Atomically increment served count
        pthread_mutex_unlock(&visitor_id_mutex); // Release lock after ride
    }
    
    printf("Boat %d finished serving all visitors\n", boat_id);
    return NULL;
}

void *visitor_thread(void *arg) {
    int visitor_id = *((int *)arg);
    free(arg);

    srand(time(NULL) ^ (visitor_id << 2));
    int vtime = rand() % 5 + 1;
    printf("Visitor %d arriving, waiting %d seconds\n", visitor_id, vtime);
    sleep(vtime);
    
    // Set the current visitor ID and signal boat atomically
    pthread_mutex_lock(&visitor_id_mutex);
    current_visitor_id = visitor_id;
    printf("Visitor %d signaling boat\n", visitor_id);
    V(&boat); // Signal readiness
    pthread_mutex_unlock(&visitor_id_mutex);
    
    printf("Visitor %d waiting for ride\n", visitor_id);
    P(&visitor); // Wait for boat to signal ride start
    
    printf("Visitor %d riding\n", visitor_id);
    printf("Visitor %d finished\n", visitor_id);
    pthread_barrier_wait(&EOS); // Sync with main for termination
    
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

    pthread_t bid[m], vid[n]; // Arrays for m boats, n visitors

    // Initialize semaphores and arrays
    boat.value = 0;
    visitor.value = 0;
    pthread_mutex_init(&boat.mutex, NULL);
    pthread_cond_init(&boat.cond, NULL);
    pthread_mutex_init(&visitor.mutex, NULL);
    pthread_cond_init(&visitor.cond, NULL);
    pthread_mutex_init(&bmtx, NULL);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    for (int i = 0; i < m; i++) {
        BA[i] = 1;
        BC[i] = 1;
    }

    // Barrier for n visitors + main
    pthread_barrier_init(&EOS, NULL, n + 1);

    printf("Main thread initialized\n");

    // Create 1 boat
    int *boat_id = malloc(sizeof(int));
    *boat_id = 0;
    pthread_create(&bid[0], NULL, boat_thread, boat_id);

    // Create n visitors
    for (int i = 0; i < n; i++) {
        int *visitor_id = malloc(sizeof(int));
        *visitor_id = i;
        pthread_create(&vid[i], NULL, visitor_thread, visitor_id);
    }

    pthread_barrier_wait(&EOS); // Wait for all visitors to finish
    pthread_join(bid[0], NULL);
    for (int i = 0; i < n; i++) {
        pthread_join(vid[i], NULL);
    }

    printf("Main thread terminating\n");

    // Clean up
    pthread_mutex_destroy(&boat.mutex);
    pthread_cond_destroy(&boat.cond);
    pthread_mutex_destroy(&visitor.mutex);
    pthread_cond_destroy(&visitor.cond);
    pthread_mutex_destroy(&bmtx);
    pthread_mutex_destroy(&visitor_id_mutex);
    free(BA);
    free(BC);
    free(BT);
    pthread_barrier_destroy(&EOS);

    return 0;
}