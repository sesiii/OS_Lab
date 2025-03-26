// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>

// // // Constants from the assignment
// // #define TOTAL_FRAMES 16384      // 64 MB / 4 KB
// // #define OS_RESERVED 4096        // 16 MB / 4 KB
// // #define USER_FRAMES 12288       // 48 MB / 4 KB
// // #define ESSENTIAL_PAGES 10      // Essential pages per process
// // #define PAGE_SIZE 4096          // 4 KB pages
// // #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// // #define MAX_PROCESSES 500       // Max n
// // #define MAX_SEARCHES 100        // Max m

// // // Process structure
// // typedef struct {
// //     int pid;                    // Process ID
// //     int s;                      // Size of array A (in integers)
// //     int searches[MAX_SEARCHES]; // Search indices (k0, k1, ...)
// //     int search_count;           // Number of searches completed
// //     unsigned short page_table[PAGE_TABLE_SIZE]; // Page table (16-bit entries)
// //     int essential_frames[ESSENTIAL_PAGES];      // Frames for essential pages
// //     int active;                 // 1 if active, 0 if swapped out
// // } Process;

// // // Queue structure for FIFO (ready queue, free frames, swapped-out processes)
// // typedef struct {
// //     int *data;
// //     int front, rear, size, capacity;
// // } Queue;

// // Queue* create_queue(int capacity) {
// //     Queue *q = (Queue*)malloc(sizeof(Queue));
// //     q->data = (int*)malloc(capacity * sizeof(int));
// //     q->front = q->rear = -1;
// //     q->size = 0;
// //     q->capacity = capacity;
// //     return q;
// // }

// // void enqueue(Queue *q, int item) {
// //     if (q->size == q->capacity) return;
// //     if (q->front == -1) q->front = 0;
// //     q->rear = (q->rear + 1) % q->capacity;
// //     q->data[q->rear] = item;
// //     q->size++;
// // }

// // int dequeue(Queue *q) {
// //     if (q->size == 0) return -1;
// //     int item = q->data[q->front];
// //     q->front = (q->front + 1) % q->capacity;
// //     q->size--;
// //     if (q->size == 0) q->front = q->rear = -1;
// //     return item;
// // }

// // void free_queue(Queue *q) {
// //     free(q->data);
// //     free(q);
// // }

// // // Global variables
// // Process processes[MAX_PROCESSES];
// // int n, m;                   // Number of processes and searches
// // Queue *ready_queue, *free_frames, *swapped_out;
// // long page_accesses = 0, page_faults = 0, swaps = 0;
// // int active_processes = 0;
// // int min_active_when_full = MAX_PROCESSES;

// // // Helper to set valid bit and frame number in page table entry
// // unsigned short make_pte(int frame, int valid) {
// //     return (valid << 15) | (frame & 0x7FFF); // MSB is valid bit, 15 bits for frame
// // }

// // // Simulate binary search and handle page faults
// // int simulate_binary_search(Process *p, int search_idx) {
// //     int k = p->searches[search_idx]; // Target index
// //     int L = 0, R = p->s - 1;
// //     while (L < R) {
// //         int M = (L + R) / 2;
// //         page_accesses++;
// //         int page = M * sizeof(int) / PAGE_SIZE; // Page containing A[M]
// //         unsigned short pte = p->page_table[page];
// //         int valid = (pte >> 15) & 1;
// //         int frame = pte & 0x7FFF;

// //         if (!valid) { // Page fault
// //             page_faults++;
// //             if (free_frames->size == 0) { // Memory full, swap out
// //                 swaps++;
// //                 printf("+++ Swapping out process %3d  [%d active processes]\n", p->pid, active_processes - 1);
// //                 p->active = 0;
// //                 active_processes--;
// //                 for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// //                     enqueue(free_frames, p->essential_frames[i]);
// //                 }
// //                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// //                     if (p->page_table[i] >> 15) { // If valid
// //                         enqueue(free_frames, p->page_table[i] & 0x7FFF);
// //                         p->page_table[i] = 0; // Invalidate
// //                     }
// //                 }
// //                 enqueue(swapped_out, p->pid);
// //                 return 0; // Search interrupted
// //             }
// //             frame = dequeue(free_frames);
// //             p->page_table[page] = make_pte(frame, 1);
// //         }
// //         if (k <= M) R = M;
// //         else L = M + 1;
// //     }
// //     return 1; // Search completed
// // }

