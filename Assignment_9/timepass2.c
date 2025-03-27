// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>

// // #define TOTAL_FRAMES 16384      // Total memory frames (64 MB / 4 KB)
// // #define USER_FRAMES 12288       // User-available frames (48 MB / 4 KB)
// // #define ESSENTIAL_PAGES 10      // Essential pages per process
// // #define PAGE_SIZE 4096          // Page size in bytes
// // #define INTS_PER_PAGE 1024      // Integers per page (4096 / 4)
// // #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// // #define MAX_PROCESSES 500       // Max n
// // #define MAX_SEARCHES 100        // Max m

// // // Process structure
// // typedef struct {
// //     int pid;                    // Process ID
// //     int addi_data_seg;          // Number of additional data segment pages loaded
// //     int cur_search_idx;         // Current search index
// //     int max_A_size;             // Size of array A in integers
// //     int searches[MAX_SEARCHES]; // Search keys
// //     int is_swapped;             // 1 if swapped out, 0 if active
// //     int L, R;                   // Binary search state
// //     int in_progress;            // 1 if search interrupted
// //     char pages_accessed[PAGE_TABLE_SIZE]; // Bitmap for pages accessed in current search
// //     char pages_faulted[PAGE_TABLE_SIZE];  // Bitmap for pages faulted in current search
// // } Process;

// // // Queue structure for FIFO
// // typedef struct {
// //     Process *data[MAX_PROCESSES]; // Array of process pointers
// //     int front, rear, size, capacity;
// // } Queue;

// // // Page table structure
// // typedef struct {
// //     unsigned short entries[PAGE_TABLE_SIZE]; // 16-bit page table entries
// // } PageTable;

// // // Global variables
// // Process *processes;             // Dynamic array of processes
// // PageTable *page_tables;         // Dynamic array of page tables
// // int n, m;                       // Number of processes and searches
// // Queue ready_queue, swapped_out; // Queues for ready and swapped-out processes
// // int tot_page_accs = 0;          // Total page accesses
// // int tot_page_faults = 0;        // Total page faults
// // int tot_swaps = 0;              // Total swaps
// // int available_frames = USER_FRAMES; // Free frames available
// // int multi_prog;                 // Degree of multiprogramming
// // int completed_count = 0;        // Number of completed processes

// // // Initialize queue
// // Queue* create_queue(int capacity) {
// //     Queue *q = (Queue*)malloc(sizeof(Queue));
// //     q->front = q->rear = -1;
// //     q->size = 0;
// //     q->capacity = capacity;
// //     return q;
// // }

// // // Add to queue
// // void enqueue(Queue *q, Process *item) {
// //     if (q->size == q->capacity) return;
// //     if (q->front == -1) q->front = 0;
// //     q->rear = (q->rear + 1) % q->capacity;
// //     q->data[q->rear] = item;
// //     q->size++;
// // }

// // // Remove from queue
// // Process* dequeue(Queue *q) {
// //     if (q->size == 0) return NULL;
// //     Process *item = q->data[q->front];
// //     q->front = (q->front + 1) % q->capacity;
// //     q->size--;
// //     if (q->size == 0) q->front = q->rear = -1;
// //     return item;
// // }

// // // Free queue
// // void free_queue(Queue *q) {
// //     free(q);
// // }

// // // Set valid bit in page table
// // void page_set(PageTable *page_tables, int process_id, int entry) {
// //     if (!(page_tables[process_id].entries[entry] & (1 << 15))) { // If not valid
// //         page_tables[process_id].entries[entry] |= (1 << 15);     // Set valid bit
// //         if (entry >= ESSENTIAL_PAGES) {                          // Track additional pages
// //             processes[process_id].addi_data_seg++;
// //         }
// //         available_frames--;                                      // Reduce free frames
// //     }
// // }

// // // Clear valid bit in page table
// // void page_clear(PageTable *page_tables, int process_id, int entry) {
// //     if (page_tables[process_id].entries[entry] & (1 << 15)) { // If valid
// //         page_tables[process_id].entries[entry] &= ~(1 << 15); // Clear valid bit
// //         if (entry >= ESSENTIAL_PAGES) {                       // Update additional pages
// //             processes[process_id].addi_data_seg--;
// //         }
// //         available_frames++;                                   // Increase free frames
// //     }
// // }

// // // Check if page is valid
// // int page_retrieve(PageTable *page_tables, int process_id, int entry) {
// //     return page_tables[process_id].entries[entry] >> 15;      // Return valid bit
// // }

// // // Swap out a process
// // void swap_out(Process *proc) {
// //     proc->is_swapped = 1;                                     // Mark as swapped out
// //     proc->in_progress = 1;                                    // Search interrupted
// //     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {               // Free all frames
// //         if (page_retrieve(page_tables, proc->pid, i)) {
// //             page_clear(page_tables, proc->pid, i);
// //         }
// //     }
// //     enqueue(&swapped_out, proc);                              // Add to swapped-out queue
// //     int active = n - swapped_out.size - completed_count;      // Active processes
// //     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);
// //     tot_swaps++;                                              // Increment swaps
// //     if (available_frames == 0 && active > 0) {                // Update multiprogramming
// //         multi_prog = (active < multi_prog) ? active : multi_prog;
// //     }
// // }

// // // Swap in a process
// // void swap_in(Process *proc) {
// //     proc->is_swapped = 0;                                     // Mark as active
// //     for (int i = 0; i < ESSENTIAL_PAGES; i++) {               // Load essential pages
// //         page_set(page_tables, proc->pid, i);
// //     }
// //     enqueue(&ready_queue, proc);                              // Add to ready queue
// //     int active = n - swapped_out.size - completed_count;      // Active processes
// //     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// // }

// // // Simulate binary search with demand paging
// // int simulate_binary_search(Process *proc) {
// //     int key = proc->searches[proc->cur_search_idx];           // Current search key
// //     int L = proc->in_progress ? proc->L : 0;                  // Resume or start left
// //     int R = proc->in_progress ? proc->R : proc->max_A_size - 1; // Resume or start right
// //     if (!proc->in_progress) {                                 // New search, reset bitmaps
// //         memset(proc->pages_accessed, 0, PAGE_TABLE_SIZE);
// //         memset(proc->pages_faulted, 0, PAGE_TABLE_SIZE);
// //     }
// //     proc->in_progress = 0;                                    // Reset unless swapped out

// //     while (L < R) {
// //         int m = (L + R) / 2;                                  // Middle index
// //         int page = m / INTS_PER_PAGE;                         // Page for A[m]
// //         if (!proc->pages_accessed[page]) {                    // First access in this search
// //             tot_page_accs++;                                  // Count page access
// //             proc->pages_accessed[page] = 1;
// //         }
// //         if (!page_retrieve(page_tables, proc->pid, page)) {   // Page fault
// //             if (!proc->pages_faulted[page]) {                 // First fault in this search
// //                 tot_page_faults++;                            // Count page fault
// //                 proc->pages_faulted[page] = 1;
// //             }
// //             if (available_frames <= 0) {                      // No free frames
// //                 proc->L = L;                                  // Save state
// //                 proc->R = R;
// //                 swap_out(proc);                               // Swap out process
// //                 return 0;                                     // Search interrupted
// //             }
// //             page_set(page_tables, proc->pid, page);           // Load page
// //         }
// //         if (key <= m) R = m;                                  // Update search bounds
// //         else L = m + 1;
// //     }
// //     return 1;                                                 // Search completed
// // }

