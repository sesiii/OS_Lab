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

// // Process structure
// typedef struct {
//     int pid;                    // Process ID
//     int s;                      // Size of array A (in integers)
//     int searches[MAX_SEARCHES]; // Search indices (k0, k1, ...)
//     int search_count;           // Number of searches completed
//     unsigned short page_table[PAGE_TABLE_SIZE]; // Page table (16-bit entries)
//     int essential_frames[ESSENTIAL_PAGES];      // Frames for essential pages
//     int active;                 // 1 if active, 0 if swapped out
// } Process;

// // Queue structure for FIFO (ready queue, free frames, swapped-out processes)
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
// int min_active_when_full = MAX_PROCESSES;

// // Helper to set valid bit and frame number in page table entry
// unsigned short make_pte(int frame, int valid) {
//     return (valid << 15) | (frame & 0x7FFF); // MSB is valid bit, 15 bits for frame
// }

// // Simulate binary search and handle page faults
// int simulate_binary_search(Process *p, int search_idx) {
//     int k = p->searches[search_idx]; // Target index
//     int L = 0, R = p->s - 1;
//     while (L < R) {
//         int M = (L + R) / 2;
//         page_accesses++;
//         int page = M * sizeof(int) / PAGE_SIZE; // Page containing A[M]
//         unsigned short pte = p->page_table[page];
//         int valid = (pte >> 15) & 1;
//         int frame = pte & 0x7FFF;

//         if (!valid) { // Page fault
//             page_faults++;
//             if (free_frames->size == 0) { // Memory full, swap out
//                 swaps++;
//                 printf("+++ Swapping out process %3d  [%d active processes]\n", p->pid, active_processes - 1);
//                 p->active = 0;
//                 active_processes--;
//                 for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//                     enqueue(free_frames, p->essential_frames[i]);
//                 }
//                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                     if (p->page_table[i] >> 15) { // If valid
//                         enqueue(free_frames, p->page_table[i] & 0x7FFF);
//                         p->page_table[i] = 0; // Invalidate
//                     }
//                 }
//                 enqueue(swapped_out, p->pid);
//                 return 0; // Search interrupted
//             }
//             frame = dequeue(free_frames);
//             p->page_table[page] = make_pte(frame, 1);
//         }
//         if (k <= M) R = M;
//         else L = M + 1;
//     }
//     return 1; // Search completed
// }

// // Initialize process and load essential pages
// void init_process(int pid) {
//     processes[pid].pid = pid;
//     processes[pid].active = 1;
//     processes[pid].search_count = 0;
//     memset(processes[pid].page_table, 0, sizeof(processes[pid].page_table));
//     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//         processes[pid].essential_frames[i] = dequeue(free_frames);
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
//     printf("+++ Kernel data initialized\n");

//     // Run simulation
//     while (ready_queue->size > 0 || swapped_out->size > 0) {
//         int pid = dequeue(ready_queue);
//         if (pid == -1) break; // Shouldn't happen with proper logic

//         Process *p = &processes[pid];
//         if (!p->active) continue; // Skip if swapped out

//         int search_idx = p->search_count;
//         if (search_idx >= m) continue; // Process done

// #ifdef VERBOSE
//         printf("\tSearch %d by Process %d\n", search_idx + 1, pid);
// #endif

//         if (simulate_binary_search(p, search_idx)) {
//             p->search_count++;
//             if (p->search_count < m) {
//                 enqueue(ready_queue, pid); // Back to ready queue
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
//                 // Swap in a process if possible
//                 if (swapped_out->size > 0 && free_frames->size >= ESSENTIAL_PAGES) {
//                     int swap_pid = dequeue(swapped_out);
//                     init_process(swap_pid);
//                     printf("+++ Swapping in process %3d  [%d active processes]\n", swap_pid, active_processes);
//                 }
//             }
//         }

//         // Update degree of multiprogramming
//         if (free_frames->size == 0 && active_processes < min_active_when_full) {
//             min_active_when_full = active_processes;
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
#include <stdint.h>
#include <stdbool.h>