// // // Initialize process and load essential pages
// // void init_process(int pid) {
// //     processes[pid].pid = pid;
// //     processes[pid].active = 1;
// //     processes[pid].search_count = 0;
// //     memset(processes[pid].page_table, 0, sizeof(processes[pid].page_table));
// //     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// //         processes[pid].essential_frames[i] = dequeue(free_frames);
// //     }
// //     active_processes++;
// //     enqueue(ready_queue, pid);
// // }

// // // Main simulation
// // void run_simulation() {
// //     FILE *fp = fopen("search.txt", "r");
// //     if (!fp) {
// //         perror("Failed to open search.txt");
// //         exit(1);
// //     }
// //     fscanf(fp, "%d %d", &n, &m);
// //     for (int i = 0; i < n; i++) {
// //         fscanf(fp, "%d", &processes[i].s);
// //         for (int j = 0; j < m; j++) {
// //             fscanf(fp, "%d", &processes[i].searches[j]);
// //         }
// //     }
// //     fclose(fp);
// //     printf("+++ Simulation data read from file\n");

// //     // Initialize kernel data
// //     ready_queue = create_queue(n);
// //     free_frames = create_queue(TOTAL_FRAMES);
// //     swapped_out = create_queue(n);
// //     for (int i = OS_RESERVED; i < TOTAL_FRAMES; i++) {
// //         enqueue(free_frames, i);
// //     }
// //     for (int i = 0; i < n; i++) {
// //         init_process(i);
// //     }
// //     printf("+++ Kernel data initialized\n");

// //     // Run simulation
// //     while (ready_queue->size > 0 || swapped_out->size > 0) {
// //         int pid = dequeue(ready_queue);
// //         if (pid == -1) break; // Shouldn't happen with proper logic

// //         Process *p = &processes[pid];
// //         if (!p->active) continue; // Skip if swapped out

// //         int search_idx = p->search_count;
// //         if (search_idx >= m) continue; // Process done

// // #ifdef VERBOSE
// //         printf("\tSearch %d by Process %d\n", search_idx + 1, pid);
// // #endif

// //         if (simulate_binary_search(p, search_idx)) {
// //             p->search_count++;
// //             if (p->search_count < m) {
// //                 enqueue(ready_queue, pid); // Back to ready queue
// //             } else { // Process terminates
// //                 active_processes--;
// //                 for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// //                     enqueue(free_frames, p->essential_frames[i]);
// //                 }
// //                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// //                     if (p->page_table[i] >> 15) {
// //                         enqueue(free_frames, p->page_table[i] & 0x7FFF);
// //                     }
// //                 }
// //                 // Swap in a process if possible
// //                 if (swapped_out->size > 0 && free_frames->size >= ESSENTIAL_PAGES) {
// //                     int swap_pid = dequeue(swapped_out);
// //                     init_process(swap_pid);
// //                     printf("+++ Swapping in process %3d  [%d active processes]\n", swap_pid, active_processes);
// //                 }
// //             }
// //         }

// //         // Update degree of multiprogramming
// //         if (free_frames->size == 0 && active_processes < min_active_when_full) {
// //             min_active_when_full = active_processes;
// //         }
// //     }

// //     // Final statistics
// //     printf("+++ Page access summary\n");
// //     printf("\tTotal number of page accesses  =  %ld\n", page_accesses);
// //     printf("\tTotal number of page faults    =  %ld\n", page_faults);
// //     printf("\tTotal number of swaps          =  %ld\n", swaps);
// //     printf("\tDegree of multiprogramming     =  %d\n", min_active_when_full);