// // int main() {
// //     FILE *fin = fopen("search.txt", "r");
// //     if (!fin) {
// //         printf("Error: Could not open search.txt\n");
// //         return 1;
// //     }

// //     fscanf(fin, "%d %d", &n, &m);                            // Read n and m
// //     processes = (Process*)malloc(n * sizeof(Process));
// //     page_tables = (PageTable*)malloc(n * sizeof(PageTable));
// //     memset(page_tables, 0, n * sizeof(PageTable));           // Initialize page tables
// //     ready_queue = *create_queue(n);                          // Initialize ready queue
// //     swapped_out = *create_queue(n);                          // Initialize swapped-out queue

// //     printf("+++ Simulation data read from file\n");
// //     for (int i = 0; i < n; i++) {                            // Initialize processes
// //         fscanf(fin, "%d", &processes[i].max_A_size);
// //         processes[i].is_swapped = 0;
// //         processes[i].cur_search_idx = 0;
// //         processes[i].addi_data_seg = 0;
// //         processes[i].pid = i;
// //         processes[i].in_progress = 0;
// //         memset(processes[i].pages_accessed, 0, PAGE_TABLE_SIZE);
// //         memset(processes[i].pages_faulted, 0, PAGE_TABLE_SIZE);
// //         for (int j = 0; j < ESSENTIAL_PAGES; j++) {          // Load essential pages
// //             page_set(page_tables, i, j);
// //         }
// //         for (int j = 0; j < m; j++) {                        // Read search keys
// //             fscanf(fin, "%d", &processes[i].searches[j]);
// //         }
// //         enqueue(&ready_queue, &processes[i]);                // Add to ready queue
// //     }
// //     fclose(fin);
// //     multi_prog = n;                                          // Initial multiprogramming
// //     printf("+++ Kernel data initialized\n");

// //     while (ready_queue.size > 0 || swapped_out.size > 0) {   // Main simulation loop
// //         if (ready_queue.size == 0 && swapped_out.size > 0) { // Handle swapped-out processes
// //             Process *proc = dequeue(&swapped_out);
// //             if (available_frames >= ESSENTIAL_PAGES) {
// //                 swap_in(proc);
// //             } else {
// //                 enqueue(&swapped_out, proc);
// //                 break;
// //             }
// //             continue;
// //         }

// //         Process *current = dequeue(&ready_queue);            // Get next process
// //         if (!current) break;

// //         if (current->cur_search_idx >= m) {                  // Process completed
// //             completed_count++;
// //             for (int i = 0; i < PAGE_TABLE_SIZE; i++) {      // Free all frames
// //                 page_clear(page_tables, current->pid, i);
// //             }
// //             current->addi_data_seg = 0;
// //             if (swapped_out.size > 0 && available_frames >= ESSENTIAL_PAGES) {
// //                 Process *next = dequeue(&swapped_out);       // Swap in one process
// //                 swap_in(next);
// //             }
// //             continue;
// //         }

// // #ifdef VERBOSE
// //         printf("\tSearch %d by Process %d\n", current->cur_search_idx + 1, current->pid);
// // #endif

// //         if (simulate_binary_search(current)) {               // Run binary search
// //             current->cur_search_idx++;                       // Move to next search
// //             if (current->cur_search_idx < m) {               // More searches remain
// //                 enqueue(&ready_queue, current);
// //             } else {                                         // Process finished
// //                 completed_count++;
// //                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {  // Free all frames
// //                     page_clear(page_tables, current->pid, i);
// //                 }
// //                 current->addi_data_seg = 0;
// //                 if (swapped_out.size > 0 && available_frames >= ESSENTIAL_PAGES) {
// //                     Process *next = dequeue(&swapped_out);   // Swap in one process
// //                     swap_in(next);
// //                 }
// //             }
// //         }
// //     }

// //     // Print final statistics
// //     printf("+++ Page access summary\n");
// //     printf("Total number of page accesses = %d\n", tot_page_accs);
// //     printf("Total number of page faults = %d\n", tot_page_faults);
// //     printf("Total number of swaps = %d\n", tot_swaps);
// //     printf("Degree of multiprogramming = %d\n", multi_prog);

// //     free(processes);                                         // Clean up
// //     free(page_tables);
// //     free_queue(&ready_queue);
// //     free_queue(&swapped_out);
// //     return 0;
// // }


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #define TOTAL_FRAMES 16384      // Total memory frames (64 MB / 4 KB)
// #define USER_FRAMES 12288       // User-available frames (48 MB / 4 KB)
// #define ESSENTIAL_PAGES 10      // Essential pages per process
// #define PAGE_SIZE 4096          // Page size in bytes
// #define INTS_PER_PAGE 1024      // Integers per page (4096 / 4)
// #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// #define MAX_PROCESSES 500       // Max n
// #define MAX_SEARCHES 100        // Max m

// // Process structure
// typedef struct {
//     int pid;                    // Process ID
//     int addi_data_seg;          // Number of additional data segment pages loaded
//     int cur_search_idx;         // Current search index
//     int max_A_size;             // Size of array A in integers
//     int searches[MAX_SEARCHES]; // Search keys
//     int is_swapped;             // 1 if swapped out, 0 if active
// } Process;

// // Page table structure
// typedef struct {
//     unsigned short entries[PAGE_TABLE_SIZE]; // 16-bit page table entries
// } PageTable;

// // Global variables
// Process *processes;             // Dynamic array of processes
// PageTable *page_tables;         // Dynamic array of page tables
// int n, m;                       // Number of processes and searches
// int tot_page_accs = 0;          // Total page accesses
// int tot_page_faults = 0;        // Total page faults
// int tot_swaps = 0;              // Total swaps
// int available_frames = USER_FRAMES; // Free frames available
// int multi_prog = 0;             // Degree of multiprogramming
// int completed_count = 0;        // Number of completed processes

// // Custom queue implementation for swapped and ready processes
// typedef struct {
//     Process **data;
//     int front, rear, size, capacity;
// } Queue;

// // Create a new queue
// Queue* create_queue(int capacity) {
//     Queue *q = (Queue*)malloc(sizeof(Queue));
//     q->data = (Process**)malloc(capacity * sizeof(Process*));
//     q->front = q->rear = -1;
//     q->size = 0;
//     q->capacity = capacity;
//     return q;
// }

// // Add to queue
// void enqueue(Queue *q, Process *item) {
//     if (q->size == q->capacity) return;
    
//     if (q->front == -1) q->front = 0;
    
