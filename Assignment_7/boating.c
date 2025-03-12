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

void *visitor_thread(void *arg) {
    int id = *((int *)arg); // Visitor ID
    free(arg); // Clean up allocated memory

    // Seed random number generator uniquely for this thread
    srand(time(NULL) ^ (id << 2));
    
    // Generate random vtime (arrival time) and rtime (ride time) in seconds
    int vtime = rand() % 5 + 1; // 1 to 5 seconds
    int rtime = rand() % 3 + 1; // 1 to 3 seconds
    
    printf("Visitor %d arriving, waiting %d seconds\n", id, vtime);
    sleep(vtime); // Simulate arrival delay
    
    printf("Visitor %d signaling boat\n", id);
    V(&boat); // Signal that visitor is ready for a boat
    
    printf("Visitor %d waiting for ride\n", id);
    P(&visitor); // Wait for the boat to signal ride completion
    
    printf("Visitor %d riding for %d seconds\n", id, rtime);
    sleep(rtime); // Simulate the ride
    
    printf("Visitor %d finished\n", id);
    pthread_barrier_wait(&EOS); // Sync with main for termination
    
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

    // Test with 1 visitor for now
    pthread_t vid;

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

    // Initialize barrier for 2 threads (main + 1 visitor)
    pthread_barrier_init(&EOS, NULL, 2);

    printf("Main thread initialized\n");

    // Create one visitor thread
    int *id = malloc(sizeof(int));
    *id = 0; // Visitor ID 0
    pthread_create(&vid, NULL, visitor_thread, id);

    // Simulate boat interaction manually for testing
    P(&boat); // Main waits for visitor to signal boat
    printf("Main simulating boat: Visitor boarded\n");
    sleep(1); // Simulate some boat processing time
    V(&visitor); // Signal visitor that ride is done

    pthread_barrier_wait(&EOS); // Wait for visitor to finish
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