// //     free_queue(ready_queue);
// //     free_queue(free_frames);
// //     free_queue(swapped_out);
// // }

// // int main() {
// //     run_simulation();
// //     return 0;
// // }


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// // Constants from the assignment
// #define TOTAL_FRAMES 16384      // 64 MB / 4 KB
// #define OS_RESERVED 4096        // 16 MB / 4 KB
// #define USER_FRAMES 12288       // 48 MB / 4 KB
// #define ESSENTIAL_PAGES 10      // Essential pages per process
// #define PAGE_SIZE 4096          // 4 KB pages
// #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// #define MAX_PROCESSES 500       // Max n
// #define MAX_SEARCHES 100        // Max m
// #define INTS_PER_PAGE 1024      // 4096 / 4 = 1024 ints per page

// // Process structure
// typedef struct {
//     int pid;                    // Process ID
//     int s;                      // Size of array A (in integers)
//     int searches[MAX_SEARCHES]; // Search indices (k0, k1, ...)
//     int search_count;           // Number of searches completed
//     unsigned short page_table[PAGE_TABLE_SIZE]; // Page table (16-bit entries)
//     int essential_frames[ESSENTIAL_PAGES];      // Frames for essential pages
//     int active;                 // 1 if active, 0 if swapped out
//     int L, R;                   // Binary search state for resumption
//     int in_progress;            // 1 if search was interrupted
// } Process;

// // Queue structure for FIFO
// typedef struct {
//     int *data;
//     int front, rear, size, capacity;
// } Queue;

// Queue* create_queue(int capacity) {
//     Queue *q = (Queue*)malloc(sizeof(Queue));
//     q->data = (int*)malloc(capacity * sizeof(int));
//     q->front = q->rear = -1;
//     q->size = 0;
//     q->capacity = capacity;
//     return q;
// }

// void enqueue(Queue *q, int item) {
//     if (q->size == q->capacity) return;
//     if (q->front == -1) q->front = 0;
//     q->rear = (q->rear + 1) % q->capacity;
//     q->data[q->rear] = item;
//     q->size++;
// }

// int dequeue(Queue *q) {
//     if (q->size == 0) return -1;
//     int item = q->data[q->front];
//     q->front = (q->front + 1) % q->capacity;
//     q->size--;
//     if (q->size == 0) q->front = q->rear = -1;
//     return item;
// }

// void free_queue(Queue *q) {
//     free(q->data);
//     free(q);
// }

// // Global variables
// Process processes[MAX_PROCESSES];
// int n, m;                   // Number of processes and searches
// Queue *ready_queue, *free_frames, *swapped_out;
// long page_accesses = 0, page_faults = 0, swaps = 0;
// int active_processes = 0;
// int min_active_when_full;

// // Helper to set valid bit and frame number in page table entry
// unsigned short make_pte(int frame, int valid) {
//     return (valid << 15) | (frame & 0x7FFF); // MSB is valid bit, 15 bits for frame
// }

// // Simulate binary search with state preservation
// int simulate_binary_search(Process *p, int search_idx) {
//     int k = p->searches[search_idx];
//     int L = p->in_progress ? p->L : 0;
//     int R = p->in_progress ? p->R : p->s - 1;
//     p->in_progress = 0; // Reset unless swapped out again

//     while (L < R) {
//         int M = (L + R) / 2;
//         int page = M / INTS_PER_PAGE;
//         unsigned short pte = p->page_table[page];
//         int valid = (pte >> 15) & 1;

//         // Only count access/fault if this is a new iteration
//         if (!p->in_progress || (p->in_progress && M != p->L && M != p->R)) {
//             page_accesses++;
//             if (!valid) page_faults++;
//         }