//     // For swapped out, we want to push to front
//     if (item->is_swapped) {
//         // Shift elements to make space at front
//         for (int i = q->rear; i >= q->front; i--) {
//             q->data[i + 1] = q->data[i];
//         }
//         q->data[q->front] = item;
//         q->rear++;
//     } else {
//         // For ready processes, push to rear
//         q->rear = (q->rear + 1) % q->capacity;
//         q->data[q->rear] = item;
//     }
//     q->size++;
// }

// // Remove from queue
// Process* dequeue(Queue *q) {
//     if (q->size == 0) return NULL;
    
//     Process *item = q->data[q->front];
    
//     if (q->size == 1) {
//         q->front = q->rear = -1;
//     } else {
//         q->front = (q->front + 1) % q->capacity;
//     }
//     q->size--;
    
//     return item;
// }

// // Page table management functions
// void page_set(PageTable *page_tables, int process_id, int entry) {
//     if (!(page_tables[process_id].entries[entry] & (1 << 15))) {
//         page_tables[process_id].entries[entry] |= (1 << 15);
//         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg++;
//         available_frames--;
//     }
// }

// void page_clear(PageTable *page_tables, int process_id, int entry) {
//     if (page_tables[process_id].entries[entry] & (1 << 15)) {
//         page_tables[process_id].entries[entry] &= ~(1 << 15);
//         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg--;
//         available_frames++;
//     }
// }

// int page_retrieve(PageTable *page_tables, int process_id, int entry) {
//     return page_tables[process_id].entries[entry] >> 15;
// }

// // Swap out a process
// void swap_out(Process *proc, Queue *ready_queue, Queue *swapped_out) {
//     proc->is_swapped = 1;
    
//     // Clear all page table entries
//     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//         if (page_retrieve(page_tables, proc->pid, i)) {
//             page_clear(page_tables, proc->pid, i);
//         }
//     }
//     proc->addi_data_seg = 0;
    
//     // Add to swapped out queue
//     enqueue(swapped_out, proc);
    
//     // Update active processes count
//     int swapped_out_count = swapped_out->size;
//     int active = n - swapped_out_count - completed_count;
    
//     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);
    
//     // Update total swaps and multiprogramming degree
//     tot_swaps++;
//     multi_prog = (active < multi_prog || multi_prog == 0) ? active : multi_prog;
// }

// // Swap in a process
// void swap_in(Process *proc, Queue *ready_queue) {
//     proc->is_swapped = 0;
    
//     // Load essential pages
//     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//         page_set(page_tables, proc->pid, i);
//     }
    
//     // Add to ready queue with priority
//     enqueue(ready_queue, proc);
    
//     // Update active processes count
//     int swapped_out_count = 0; // You'll need to track this separately
//     int active = n - swapped_out_count - completed_count;
    
//     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// }

// int main() {
//     FILE *fin = fopen("search.txt", "r");
//     if (!fin) {
//         printf("Error: Could not open search.txt\n");
//         return 1;
//     }

//     // Read number of processes and searches
//     fscanf(fin, "%d %d", &n, &m);
    
//     // Allocate memory for processes and page tables
//     processes = (Process*)malloc(n * sizeof(Process));
//     page_tables = (PageTable*)malloc(n * sizeof(PageTable));
    
//     // Create queues
//     Queue *ready_queue = create_queue(n);
//     Queue *swapped_out = create_queue(n);

//     printf("+++ Simulation data read from file\n");
    
//     // Initialize processes
//     for (int i = 0; i < n; i++) {
//         fscanf(fin, "%d", &processes[i].max_A_size);
        
//         processes[i].pid = i;
//         processes[i].cur_search_idx = 0;
//         processes[i].is_swapped = 0;
//         processes[i].addi_data_seg = 0;
        
//         // Load essential pages
//         for (int j = 0; j < ESSENTIAL_PAGES; j++) {
//             page_set(page_tables, i, j);
//         }
        
//         // Read search keys
//         for (int j = 0; j < m; j++) {
//             fscanf(fin, "%d", &processes[i].searches[j]);
//         }
        
//         // Add to ready queue
//         enqueue(ready_queue, &processes[i]);
//     }
//     fclose(fin);

//     // Set initial multiprogramming to number of processes
//     multi_prog = n;
//     printf("+++ Kernel data initialized\n");

//     // Main simulation loop
//     while (ready_queue->size > 0 || swapped_out->size > 0) {
//         // Handle swapped out processes when ready queue is empty
//         if (ready_queue->size == 0 && swapped_out->size > 0) {
//             Process *proc = dequeue(swapped_out);
//             if (available_frames >= ESSENTIAL_PAGES) {
//                 swap_in(proc, ready_queue);
//             } else {
//                 enqueue(swapped_out, proc);
//                 break;
//             }
//             continue;
//         }

//         // Get next process
//         Process *current = dequeue(ready_queue);
        
//         // Check if process has completed all searches
//         if (current->cur_search_idx >= m) {
//             completed_count++;
            
//             // Clear all page table entries
//             for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                 page_clear(page_tables, current->pid, i);
//             }
//             current->addi_data_seg = 0;
            
//             // Try to swap in a new process
//             if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
//                 Process *next = dequeue(swapped_out);
//                 swap_in(next, ready_queue);
//             }
//             continue;
//         }

//         // Perform binary search
//         int key = current->searches[current->cur_search_idx];
//         int l = 0, r = current->max_A_size - 1;
//         int swapped_out_flag = 0;

//         while (l < r && !swapped_out_flag) {
//             int m = (l + r) / 2;
//             tot_page_accs++;
            
//             // Determine page for current index
//             int page = 10 + (m / 1024);
            
//             // Check page validity
//             if (!page_retrieve(page_tables, current->pid, page)) {
//                 tot_page_faults++;
                
//                 // Swap out if no frames available
//                 if (available_frames <= 0) {
//                     swap_out(current, ready_queue, swapped_out);
//                     swapped_out_flag = 1;
//                     break;
//                 }
                
//                 // Load page
//                 page_set(page_tables, current->pid, page);
//             }
            
//             // Binary search logic
//             if (key <= m) r = m;
//             else l = m + 1;
//         }

//         // Handle process after binary search
//         if (!swapped_out_flag) {
//             current->cur_search_idx++;
            
//             if (current->cur_search_idx < m) {
//                 // More searches left, add back to queue
//                 enqueue(ready_queue, current);
//             } else {
//                 // Process completed
//                 completed_count++;
                
//                 // Clear page table
//                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                     page_clear(page_tables, current->pid, i);
//                 }
//                 current->addi_data_seg = 0;
                
//                 // Try to swap in a new process
//                 if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
//                     Process *next = dequeue(swapped_out);
//                     swap_in(next, ready_queue);
//                 }
//             }
//         }
//     }

//     // Print simulation results
//     printf("+++ Page access summary\n");
//     printf("Total number of page accesses = %d\n", tot_page_accs);
//     printf("Total number of page faults = %d\n", tot_page_faults);
//     printf("Total number of swaps = %d\n", tot_swaps);
//     printf("Degree of multiprogramming = %d\n", multi_prog);

//     // Clean up
//     free(processes);
//     free(page_tables);
//     free(ready_queue->data);
//     free(ready_queue);
//     free(swapped_out->data);
//     free(swapped_out);

