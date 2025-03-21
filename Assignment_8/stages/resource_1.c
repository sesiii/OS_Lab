#include <stdio.h>
#include <stdlib.h>

// Structure for a request in the queue
struct Request {
    int thread_id;          // Which thread made this request (0 to n-1)
    int* req;               // Array of m values (e.g., 1 0 -1)
    struct Request* next;   // Pointer to the next request in Q
};

// Variables (will be in main or a master thread function)
int m, n;                   // Number of resource types and threads
int* AVAILABLE;             // Array of available resources
int** NEED;                 // 2D array of max needs remaining
int** ALLOC;                // 2D array of allocated resources
struct Request* Q_front;    // Front of the request queue
struct Request* Q_back;     // Back of the request queue (for fast enqueue)

int main() {
    // Example initialization (we’ll fill this in Milestone 3)
    m = 5;  // Sample value; will come from system.txt
    n = 10; // Sample value

    // Allocate AVAILABLE
    AVAILABLE = (int*)malloc(m * sizeof(int));
    if (!AVAILABLE) { printf("Memory error\n"); return 1; }

    // Allocate NEED and ALLOC as 2D arrays
    NEED = (int**)malloc(n * sizeof(int*));
    ALLOC = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        NEED[i] = (int*)malloc(m * sizeof(int));
        ALLOC[i] = (int*)malloc(m * sizeof(int));
        if (!NEED[i] || !ALLOC[i]) { printf("Memory error\n"); return 1; }
    }

    // Initialize queue as empty
    Q_front = NULL;
    Q_back = NULL;

    // Placeholder: We’ll populate these in Milestone 3 from files
    printf("Structures set up: m=%d, n=%d\n", m, n);

    // Free memory (cleanup for now)
    free(AVAILABLE);
    for (int i = 0; i < n; i++) {
        free(NEED[i]);
        free(ALLOC[i]);
    }
    free(NEED);
    free(ALLOC);

    return 0;
}