//         if (!valid) { // Page fault
//             if (free_frames->size == 0) { // Memory full, swap out
//                 swaps++;
//                 printf("+++ Swapping out process %3d  [%d active processes]\n", p->pid, active_processes - 1);
//                 p->active = 0;
//                 p->in_progress = 1;
//                 p->L = L;
//                 p->R = R;
//                 active_processes--;
//                 for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//                     enqueue(free_frames, p->essential_frames[i]);
//                 }
//                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                     if (p->page_table[i] >> 15) {
//                         enqueue(free_frames, p->page_table[i] & 0x7FFF);
//                         p->page_table[i] = 0; // Invalidate
//                     }
//                 }
//                 enqueue(swapped_out, p->pid);
//                 if (active_processes < min_active_when_full) {
//                     min_active_when_full = active_processes;
//                 }
//                 return 0; // Search interrupted
//             }
//             int frame = dequeue(free_frames);
//             p->page_table[page] = make_pte(frame, 1);
//         }
//         if (k <= M) R = M;
//         else L = M + 1;
//     }
//     return 1; // Search completed
// }

// // Initialize process and load essential pages
// void init_process(int pid) {
//     Process *p = &processes[pid];
//     p->pid = pid;
//     p->active = 1;
//     p->search_count = 0;
//     p->in_progress = 0;
//     memset(p->page_table, 0, sizeof(p->page_table));
//     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//         p->essential_frames[i] = dequeue(free_frames);
//     }
//     active_processes++;
//     enqueue(ready_queue, pid);
// }

// // Main simulation
// void run_simulation() {
//     FILE *fp = fopen("search.txt", "r");
//     if (!fp) {
//         perror("Failed to open search.txt");
//         exit(1);
//     }
//     fscanf(fp, "%d %d", &n, &m);
//     for (int i = 0; i < n; i++) {
//         fscanf(fp, "%d", &processes[i].s);
//         for (int j = 0; j < m; j++) {
//             fscanf(fp, "%d", &processes[i].searches[j]);
//         }
//     }
//     fclose(fp);
//     printf("+++ Simulation data read from file\n");

//     // Initialize kernel data
//     ready_queue = create_queue(n);
//     free_frames = create_queue(TOTAL_FRAMES);
//     swapped_out = create_queue(n);
//     for (int i = OS_RESERVED; i < TOTAL_FRAMES; i++) {
//         enqueue(free_frames, i);
//     }
//     for (int i = 0; i < n; i++) {
//         init_process(i);
//     }
//     min_active_when_full = n; // Initialize to initial active count
//     printf("+++ Kernel data initialized\n");

//     // Run simulation
//     while (ready_queue->size > 0 || swapped_out->size > 0) {
//         int pid = dequeue(ready_queue);
//         if (pid == -1) break;

//         Process *p = &processes[pid];
//         if (!p->active) continue;

//         int search_idx = p->search_count;
//         if (search_idx >= m) continue;

// #ifdef VERBOSE
//         printf("\tSearch %d by Process %d\n", search_idx + 1, pid);
// #endif

//         if (simulate_binary_search(p, search_idx)) {
//             p->search_count++;
//             if (p->search_count < m) {
//                 enqueue(ready_queue, pid);
//             } else { // Process terminates
//                 active_processes--;
//                 for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//                     enqueue(free_frames, p->essential_frames[i]);
//                 }
//                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                     if (p->page_table[i] >> 15) {
//                         enqueue(free_frames, p->page_table[i] & 0x7FFF);
//                     }
//                 }
//                 if (swapped_out->size > 0 && free_frames->size >= ESSENTIAL_PAGES) {
//                     int swap_pid = dequeue(swapped_out);
//                     init_process(swap_pid);
//                     printf("+++ Swapping in process %3d  [%d active processes]\n", swap_pid, active_processes);
//                 }
//             }
//         }
//     }

//     // Final statistics
//     printf("+++ Page access summary\n");
//     printf("\tTotal number of page accesses  =  %ld\n", page_accesses);
//     printf("\tTotal number of page faults    =  %ld\n", page_faults);
//     printf("\tTotal number of swaps          =  %ld\n", swaps);
//     printf("\tDegree of multiprogramming     =  %d\n", min_active_when_full);