//     return 0;
// }



// // // #include <stdio.h>
// // // #include <stdlib.h>
// // // #include <string.h>
// // // #include <limits.h>

// // // #define TOTAL_FRAMES 16384      // Total memory frames (64 MB / 4 KB)
// // // #define USER_FRAMES 12288       // User-available frames (48 MB / 4 KB)
// // // #define ESSENTIAL_PAGES 10      // Essential pages per process
// // // #define PAGE_SIZE 4096          // Page size in bytes
// // // #define INTS_PER_PAGE 1024      // Integers per page (4096 / 4)
// // // #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// // // #define MAX_PROCESSES 500       // Max n
// // // #define MAX_SEARCHES 100        // Max m

// // // // Process structure closely mimicking reference implementation
// // // typedef struct {
// // //     int pid;                    // Process ID
// // //     int addi_data_seg;          // Number of additional data segment pages loaded
// // //     int cur_search_idx;         // Current search index
// // //     int max_A_size;             // Size of array A in integers
// // //     int searches[MAX_SEARCHES]; // Search keys
// // //     int is_swapped;             // 1 if swapped out, 0 if active
// // // } Process;

// // // // Page table structure
// // // typedef struct {
// // //     unsigned short entries[PAGE_TABLE_SIZE]; // 16-bit page table entries
// // // } PageTable;

// // // // Global variables
// // // Process *processes;             // Dynamic array of processes
// // // PageTable *page_tables;         // Dynamic array of page tables
// // // int n, m;                       // Number of processes and searches
// // // int tot_page_accs = 0;          // Total page accesses
// // // int tot_page_faults = 0;        // Total page faults
// // // int tot_swaps = 0;              // Total swaps
// // // int available_frames = USER_FRAMES; // Free frames available
// // // int multi_prog = INT_MAX;       // Degree of multiprogramming (initialized to max)
// // // int completed_count = 0;        // Number of completed processes

// // // // Custom queue to exactly match reference behavior
// // // typedef struct {
// // //     Process **data;
// // //     int front, rear, size, capacity;
// // // } Queue;

// // // // Create a new queue
// // // Queue* create_queue(int capacity) {
// // //     Queue *q = (Queue*)malloc(sizeof(Queue));
// // //     q->data = (Process**)malloc(capacity * sizeof(Process*));
// // //     q->front = q->rear = -1;
// // //     q->size = 0;
// // //     q->capacity = capacity;
// // //     return q;
// // // }

// // // // Custom enqueue to match reference implementation's queue behavior
// // // void enqueue(Queue *q, Process *item) {
// // //     if (q->size == q->capacity) return;
    
// // //     if (q->front == -1) q->front = 0;
// // //     q->rear = (q->rear + 1) % q->capacity;
// // //     q->data[q->rear] = item;
// // //     q->size++;
// // // }

// // // // // Custom dequeue to match reference implementation
// // // // Process* dequeue(Queue *q) {
// // // //     if (q->size == 0) return NULL;
    
// // // //     Process *item = q->data[q->front];
    
// // // //     if (q->size == 1) {
// // // //         q->front = q->rear = -1;
// // // //     } else {
// // // //         q->front = (q->front + 1) % q->capacity;
// // // //     }
// // // //     q->size--;
    
// // // //     return item;
// // // // }

// // // Process* dequeue(Queue *q) {
// // //     if (q->size == 0) return NULL;
    
// // //     Process *item = q->data[q->front];
    
// // //     if (q->size == 1) {
// // //         q->front = q->rear = -1;
// // //     } else {
// // //         q->front = (q->front + 1) % q->capacity;
// // //     }
// // //     q->size--;
    
// // //     return item;
// // // }

// // // // Page table management functions
// // // void page_set(PageTable *page_tables, int process_id, int entry) {
// // //     if (!(page_tables[process_id].entries[entry] & (1 << 15))) {
// // //         page_tables[process_id].entries[entry] |= (1 << 15);
// // //         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg++;
// // //         available_frames--;
// // //     }
// // // }

// // // void page_clear(PageTable *page_tables, int process_id, int entry) {
// // //     if (page_tables[process_id].entries[entry] & (1 << 15)) {
// // //         page_tables[process_id].entries[entry] &= ~(1 << 15);
// // //         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg--;
// // //         available_frames++;
// // //     }
// // // }

// // // int page_retrieve(PageTable *page_tables, int process_id, int entry) {
// // //     return page_tables[process_id].entries[entry] >> 15;
// // // }

// // // // Swap out a process
// // // // void swap_out(Process *proc, Queue *ready_queue, Queue *swapped_out) {
// // // //     proc->is_swapped = 1;
    
// // // //     // Clear all page table entries
// // // //     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// // // //         if (page_retrieve(page_tables, proc->pid, i)) {
// // // //             page_clear(page_tables, proc->pid, i);
// // // //         }
// // // //     }
// // // //     proc->addi_data_seg = 0;
    
// // // //     // Enqueue to swapped out queue
// // // //     enqueue(swapped_out, proc);
    
// // // //     // Calculate active processes
// // // //     int swapped_out_count = swapped_out->size;
// // // //     int active = n - swapped_out_count - completed_count;
    
// // // //     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);
    
// // // //     // Update total swaps and minimum multiprogramming degree
// // // //     tot_swaps++;
// // // //     multi_prog = (active < multi_prog) ? active : multi_prog;
// // // // }

// // // void swap_out(Process *proc, Queue *ready_queue, Queue *swapped_out) {
// // //     proc->is_swapped = 1;
    
// // //     // Clear all page table entries
// // //     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// // //         if (page_retrieve(page_tables, proc->pid, i)) {
// // //             page_clear(page_tables, proc->pid, i);
// // //         }
// // //     }
// // //     proc->addi_data_seg = 0;
    
// // //     // Enqueue to swapped out queue
// // //     enqueue(swapped_out, proc);
    
// // //     // Calculate active processes
// // //     int swapped_out_count = swapped_out->size;
// // //     int active = n - swapped_out_count - completed_count;
    
// // //     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);
    
// // //     // Update total swaps and minimum multiprogramming degree
// // //     tot_swaps++;
// // //     multi_prog = (active < multi_prog) ? active : multi_prog;
// // // }

// // // // Swap in a process with priority handling
// // // // void swap_in(Process *proc, Queue *ready_queue) {
// // // //     proc->is_swapped = 0;
    
// // // //     // Load essential pages
// // // //     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// // // //         page_set(page_tables, proc->pid, i);
// // // //     }
    
// // // //     // Recreate queue with current process at front
// // // //     Queue *temp_queue = create_queue(ready_queue->capacity);
// // // //     temp_queue->data[0] = proc;
// // // //     temp_queue->front = 0;
// // // //     temp_queue->rear = 0;
// // // //     temp_queue->size = 1;
    
// // // //     // Move remaining processes
// // // //     while (ready_queue->size > 0) {
// // // //         Process *temp = dequeue(ready_queue);
// // // //         enqueue(temp_queue, temp);
// // // //     }
    
