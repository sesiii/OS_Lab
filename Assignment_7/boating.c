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

void *boat_thread(void *arg) {
    int id = *((int *)arg); // Boat ID
    free(arg); // Clean up allocated memory

    printf("Boat %d waiting for visitor\n", id);
    P(&boat); // Wait for a visitor to signal readiness
    
    printf("Boat %d signaling visitor to ride\n", id);
    V(&visitor); // Signal visitor that the ride can start
    
    // Use a hardcoded rtime for the ride (e.g., 2 seconds)
    int rtime = 2;
    printf("Boat %d taking visitor for a %d-second ride\n", id, rtime);
    sleep(rtime); // Simulate the ride
    
    printf("Boat %d ride completed\n", id);
    pthread_barrier_wait(&EOS); // Sync with main and visitor for termination
    
    return NULL;
}

void *visitor_thread(void *arg) {
    int id = *((int *)arg); // Visitor ID
    free(arg); // Clean up allocated memory

    // Seed random number generator uniquely for this thread
    srand(time(NULL) ^ (id << 2));
    
    // Generate random vtime (arrival time)
    int vtime = rand() % 5 + 1; // 1 to 5 seconds
    
    printf("Visitor %d arriving, waiting %d seconds\n", id, vtime);
    sleep(vtime); // Simulate arrival delay
    
    printf("Visitor %d signaling boat\n", id);
    V(&boat); // Signal that visitor is ready for a boat
    
    printf("Visitor %d waiting for ride\n", id);
    P(&visitor); // Wait for the boat to signal ride start
    
    printf("Visitor %d riding\n", id);
    // Ride time is handled by boat, so just wait for completion implicitly
    
    printf("Visitor %d finished\n", id);
    pthread_barrier_wait(&EOS); // Sync with main and boat for termination
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <m> <n>\n", argv[0]);
        return 1;
    }

    m = atoi(argv[1]); // Number of boats
    n = atoi(argv[2]); // Number of visitors
    printf("m = %d, n = %d\n", m, n);

    // Test with 1 boat and 1 visitor
    pthread_t bid, vid;

    // Initialize semaphores
    boat.value = 0;
    visitor.value = 0;
    pthread_mutex_init(&boat.mutex, NULL);
    pthread_cond_init(&boat.cond, NULL);
    pthread_mutex_init(&visitor.mutex, NULL);
    pthread_cond_init(&visitor.cond, NULL);
    pthread_mutex_init(&bmtx, NULL);

    // Allocate and initialize arrays
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    for (int i = 0; i < m; i++) {
        BA[i] = 1; // All boats initially available
        BC[i] = 1; // Capacity of 1 visitor per boat
    }

    // Initialize barrier for 3 threads (main + 1 boat + 1 visitor)
    pthread_barrier_init(&EOS, NULL, 3);

    printf("Main thread initialized\n");

    // Create one boat thread
    int *boat_id = malloc(sizeof(int));
    *boat_id = 0; // Boat ID 0
    pthread_create(&bid, NULL, boat_thread, boat_id);

    // Create one visitor thread
    int *visitor_id = malloc(sizeof(int));
    *visitor_id = 0; // Visitor ID 0
    pthread_create(&vid, NULL, visitor_thread, visitor_id);

    pthread_barrier_wait(&EOS); // Wait for boat and visitor to finish
    pthread_join(bid, NULL);
    pthread_join(vid, NULL);

    printf("Main thread terminating\n");

    // Clean up
    pthread_mutex_destroy(&boat.mutex);
    pthread_cond_destroy(&boat.cond);
    pthread_mutex_destroy(&visitor.mutex);
    pthread_cond_destroy(&visitor.cond);
    pthread_mutex_destroy(&bmtx);
    free(BA);
    free(BC);
    free(BT);
    pthread_barrier_destroy(&EOS);

    return 0;
}