//     free_queue(ready_queue);
//     free_queue(free_frames);
//     free_queue(swapped_out);
// }

// int main() {
//     run_simulation();
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants from the assignment
#define TOTAL_FRAMES 16384      // 64 MB / 4 KB
#define OS_RESERVED 4096        // 16 MB / 4 KB
#define USER_FRAMES 12288       // 48 MB / 4 KB
#define ESSENTIAL_PAGES 10      // Essential pages per process
#define PAGE_SIZE 4096          // 4 KB pages
#define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
#define MAX_PROCESSES 500       // Max n
#define MAX_SEARCHES 100        // Max m
#define INTS_PER_PAGE 1024      // 4096 / 4 = 1024 ints per page

// Process structure
typedef struct {
    int pid;                    // Process ID
    int s;                      // Size of array A (in integers)
    int searches[MAX_SEARCHES]; // Search indices (k0, k1, ...)
    int search_count;           // Number of searches completed
    unsigned short page_table[PAGE_TABLE_SIZE]; // Page table (16-bit entries)
    int essential_frames[ESSENTIAL_PAGES];      // Frames for essential pages
    int active;                 // 1 if active, 0 if swapped out
    int L, R;                   // Binary search state for resumption
    int in_progress;            // 1 if search was interrupted
    char pages_accessed[PAGE_TABLE_SIZE]; // Bitmap for pages accessed in current search
    char pages_faulted[PAGE_TABLE_SIZE];  // Bitmap for pages faulted in current search
} Process;

// Queue structure for FIFO
typedef struct {
    int *data;
    int front, rear, size, capacity;
} Queue;

Queue* create_queue(int capacity) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->data = (int*)malloc(capacity * sizeof(int));
    q->front = q->rear = -1;
    q->size = 0;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue *q, int item) {
    if (q->size == q->capacity) return;
    if (q->front == -1) q->front = 0;
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = item;
    q->size++;
}

int dequeue(Queue *q) {
    if (q->size == 0) return -1;
    int item = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    if (q->size == 0) q->front = q->rear = -1;
    return item;
}

void free_queue(Queue *q) {
    free(q->data);
    free(q);
}

// Global variables
Process processes[MAX_PROCESSES];
int n, m;                   // Number of processes and searches
Queue *ready_queue, *free_frames, *swapped_out;
long page_accesses = 0, page_faults = 0, swaps = 0;
int active_processes = 0;
int min_active_when_full;

// Helper to set valid bit and frame number in page table entry
unsigned short make_pte(int frame, int valid) {
    return (valid << 15) | (frame & 0x7FFF); // MSB is valid bit, 15 bits for frame
}

int simulate_binary_search(Process *p, int search_idx) {
    int k = p->searches[search_idx];
    int L = p->in_progress ? p->L : 0;
    int R = p->in_progress ? p->R : p->s - 1;
    if (!p->in_progress) {
        memset(p->pages_accessed, 0, PAGE_TABLE_SIZE);
        memset(p->pages_faulted, 0, PAGE_TABLE_SIZE);
    }
    p->in_progress = 0;

    while (L < R) {
        int M = (L + R) / 2;
        int page = M / INTS_PER_PAGE;
        unsigned short pte = p->page_table[page];
        int valid = (pte >> 15) & 1;

        if (!p->pages_accessed[page]) {
            page_accesses++;
            p->pages_accessed[page] = 1;
        }
        if (!valid && !p->pages_faulted[page]) {
            page_faults++;
            p->pages_faulted[page] = 1;
        }

        if (!valid) {
            if (free_frames->size == 0) {
                swaps++;
                printf("+++ Swapping out process %3d  [%d active processes]\n", p->pid, active_processes - 1);
                p->active = 0;
                p->in_progress = 1;
                p->L = L;
                p->R = R;
                active_processes--;
                for (int i = 0; i < ESSENTIAL_PAGES; i++) {
                    enqueue(free_frames, p->essential_frames[i]);
                }
                for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
                    if (p->page_table[i] >> 15) {
                        enqueue(free_frames, p->page_table[i] & 0x7FFF);
                        p->page_table[i] = 0;
                    }
                }
                enqueue(swapped_out, p->pid);
                if (active_processes < min_active_when_full && active_processes > 0) {
                    min_active_when_full = active_processes;
                }
                return 0;
            }
            int frame = dequeue(free_frames);
            p->page_table[page] = make_pte(frame, 1);
        }
        if (k <= M) R = M;
        else L = M + 1;
    }
    return 1;
}