// // // //     // Copy back to original queue
// // // //     memcpy(ready_queue->data, temp_queue->data, temp_queue->capacity * sizeof(Process*));
// // // //     ready_queue->front = temp_queue->front;
// // // //     ready_queue->rear = temp_queue->rear;
// // // //     ready_queue->size = temp_queue->size;
    
// // // //     // Free temporary queue
// // // //     free(temp_queue->data);
// // // //     free(temp_queue);
    
// // // //     // Calculate active processes
// // // //     int swapped_out_count = 0; // This will be handled by caller
// // // //     int active = n - swapped_out_count - completed_count;
    
// // // //     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// // // // }
// // // void swap_in(Process *proc, Queue *ready_queue) {
// // //     proc->is_swapped = 0;
    
// // //     // Load essential pages
// // //     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// // //         page_set(page_tables, proc->pid, i);
// // //     }
    
// // //     // Recreate queue with current process at front using a deque-like approach
// // //     Queue *temp_queue = create_queue(ready_queue->capacity);
    
// // //     // First, add the swapped-in process to the front
// // //     enqueue(temp_queue, proc);
    
// // //     // Then add all processes from the original queue
// // //     int original_size = ready_queue->size;
// // //     for (int i = 0; i < original_size; i++) {
// // //         Process *temp = dequeue(ready_queue);
// // //         enqueue(temp_queue, temp);
// // //     }
    
// // //     // Copy back to original queue
// // //     memcpy(ready_queue->data, temp_queue->data, temp_queue->capacity * sizeof(Process*));
// // //     ready_queue->front = 0;
// // //     ready_queue->rear = temp_queue->size - 1;
// // //     ready_queue->size = temp_queue->size;
    
// // //     // Free temporary queue
// // //     free(temp_queue->data);
// // //     free(temp_queue);
    
// // //     // Calculate active processes
// // //     int swapped_out_count = 0; // This will be handled by caller
// // //     int active = n - swapped_out_count - completed_count;
    
// // //     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// // // }


// // // int main() {
// // //     FILE *fin = fopen("search.txt", "r");
// // //     if (!fin) {
// // //         printf("Error: Could not open search.txt\n");
// // //         return 1;
// // //     }

// // //     // Read number of processes and searches
// // //     fscanf(fin, "%d %d", &n, &m);
    
// // //     // Allocate memory for processes and page tables
// // //     processes = (Process*)malloc(n * sizeof(Process));
// // //     page_tables = (PageTable*)malloc(n * sizeof(PageTable));
    
// // //     // Create queues
// // //     Queue *ready_queue = create_queue(n);
// // //     Queue *swapped_out = create_queue(n);

// // //     printf("+++ Simulation data read from file\n");
    
// // //     // Initialize processes
// // //     for (int i = 0; i < n; i++) {
// // //         fscanf(fin, "%d", &processes[i].max_A_size);
        
// // //         processes[i].pid = i;
// // //         processes[i].cur_search_idx = 0;
// // //         processes[i].is_swapped = 0;
// // //         processes[i].addi_data_seg = 0;
        
// // //         // Load essential pages
// // //         for (int j = 0; j < ESSENTIAL_PAGES; j++) {
// // //             page_set(page_tables, i, j);
// // //         }
        
// // //         // Read search keys
// // //         for (int j = 0; j < m; j++) {
// // //             fscanf(fin, "%d", &processes[i].searches[j]);
// // //         }
        
// // //         // Add to ready queue
// // //         enqueue(ready_queue, &processes[i]);
// // //     }
// // //     fclose(fin);

// // //     // Set initial multiprogramming to number of processes
// // //     multi_prog = n;
// // //     printf("+++ Kernel data initialized\n");

// // //     // Main simulation loop
// // //     // while (ready_queue->size > 0 || swapped_out->size > 0) {
// // //     //     // Handle case when ready queue is empty
// // //     //     if (ready_queue->size == 0 && swapped_out->size > 0) {
// // //     //         Process *proc = dequeue(swapped_out);
// // //     //         if (available_frames >= ESSENTIAL_PAGES) {
// // //     //             swap_in(proc, ready_queue);
// // //     //         } else {
// // //     //             enqueue(swapped_out, proc);
// // //     //             break;
// // //     //         }
// // //     //         continue;
// // //     //     }
// // //     while (ready_queue->size > 0 || swapped_out->size > 0) {
// // //         // Handle case when ready queue is empty
// // //         if (ready_queue->size == 0 && swapped_out->size > 0) {
// // //             Process *proc = dequeue(swapped_out);
// // //             if (available_frames >= ESSENTIAL_PAGES) {
// // //                 swap_in(proc, ready_queue);
// // //             } else {
// // //                 enqueue(swapped_out, proc);
// // //                 break;
// // //             }
// // //             continue;
// // //         }
// // //         // Get next process
// // //         Process *current = dequeue(ready_queue);
        
// // //         // Check if process has completed all searches
// // //         if (current->cur_search_idx >= m) {
// // //             completed_count++;
            
// // //             // Clear all page table entries
// // //             for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// // //                 page_clear(page_tables, current->pid, i);
// // //             }
// // //             current->addi_data_seg = 0;
            
// // //             // Try to swap in a new process
// // //             if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
// // //                 Process *next = dequeue(swapped_out);
// // //                 swap_in(next, ready_queue);
// // //             }
// // //             continue;
// // //         }

// // //         // Perform binary search
// // //         int key = current->searches[current->cur_search_idx];
// // //         int l = 0, r = current->max_A_size - 1;
// // //         int swapped_out_flag = 0;

// // //         while (l < r && !swapped_out_flag) {
// // //             int m = (l + r) / 2;
// // //             tot_page_accs++;
            
// // //             // Determine page for current index
// // //             int page = 10 + (m / 1024);
            
// // //             // Check page validity
// // //             if (!page_retrieve(page_tables, current->pid, page)) {
// // //                 tot_page_faults++;
                
// // //                 // Swap out if no frames available
// // //                 if (available_frames <= 0) {
// // //                     swap_out(current, ready_queue, swapped_out);
// // //                     swapped_out_flag = 1;
// // //                     break;
// // //                 }
                
// // //                 // Load page
// // //                 page_set(page_tables, current->pid, page);
// // //             }
            
// // //             // Binary search logic
// // //             if (key <= m) r = m;
// // //             else l = m + 1;
// // //         }

// // //         // Handle process after binary search
// // //         if (!swapped_out_flag) {
// // //             current->cur_search_idx++;
            
// // //             if (current->cur_search_idx < m) {
// // //                 // More searches left, add back to queue
// // //                 enqueue(ready_queue, current);
// // //             } else {
// // //                 // Process completed
// // //                 completed_count++;
                
// // //                 // Clear page table
// // //                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// // //                     page_clear(page_tables, current->pid, i);
// // //                 }
// // //                 current->addi_data_seg = 0;
                
// // //                 // Try to swap in a new process
// // //                 if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
// // //                     Process *next = dequeue(swapped_out);
// // //                     swap_in(next, ready_queue);
// // //                 }
// // //             }
// // //         }
// // //     }