// Constants
#define TOTAL_FRAMES 16384        // 64 MB / 4 KB
#define USER_FRAMES 12288         // 48 MB / 4 KB
#define PAGE_SIZE 4096            // 4 KB
#define ESSENTIAL_PAGES 10        // Essential pages per process
#define PAGE_TABLE_SIZE 2048      // 2048 pages per process
#define MAX_PROCESSES 500         // Max number of processes
#define MAX_SEARCHES 100          // Max searches per process
#define INTS_PER_PAGE (PAGE_SIZE / sizeof(int32_t)) // 1024 integers per 4 KB page

// Process structure
typedef struct {
    int pid;                      // Process ID
    int size;                     // Size of array A (in integers)
    int searches[MAX_SEARCHES];   // Search indices
    int num_searches;             // Number of searches (m)
    int current_search;           // Current search index
    uint16_t page_table[PAGE_TABLE_SIZE]; // Page table (16-bit entries)
    int frame_count;              // Number of frames allocated
} Process;

// Queue structure for FIFO
typedef struct {
    int* items;
    int front;
    int rear;
    int capacity;
} Queue;

Queue* create_queue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->items = (int*)malloc(capacity * sizeof(int));
    q->front = 0;
    q->rear = -1;
    q->capacity = capacity;
    return q;
}

void enqueue(Queue* q, int item) {
    if (q->rear + 1 < q->capacity) {
        q->rear++;
        q->items[q->rear] = item;
    }
}

int dequeue(Queue* q) {
    if (q->front > q->rear) return -1;
    int item = q->items[q->front];
    q->front++;
    return item;
}

bool is_empty(Queue* q) {
    return q->front > q->rear;
}

int queue_size(Queue* q) {
    if (is_empty(q)) return 0;
    return q->rear - q->front + 1;
}

void free_queue(Queue* q) {
    free(q->items);
    free(q);
}

// Global simulation state
Process processes[MAX_PROCESSES];
int num_processes;                // n
int num_searches;                 // m
Queue* ready_queue;
Queue* free_frames;
Queue* swapped_out;
int active_processes = 0;
int total_page_accesses = 0;
int total_page_faults = 0;
int total_swaps = 0;
int min_active_when_full = MAX_PROCESSES;

// Initialize page table entry
void init_page_table(uint16_t* table) {
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        table[i] = 0; // Invalid bit (MSB) is 0
    }
}

// Allocate a frame and update page table
int allocate_frame(Process* p, int page) {
    int frame = dequeue(free_frames);
    if (frame == -1) return -1; // No free frames
    p->page_table[page] = (1 << 15) | frame; // Set valid bit and frame number
    p->frame_count++;
    return frame;
}

// Free all frames of a process
void free_process_frames(Process* p) {
    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        if (p->page_table[i] & (1 << 15)) { // Valid bit check
            int frame = p->page_table[i] & 0x7FFF; // Mask to get frame number
            enqueue(free_frames, frame);
        }
    }
    p->frame_count = 0;
    init_page_table(p->page_table);
}

// Load essential pages
void load_essential_pages(Process* p) {
    for (int i = 0; i < ESSENTIAL_PAGES; i++) {
        int frame = allocate_frame(p, i);
        if (frame == -1) {
            fprintf(stderr, "Error: Not enough frames for essential pages\n");
            exit(1);
        }
    }
}

// Simulate binary search
bool binary_search(Process* p, int search_idx) {
    int k = p->searches[search_idx];
    int L = 0, R = p->size - 1;
    while (L < R) {
        int M = (L + R) / 2;
        total_page_accesses++;
        int page = M / INTS_PER_PAGE;
        if (!(p->page_table[page] & (1 << 15))) { // Page fault
            total_page_faults++;
            int frame = allocate_frame(p, page);
            if (frame == -1) return false; // Swap out needed
        }
        if (k <= M) R = M;
        else L = M + 1;
    }
    return true;
}

// Swap out a process
void swap_out(int pid) {
    Process* p = &processes[pid];
    free_process_frames(p);
    load_essential_pages(p); // Reload essential pages after swap-out
    enqueue(swapped_out, pid);
    active_processes--;
    total_swaps++;
    printf("+++ Swapping out process %4d  [%3d active processes]\n", pid, active_processes);
    if (queue_size(free_frames) == 0) {
        if (active_processes < min_active_when_full) {
            min_active_when_full = active_processes;
        }
    }
}