// Initialize process and load essential pages
void init_process(int pid) {
    Process *p = &processes[pid];
    p->pid = pid;
    p->active = 1;
    p->search_count = 0;
    p->in_progress = 0;
    memset(p->page_table, 0, sizeof(p->page_table));
    memset(p->pages_accessed, 0, PAGE_TABLE_SIZE);
    memset(p->pages_faulted, 0, PAGE_TABLE_SIZE);
    for (int i = 0; i < ESSENTIAL_PAGES; i++) {
        p->essential_frames[i] = dequeue(free_frames);
    }
    active_processes++;
    enqueue(ready_queue, pid);
}

// Main simulation
void run_simulation() {
    FILE *fp = fopen("search.txt", "r");
    if (!fp) {
        perror("Failed to open search.txt");
        exit(1);
    }
    fscanf(fp, "%d %d", &n, &m);
    for (int i = 0; i < n; i++) {
        fscanf(fp, "%d", &processes[i].s);
        for (int j = 0; j < m; j++) {
            fscanf(fp, "%d", &processes[i].searches[j]);
        }
    }
    fclose(fp);
    printf("+++ Simulation data read from file\n");

    // Initialize kernel data
    ready_queue = create_queue(n);
    free_frames = create_queue(TOTAL_FRAMES);
    swapped_out = create_queue(n);
    for (int i = OS_RESERVED; i < TOTAL_FRAMES; i++) {
        enqueue(free_frames, i);
    }
    for (int i = 0; i < n; i++) {
        init_process(i);
    }
    min_active_when_full = n; // Initialize to initial active count
    printf("+++ Kernel data initialized\n");

    // Run simulation
    while (ready_queue->size > 0 || swapped_out->size > 0) {
        int pid = dequeue(ready_queue);
        if (pid == -1) break;

        Process *p = &processes[pid];
        if (!p->active) continue;

        int search_idx = p->search_count;
        if (search_idx >= m) continue;

#ifdef VERBOSE
        printf("\tSearch %d by Process %d\n", search_idx + 1, pid);
#endif

        if (simulate_binary_search(p, search_idx)) {
            p->search_count++;
            if (p->search_count < m) {
                enqueue(ready_queue, pid);
            } else { // Process terminates
                active_processes--;
                for (int i = 0; i < ESSENTIAL_PAGES; i++) {
                    enqueue(free_frames, p->essential_frames[i]);
                }
                for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
                    if (p->page_table[i] >> 15) {
                        enqueue(free_frames, p->page_table[i] & 0x7FFF);
                    }
                }
                if (swapped_out->size > 0 && free_frames->size >= ESSENTIAL_PAGES) {
                    int swap_pid = dequeue(swapped_out);
                    init_process(swap_pid);
                    printf("+++ Swapping in process %3d  [%d active processes]\n", swap_pid, active_processes);
                }
            }
        }
    }

    // Final statistics
    printf("+++ Page access summary\n");
    printf("\tTotal number of page accesses  =  %ld\n", page_accesses);
    printf("\tTotal number of page faults    =  %ld\n", page_faults);
    printf("\tTotal number of swaps          =  %ld\n", swaps);
    printf("\tDegree of multiprogramming     =  %d\n", min_active_when_full);

    free_queue(ready_queue);
    free_queue(free_frames);
    free_queue(swapped_out);
}

int main() {
    run_simulation();
    return 0;
}