// // //     // Print simulation results
// // //     printf("+++ Page access summary\n");
// // //     printf("Total number of page accesses = %d\n", tot_page_accs);
// // //     printf("Total number of page faults = %d\n", tot_page_faults);
// // //     printf("Total number of swaps = %d\n", tot_swaps);
// // //     printf("Degree of multiprogramming = %d\n", multi_prog);

// // //     // Clean up
// // //     free(processes);
// // //     free(page_tables);
// // //     free(ready_queue->data);
// // //     free(ready_queue);
// // //     free(swapped_out->data);
// // //     free(swapped_out);

// // //     return 0;
// // // }



// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// // #include <limits.h>

// // #define TOTAL_FRAMES 16384      // Total memory frames (64 MB / 4 KB)
// // #define USER_FRAMES 12288       // User-available frames (48 MB / 4 KB)
// // #define ESSENTIAL_PAGES 10      // Essential pages per process
// // #define PAGE_SIZE 4096          // Page size in bytes
// // #define INTS_PER_PAGE 1024      // Integers per page (4096 / 4)
// // #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// // #define MAX_PROCESSES 500       // Max n
// // #define MAX_SEARCHES 100        // Max m

// // // Process structure
// // typedef struct {
// //     int pid;                    // Process ID
// //     int addi_data_seg;          // Number of additional data segment pages loaded
// //     int cur_search_idx;         // Current search index
// //     int max_A_size;             // Size of array A in integers
// //     int searches[MAX_SEARCHES]; // Search keys
// //     int is_swapped;             // 1 if swapped out, 0 if active
// // } Process;

// // // Page table structure
// // typedef struct {
// //     unsigned short entries[PAGE_TABLE_SIZE]; // 16-bit page table entries
// // } PageTable;

// // // Global variables
// // Process *processes;             // Dynamic array of processes
// // PageTable *page_tables;         // Dynamic array of page tables
// // int n, m;                       // Number of processes and searches
// // int tot_page_accs = 0;          // Total page accesses
// // int tot_page_faults = 0;        // Total page faults
// // int tot_swaps = 0;              // Total swaps
// // int available_frames = USER_FRAMES; // Free frames available
// // int multi_prog = INT_MAX;       // Degree of multiprogramming
// // int completed_count = 0;        // Number of completed processes

// // // Custom queue structure
// // typedef struct {
// //     Process **data;
// //     int front, rear, size, capacity;
// // } Queue;

// // // Create a new queue
// // Queue* create_queue(int capacity) {
// //     Queue *q = (Queue*)malloc(sizeof(Queue));
// //     q->data = (Process**)malloc(capacity * sizeof(Process*));
// //     q->front = q->rear = -1;
// //     q->size = 0;
// //     q->capacity = capacity;
// //     return q;
// // }

// // // Enqueue to queue (FIFO)
// // void enqueue(Queue *q, Process *item) {
// //     if (q->size == q->capacity) return;
// //     if (q->front == -1) q->front = 0;
// //     q->rear = (q->rear + 1) % q->capacity;
// //     q->data[q->rear] = item;
// //     q->size++;
// // }

// // // Dequeue from queue (FIFO)
// // Process* dequeue(Queue *q) {
// //     if (q->size == 0) return NULL;
// //     Process *item = q->data[q->front];
// //     if (q->size == 1) {
// //         q->front = q->rear = -1;
// //     } else {
// //         q->front = (q->front + 1) % q->capacity;
// //     }
// //     q->size--;
// //     return item;
// // }

// // // Page table management functions
// // void page_set(PageTable *page_tables, int process_id, int entry) {
// //     if (!(page_tables[process_id].entries[entry] & (1 << 15))) {
// //         page_tables[process_id].entries[entry] |= (1 << 15);
// //         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg++;
// //         available_frames--;
// //     }
// // }

// // void page_clear(PageTable *page_tables, int process_id, int entry) {
// //     if (page_tables[process_id].entries[entry] & (1 << 15)) {
// //         page_tables[process_id].entries[entry] &= ~(1 << 15);
// //         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg--;
// //         available_frames++;
// //     }
// // }

// // int page_retrieve(PageTable *page_tables, int process_id, int entry) {
// //     return page_tables[process_id].entries[entry] >> 15;
// // }

// // // Swap out a process
// // void swap_out(Process *proc, Queue *ready_queue, Queue *swapped_out) {
// //     proc->is_swapped = 1;

// //     // Clear all page table entries
// //     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// //         if (page_retrieve(page_tables, proc->pid, i)) {
// //             page_clear(page_tables, proc->pid, i);
// //         }
// //     }
// //     proc->addi_data_seg = 0;

// //     // Enqueue to swapped out queue
// //     enqueue(swapped_out, proc);

// //     // Calculate active processes
// //     int active = n - swapped_out->size - completed_count;
// //     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);

// //     // Update total swaps and minimum multiprogramming degree
// //     tot_swaps++;
// //     if (active < multi_prog) multi_prog = active;
// // }

// // // Swap in a process
// // void swap_in(Process *proc, Queue *ready_queue, Queue *swapped_out) {
// //     proc->is_swapped = 0;

// //     // Load essential pages
// //     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
// //         page_set(page_tables, proc->pid, i);
// //     }

// //     // Create a temporary queue to rebuild ready_queue with proc at front
// //     Queue *temp_queue = create_queue(ready_queue->capacity);
// //     enqueue(temp_queue, proc); // Swapped-in process goes first

// //     // Add all existing ready queue processes in their original order
// //     while (ready_queue->size > 0) {
// //         Process *temp = dequeue(ready_queue);
// //         enqueue(temp_queue, temp);
// //     }

// //     // Copy back to ready_queue
// //     memcpy(ready_queue->data, temp_queue->data, temp_queue->capacity * sizeof(Process*));
// //     ready_queue->front = 0;
// //     ready_queue->rear = temp_queue->size - 1;
// //     ready_queue->size = temp_queue->size;

// //     // Free temporary queue
// //     free(temp_queue->data);
// //     free(temp_queue);

// //     // Calculate active processes
// //     int active = n - completed_count - swapped_out->size;
// //     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// // }

// // int main() {
// //     FILE *fin = fopen("search.txt", "r");
// //     if (!fin) {
// //         printf("Error: Could not open search.txt\n");
// //         return 1;
// //     }

// //     // Read number of processes and searches
// //     fscanf(fin, "%d %d", &n, &m);

// //     // Allocate memory for processes and page tables
// //     processes = (Process*)malloc(n * sizeof(Process));
// //     page_tables = (PageTable*)malloc(n * sizeof(PageTable));
// //     memset(page_tables, 0, n * sizeof(PageTable)); // Initialize page tables to 0

// //     // Create queues
// //     Queue *ready_queue = create_queue(n);
// //     Queue *swapped_out = create_queue(n);

// //     printf("+++ Simulation data read from file\n");