// Swap in a process
void swap_in(int pid) {
    Process* p = &processes[pid];
    load_essential_pages(p);
    active_processes++;
    printf("+++ Swapping in process %4d  [%3d active processes]\n", pid, active_processes);
    enqueue(ready_queue, pid);
}

// Read input from search.txt
void read_input() {
    FILE* fp = fopen("search.txt", "r");
    if (!fp) {
        perror("Failed to open search.txt");
        exit(1);
    }
    fscanf(fp, "%d %d", &num_processes, &num_searches);
    if (num_processes < 50 || num_processes > 500 || num_searches < 10 || num_searches > 100) {
        fprintf(stderr, "Invalid n or m: n=%d, m=%d\n", num_processes, num_searches);
        exit(1);
    }
    for (int i = 0; i < num_processes; i++) {
        Process* p = &processes[i];
        p->pid = i;
        fscanf(fp, "%d", &p->size);
        if (p->size < 106 || p->size > 2 * 106) {
            fprintf(stderr, "Invalid size for process %d: %d\n", i, p->size);
            exit(1);
        }
        p->num_searches = num_searches;
        p->current_search = 0;
        for (int j = 0; j < num_searches; j++) {
            fscanf(fp, "%d", &p->searches[j]);
            if (p->searches[j] < 0 || p->searches[j] >= p->size) {
                fprintf(stderr, "Invalid search index for process %d, search %d: %d\n", i, j, p->searches[j]);
                exit(1);
            }
        }
        init_page_table(p->page_table);
    }
    fclose(fp);
    printf("+++ Simulation data read from file\n");
}

// Initialize kernel data
void init_kernel() {
    ready_queue = create_queue(num_processes);
    free_frames = create_queue(USER_FRAMES);
    swapped_out = create_queue(num_processes);
    for (int i = 0; i < USER_FRAMES; i++) {
        enqueue(free_frames, i);
    }
    for (int i = 0; i < num_processes; i++) {
        enqueue(ready_queue, i);
        load_essential_pages(&processes[i]);
        active_processes++;
    }
    min_active_when_full = active_processes; // Initial value
    printf("+++ Kernel data initialized\n");
}

// Main simulation loop
void simulate() {
    while (!is_empty(ready_queue) || !is_empty(swapped_out)) {
        int pid = dequeue(ready_queue);
        if (pid == -1) {
            if (!is_empty(swapped_out)) {
                int swapped_pid = dequeue(swapped_out);
                if (swapped_pid != -1) swap_in(swapped_pid);
            }
            continue;
        }

        Process* p = &processes[pid];
        if (p->current_search >= p->num_searches) {
            free_process_frames(p);
            active_processes--;
            if (!is_empty(swapped_out)) {
                int swapped_pid = dequeue(swapped_out);
                if (swapped_pid != -1) swap_in(swapped_pid);
            }
            continue;
        }

#ifdef VERBOSE
        printf("\tSearch %d by Process %d\n", p->current_search + 1, pid);
#endif

        if (!binary_search(p, p->current_search)) {
            swap_out(pid);
            continue;
        }

        p->current_search++;
        if (p->current_search < p->num_searches) {
            enqueue(ready_queue, pid);
        } else {
            free_process_frames(p);
            active_processes--;
            if (!is_empty(swapped_out)) {
                int swapped_pid = dequeue(swapped_out);
                if (swapped_pid != -1) swap_in(swapped_pid);
            }
        }
    }
}

// Print final statistics
void print_stats() {
    printf("+++ Page access summary\n");
    printf("\tTotal number of page accesses  =  %d\n", total_page_accesses);
    printf("\tTotal number of page faults    =  %d\n", total_page_faults);
    printf("\tTotal number of swaps          =  %d\n", total_swaps);
    printf("\tDegree of multiprogramming     =  %d\n", min_active_when_full);
}

int main() {
    read_input();
    init_kernel();
    simulate();
    print_stats();
    free_queue(ready_queue);
    free_queue(free_frames);
    free_queue(swapped_out);
    return 0;
}