// //     // Initialize processes
// //     for (int i = 0; i < n; i++) {
// //         fscanf(fin, "%d", &processes[i].max_A_size);
// //         processes[i].pid = i;
// //         processes[i].cur_search_idx = 0;
// //         processes[i].is_swapped = 0;
// //         processes[i].addi_data_seg = 0;

// //         // Load essential pages
// //         for (int j = 0; j < ESSENTIAL_PAGES; j++) {
// //             page_set(page_tables, i, j);
// //         }

// //         // Read search keys
// //         for (int j = 0; j < m; j++) {
// //             fscanf(fin, "%d", &processes[i].searches[j]);
// //         }

// //         // Add to ready queue in order (0 to n-1)
// //         enqueue(ready_queue, &processes[i]);
// //     }
// //     fclose(fin);

// //     // Set initial multiprogramming to number of processes
// //     multi_prog = n;
// //     printf("+++ Kernel data initialized\n");

// //     // Main simulation loop
// //     while (ready_queue->size > 0 || swapped_out->size > 0) {
// //         // If ready queue is empty, try to swap in a process
// //         if (ready_queue->size == 0 && swapped_out->size > 0) {
// //             Process *proc = dequeue(swapped_out);
// //             if (available_frames >= ESSENTIAL_PAGES) {
// //                 swap_in(proc, ready_queue, swapped_out);
// //             } else {
// //                 enqueue(swapped_out, proc); // Put it back if no frames available
// //                 break;
// //             }
// //             continue;
// //         }

// //         // Get next process from ready queue (round-robin)
// //         Process *current = dequeue(ready_queue);

// //         // Check if process has completed all searches
// //         if (current->cur_search_idx >= m) {
// //             completed_count++;
// //             for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// //                 page_clear(page_tables, current->pid, i);
// //             }
// //             current->addi_data_seg = 0;

// //             // Try to swap in a process if possible
// //             if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
// //                 Process *next = dequeue(swapped_out);
// //                 swap_in(next, ready_queue, swapped_out);
// //             }
// //             continue;
// //         }

// //         // Perform binary search
// //         int key = current->searches[current->cur_search_idx];
// //         int l = 0, r = current->max_A_size - 1;
// //         int swapped_out_flag = 0;

// //         while (l < r && !swapped_out_flag) {
// //             int m = (l + r) / 2;
// //             tot_page_accs++;

// //             // Determine page for current index
// //             int page = 10 + (m / 1024);

// //             // Check page validity
// //             if (!page_retrieve(page_tables, current->pid, page)) {
// //                 tot_page_faults++;
// //                 if (available_frames <= 0) {
// //                     swap_out(current, ready_queue, swapped_out);
// //                     swapped_out_flag = 1;
// //                     break;
// //                 }
// //                 page_set(page_tables, current->pid, page);
// //             }

// //             // Binary search logic
// //             if (key <= m) r = m;
// //             else l = m + 1;
// //         }

// //         // Handle process after binary search
// //         if (!swapped_out_flag) {
// //             current->cur_search_idx++;
// //             if (current->cur_search_idx < m) {
// //                 enqueue(ready_queue, current); // Back to ready queue
// //             } else {
// //                 completed_count++;
// //                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
// //                     page_clear(page_tables, current->pid, i);
// //                 }
// //                 current->addi_data_seg = 0;
// //                 if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
// //                     Process *next = dequeue(swapped_out);
// //                     swap_in(next, ready_queue, swapped_out);
// //                 }
// //             }
// //         }
// //     }

// //     // Print simulation results
// //     printf("+++ Page access summary\n");
// //     printf("Total number of page accesses = %d\n", tot_page_accs);
// //     printf("Total number of page faults = %d\n", tot_page_faults);
// //     printf("Total number of swaps = %d\n", tot_swaps);
// //     printf("Degree of multiprogramming = %d\n", multi_prog);

// //     // Clean up
// //     free(processes);
// //     free(page_tables);
// //     free(ready_queue->data);
// //     free(ready_queue);
// //     free(swapped_out->data);
// //     free(swapped_out);

// //     return 0;
// // }


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <limits.h>

// #define TOTAL_FRAMES 16384      // Total memory frames (64 MB / 4 KB)
// #define USER_FRAMES 12288       // User-available frames (48 MB / 4 KB)
// #define ESSENTIAL_PAGES 10      // Essential pages per process
// #define PAGE_SIZE 4096          // Page size in bytes
// #define INTS_PER_PAGE 1024      // Integers per page (4096 / 4)
// #define PAGE_TABLE_SIZE 2048    // Virtual memory pages per process
// #define MAX_PROCESSES 500       // Max n
// #define MAX_SEARCHES 100        // Max m

// // Process structure
// typedef struct {
//     int pid;                    // Process ID
//     int addi_data_seg;          // Number of additional data segment pages loaded
//     int cur_search_idx;         // Current search index
//     int max_A_size;             // Size of array A in integers
//     int searches[MAX_SEARCHES]; // Search keys
//     int is_swapped;             // 1 if swapped out, 0 if active
// } Process;

// // Page table structure
// typedef struct {
//     unsigned short entries[PAGE_TABLE_SIZE]; // 16-bit page table entries
// } PageTable;

// // Global variables
// Process *processes;             // Dynamic array of processes
// PageTable *page_tables;         // Dynamic array of page tables
// int n, m;                       // Number of processes and searches
// int tot_page_accs = 0;          // Total page accesses
// int tot_page_faults = 0;        // Total page faults
// int tot_swaps = 0;              // Total swaps
// int available_frames = USER_FRAMES; // Free frames available
// int multi_prog = INT_MAX;       // Degree of multiprogramming
// int completed_count = 0;        // Number of completed processes

// // Custom queue structure
// typedef struct {
//     Process **data;
//     int front, rear, size, capacity;
// } Queue;

// // Create a new queue
// Queue* create_queue(int capacity) {
//     Queue *q = (Queue*)malloc(sizeof(Queue));
//     q->data = (Process**)malloc(capacity * sizeof(Process*));
//     q->front = q->rear = -1;
//     q->size = 0;
//     q->capacity = capacity;
//     return q;
// }

// // Enqueue to queue (FIFO)
// void enqueue(Queue *q, Process *item) {
//     if (q->size == q->capacity) return;
//     if (q->front == -1) q->front = 0;
//     q->rear = (q->rear + 1) % q->capacity;
//     q->data[q->rear] = item;
//     q->size++;
// }

// // Dequeue from queue (FIFO)
// Process* dequeue(Queue *q) {
//     if (q->size == 0) return NULL;
//     Process *item = q->data[q->front];
//     if (q->size == 1) {
//         q->front = q->rear = -1;
//     } else {
//         q->front = (q->front + 1) % q->capacity;
//     }
//     q->size--;
//     return item;
// }

// // Page table management functions
// void page_set(PageTable *page_tables, int process_id, int entry) {
//     if (!(page_tables[process_id].entries[entry] & (1 << 15))) {
//         page_tables[process_id].entries[entry] |= (1 << 15);
//         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg++;
//         available_frames--;
//     }
// }

// void page_clear(PageTable *page_tables, int process_id, int entry) {
//     if (page_tables[process_id].entries[entry] & (1 << 15)) {
//         page_tables[process_id].entries[entry] &= ~(1 << 15);
//         if (entry >= ESSENTIAL_PAGES) processes[process_id].addi_data_seg--;
//         available_frames++;
//     }
// }

// int page_retrieve(PageTable *page_tables, int process_id, int entry) {
//     return page_tables[process_id].entries[entry] >> 15;
// }

// // Swap out a process
// void swap_out(Process *proc, Queue *ready_queue, Queue *swapped_out) {
//     proc->is_swapped = 1;

//     // Clear all page table entries
//     for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//         if (page_retrieve(page_tables, proc->pid, i)) {
//             page_clear(page_tables, proc->pid, i);
//         }
//     }
//     proc->addi_data_seg = 0;

//     // Enqueue to swapped out queue
//     enqueue(swapped_out, proc);

//     // Calculate active processes
//     int active = n - swapped_out->size - completed_count;
//     printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);

//     // Update total swaps and minimum multiprogramming degree
//     tot_swaps++;
//     if (active < multi_prog) multi_prog = active;
// }

// // Swap in a process
// void swap_in(Process *proc, Queue *ready_queue, Queue *swapped_out) {
//     proc->is_swapped = 0;

//     // Load essential pages
//     for (int i = 0; i < ESSENTIAL_PAGES; i++) {
//         page_set(page_tables, proc->pid, i);
//     }

//     // Create a temporary queue to rebuild ready_queue with proc at front
//     Queue *temp_queue = create_queue(ready_queue->capacity);
//     enqueue(temp_queue, proc); // Swapped-in process goes first

//     // Add all existing ready queue processes in their original order
//     while (ready_queue->size > 0) {
//         Process *temp = dequeue(ready_queue);
//         enqueue(temp_queue, temp);
//     }

//     // Copy back to ready_queue
//     memcpy(ready_queue->data, temp_queue->data, temp_queue->capacity * sizeof(Process*));
//     ready_queue->front = 0;
//     ready_queue->rear = temp_queue->size - 1;
//     ready_queue->size = temp_queue->size;

//     // Free temporary queue
//     free(temp_queue->data);
//     free(temp_queue);

//     // Calculate active processes
//     int active = n - completed_count - swapped_out->size;
//     printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
// }

// int main() {
//     FILE *fin = fopen("search.txt", "r");
//     if (!fin) {
//         printf("088 Error: Could not open search.txt\n");
//         return 1;
//     }

//     // Read number of processes and searches
//     fscanf(fin, "%d %d", &n, &m);

//     // Allocate memory for processes and page tables
//     processes = (Process*)malloc(n * sizeof(Process));
//     page_tables = (PageTable*)malloc(n * sizeof(PageTable));
//     memset(page_tables, 0, n * sizeof(PageTable)); // Initialize page tables to 0

//     // Create queues
//     Queue *ready_queue = create_queue(n);
//     Queue *swapped_out = create_queue(n);

//     printf("+++ Simulation data read from file\n");

//     // Initialize processes
//     for (int i = 0; i < n; i++) {
//         fscanf(fin, "%d", &processes[i].max_A_size);
//         processes[i].pid = i;
//         processes[i].cur_search_idx = 0;
//         processes[i].is_swapped = 0;
//         processes[i].addi_data_seg = 0;

//         // Load essential pages
//         for (int j = 0; j < ESSENTIAL_PAGES; j++) {
//             page_set(page_tables, i, j);
//         }

//         // Read search keys
//         for (int j = 0; j < m; j++) {
//             fscanf(fin, "%d", &processes[i].searches[j]);
//         }

//         // Add to ready queue in order (0 to n-1)
//         enqueue(ready_queue, &processes[i]);
//     }
//     fclose(fin);

//     // Set initial multiprogramming to number of processes
//     multi_prog = n;
//     printf("+++ Kernel data initialized\n");

//     // Main simulation loop
//     while (ready_queue->size > 0 || swapped_out->size > 0) {
//         // If ready queue is empty, try to swap in a process
//         if (ready_queue->size == 0 && swapped_out->size > 0) {
//             Process *proc = dequeue(swapped_out);
//             if (available_frames >= ESSENTIAL_PAGES) {
//                 swap_in(proc, ready_queue, swapped_out);
//             } else {
//                 enqueue(swapped_out, proc); // Put it back if no frames available
//                 break;
//             }
//             continue;
//         }

//         // Get next process from ready queue (round-robin)
//         Process *current = dequeue(ready_queue);

//         // Check if process has completed all searches
//         if (current->cur_search_idx >= m) {
//             completed_count++;
//             for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                 page_clear(page_tables, current->pid, i);
//             }
//             current->addi_data_seg = 0;

//             // Try to swap in a process if possible
//             if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
//                 Process *next = dequeue(swapped_out);
//                 swap_in(next, ready_queue, swapped_out);
//             }
//             continue;
//         }

//         // Perform binary search
//         int key = current->searches[current->cur_search_idx];
//         int l = 0, r = current->max_A_size - 1;
//         int swapped_out_flag = 0;

// #ifdef VERBOSE
//         printf("\tSearch %d by Process %d\n", current->cur_search_idx + 1, current->pid);
// #endif

//         while (l < r && !swapped_out_flag) {
//             int m = (l + r) / 2;
//             tot_page_accs++;

//             // Determine page for current index
//             int page = 10 + (m / 1024);

//             // Check page validity
//             if (!page_retrieve(page_tables, current->pid, page)) {
//                 tot_page_faults++;
//                 if (available_frames <= 0) {
//                     swap_out(current, ready_queue, swapped_out);
//                     swapped_out_flag = 1;
//                     break;
//                 }
//                 page_set(page_tables, current->pid, page);
//             }

//             // Binary search logic
//             if (key <= m) r = m;
//             else l = m + 1;
//         }

//         // Handle process after binary search
//         if (!swapped_out_flag) {
//             current->cur_search_idx++;
//             if (current->cur_search_idx < m) {
//                 enqueue(ready_queue, current); // Back to ready queue
//             } else {
//                 completed_count++;
//                 for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
//                     page_clear(page_tables, current->pid, i);
//                 }
//                 current->addi_data_seg = 0;
//                 if (swapped_out->size > 0 && available_frames >= ESSENTIAL_PAGES) {
//                     Process *next = dequeue(swapped_out);
//                     swap_in(next, ready_queue, swapped_out);
//                 }
//             }
//         }
//     }

//     // Print simulation results
//     printf("+++ Page access summary\n");
//     printf("Total number of page accesses = %d\n", tot_page_accs);
//     printf("Total number of page faults = %d\n", tot_page_faults);
//     printf("Total number of swaps = %d\n", tot_swaps);
//     printf("Degree of multiprogramming = %d\n", multi_prog);

//     // Clean up
//     free(processes);
//     free(page_tables);
//     free(ready_queue->data);
//     free(ready_queue);
//     free(swapped_out->data);
//     free(swapped_out);

//     return 0;
// }