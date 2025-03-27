

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TOTAL_MEMORY_FRAMES 16384  // Total memory frames (64 MB / 4 KB)
#define USER_MEMORY_FRAMES 12288   // User-available frames (48 MB / 4 KB)
#define ESSENTIAL_SEGMENT_PAGES 10 // Essential pages per process
#define PAGE_SIZE_BYTES 4096       // Page size in bytes
#define INTEGERS_PER_PAGE 1024     // Integers per page (4096 / 4)
#define VIRTUAL_PAGES_PER_PROCESS 2048 // Virtual memory pages per process
#define MAX_PROCESS_COUNT 500      // Max number of processes
#define MAX_SEARCH_COUNT 100       // Max number of searches

// Structure for a process
typedef struct {
    int process_id;              // Process ID
    int additional_pages_loaded; // Number of additional data segment pages loaded
    int current_search_number;   // Current search index
    int array_size;              // Size of array A in integers
    int search_keys[MAX_SEARCH_COUNT]; // Search keys
    int is_swapped_out;          // 1 if swapped out, 0 if active
} SystemProcess;

// Structure for a page table
typedef struct {
    unsigned short page_entries[VIRTUAL_PAGES_PER_PROCESS]; // 16-bit page table entries
} ProcessPageTable;

// Global variables
SystemProcess *system_processes;         // Array of processes
ProcessPageTable *process_page_tables;   // Array of page tables
int total_processes, total_searches;     // Number of processes and searches
int total_page_accesses = 0;             // Total page accesses
int total_page_misses = 0;               // Total page faults
int total_swap_operations = 0;           // Total swaps
int free_frame_count = USER_MEMORY_FRAMES; // Free frames available
int min_active_processes = INT_MAX;      // Degree of multiprogramming
int finished_process_count = 0;          // Number of completed processes

// Custom queue structure for process management
typedef struct {
    SystemProcess **process_list;
    int queue_front, queue_rear, queue_size, queue_capacity;
} ProcessQueue;

// Create a new queue
ProcessQueue* initialize_queue(int capacity) {
    ProcessQueue *queue = (ProcessQueue*)malloc(sizeof(ProcessQueue));
    queue->process_list = (SystemProcess**)malloc(capacity * sizeof(SystemProcess*));
    queue->queue_front = queue->queue_rear = -1;
    queue->queue_size = 0;
    queue->queue_capacity = capacity;
    return queue;
}

// Add process to queue (FIFO)
void add_to_queue(ProcessQueue *queue, SystemProcess *proc) {
    if (queue->queue_size == queue->queue_capacity) return;
    if (queue->queue_front == -1) queue->queue_front = 0;
    queue->queue_rear = (queue->queue_rear + 1) % queue->queue_capacity;
    queue->process_list[queue->queue_rear] = proc;
    queue->queue_size++;
}

// Remove process from queue (FIFO)
SystemProcess* remove_from_queue(ProcessQueue *queue) {
    if (queue->queue_size == 0) return NULL;
    SystemProcess *proc = queue->process_list[queue->queue_front];
    if (queue->queue_size == 1) {
        queue->queue_front = queue->queue_rear = -1;
    } else {
        queue->queue_front = (queue->queue_front + 1) % queue->queue_capacity;
    }
    queue->queue_size--;
    return proc;
}

// Page table management functions
void set_page_valid(ProcessPageTable *page_tables, int proc_id, int page_index) {
    if (!(page_tables[proc_id].page_entries[page_index] & (1 << 15))) {
        page_tables[proc_id].page_entries[page_index] |= (1 << 15);
        if (page_index >= ESSENTIAL_SEGMENT_PAGES) system_processes[proc_id].additional_pages_loaded++;
        free_frame_count--;
    }
}

void clear_page_valid(ProcessPageTable *page_tables, int proc_id, int page_index) {
    if (page_tables[proc_id].page_entries[page_index] & (1 << 15)) {
        page_tables[proc_id].page_entries[page_index] &= ~(1 << 15);
        if (page_index >= ESSENTIAL_SEGMENT_PAGES) system_processes[proc_id].additional_pages_loaded--;
        free_frame_count++;
    }
}

int is_page_valid(ProcessPageTable *page_tables, int proc_id, int page_index) {
    return page_tables[proc_id].page_entries[page_index] >> 15;
}

// Swap out a process
void perform_swap_out(SystemProcess *proc, ProcessQueue *ready_queue, ProcessQueue *swapped_queue) {
    proc->is_swapped_out = 1;

    // Clear all page table entries
    for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++) {
        if (is_page_valid(process_page_tables, proc->process_id, i)) {
            clear_page_valid(process_page_tables, proc->process_id, i);
        }
    }
    proc->additional_pages_loaded = 0;

    // Add to swapped-out queue
    add_to_queue(swapped_queue, proc);

    // Calculate active processes
    int active_process_count = total_processes - swapped_queue->queue_size - finished_process_count;
    printf("+++ Swapping out process %3d [%d active processes]\n", proc->process_id, active_process_count);

    // Update swap count and minimum active processes
    total_swap_operations++;
    if (active_process_count < min_active_processes) min_active_processes = active_process_count;
}

// Swap in a process
void perform_swap_in(SystemProcess *proc, ProcessQueue *ready_queue, ProcessQueue *swapped_queue) {
    proc->is_swapped_out = 0;

    // Load essential segment pages
    for (int i = 0; i < ESSENTIAL_SEGMENT_PAGES; i++) {
        set_page_valid(process_page_tables, proc->process_id, i);
    }

    // Rebuild ready queue with the swapped-in process at the front
    ProcessQueue *temp_queue = initialize_queue(ready_queue->queue_capacity);
    add_to_queue(temp_queue, proc); // Swapped-in process goes first

    // Preserve order of existing ready queue processes
    while (ready_queue->queue_size > 0) {
        SystemProcess *temp_proc = remove_from_queue(ready_queue);
        add_to_queue(temp_queue, temp_proc);
    }

    // Copy back to ready queue
    memcpy(ready_queue->process_list, temp_queue->process_list, temp_queue->queue_capacity * sizeof(SystemProcess*));
    ready_queue->queue_front = 0;
    ready_queue->queue_rear = temp_queue->queue_size - 1;
    ready_queue->queue_size = temp_queue->queue_size;

    // Free temporary queue
    free(temp_queue->process_list);
    free(temp_queue);

    // Calculate active processes
    int active_process_count = total_processes - finished_process_count - swapped_queue->queue_size;
    printf("+++ Swapping in process %3d [%d active processes]\n", proc->process_id, active_process_count);
}

int main() {
    FILE *input_file = fopen("search.txt", "r");
    if (!input_file) {
        printf("088 Error: Could not open search.txt\n");
        return 1;
    }

    // Read number of processes and searches
    fscanf(input_file, "%d %d", &total_processes, &total_searches);

    // Allocate memory for processes and page tables
    system_processes = (SystemProcess*)malloc(total_processes * sizeof(SystemProcess));
    process_page_tables = (ProcessPageTable*)malloc(total_processes * sizeof(ProcessPageTable));
    memset(process_page_tables, 0, total_processes * sizeof(ProcessPageTable)); // Initialize page tables

    // Create queues
    ProcessQueue *active_process_queue = initialize_queue(total_processes);
    ProcessQueue *swapped_out_queue = initialize_queue(total_processes);

    printf("+++ Simulation data read from file\n");

    // Initialize processes
    for (int i = 0; i < total_processes; i++) {
        fscanf(input_file, "%d", &system_processes[i].array_size);
        system_processes[i].process_id = i;
        system_processes[i].current_search_number = 0;
        system_processes[i].is_swapped_out = 0;
        system_processes[i].additional_pages_loaded = 0;

        // Load essential segment pages
        for (int j = 0; j < ESSENTIAL_SEGMENT_PAGES; j++) {
            set_page_valid(process_page_tables, i, j);
        }

        // Read search keys
        for (int j = 0; j < total_searches; j++) {
            fscanf(input_file, "%d", &system_processes[i].search_keys[j]);
        }

        // Add to active process queue in order (0 to n-1)
        add_to_queue(active_process_queue, &system_processes[i]);
    }
    fclose(input_file);

    // Set initial degree of multiprogramming
    min_active_processes = total_processes;
    printf("+++ Kernel data initialized\n");

    // Main simulation loop
    while (active_process_queue->queue_size > 0 || swapped_out_queue->queue_size > 0) {
        // Handle empty active queue by swapping in a process
        if (active_process_queue->queue_size == 0 && swapped_out_queue->queue_size > 0) {
            SystemProcess *proc = remove_from_queue(swapped_out_queue);
            if (free_frame_count >= ESSENTIAL_SEGMENT_PAGES) {
                perform_swap_in(proc, active_process_queue, swapped_out_queue);
            } else {
                add_to_queue(swapped_out_queue, proc); // Requeue if no frames available
                break;
            }
            continue;
        }

        // Get next process from active queue (round-robin)
        SystemProcess *current_process = remove_from_queue(active_process_queue);

        // Check if process has completed all searches
        if (current_process->current_search_number >= total_searches) {
            finished_process_count++;
            for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++) {
                clear_page_valid(process_page_tables, current_process->process_id, i);
            }
            current_process->additional_pages_loaded = 0;

            // Swap in a process if possible
            if (swapped_out_queue->queue_size > 0 && free_frame_count >= ESSENTIAL_SEGMENT_PAGES) {
                SystemProcess *next_proc = remove_from_queue(swapped_out_queue);
                perform_swap_in(next_proc, active_process_queue, swapped_out_queue);
            }
            continue;
        }

        // Perform binary search
        int search_key = current_process->search_keys[current_process->current_search_number];
        int left = 0, right = current_process->array_size - 1;
        int swap_out_triggered = 0;

#ifdef VERBOSE
        printf("\tSearch %d by Process %d\n", current_process->current_search_number + 1, current_process->process_id);
#endif

        while (left < right && !swap_out_triggered) {
            int mid = (left + right) / 2;
            total_page_accesses++;

            // Determine page for current index
            int page_index = 10 + (mid / INTEGERS_PER_PAGE);

            // Check page validity
            if (!is_page_valid(process_page_tables, current_process->process_id, page_index)) {
                total_page_misses++;
                if (free_frame_count <= 0) {
                    perform_swap_out(current_process, active_process_queue, swapped_out_queue);
                    swap_out_triggered = 1;
                    break;
                }
                set_page_valid(process_page_tables, current_process->process_id, page_index);
            }

            // Binary search logic
            if (search_key <= mid) right = mid;
            else left = mid + 1;
        }

        // Handle process after binary search
        if (!swap_out_triggered) {
            current_process->current_search_number++;
            if (current_process->current_search_number < total_searches) {
                add_to_queue(active_process_queue, current_process); // Back to active queue
            } else {
                finished_process_count++;
                for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++) {
                    clear_page_valid(process_page_tables, current_process->process_id, i);
                }
                current_process->additional_pages_loaded = 0;
                if (swapped_out_queue->queue_size > 0 && free_frame_count >= ESSENTIAL_SEGMENT_PAGES) {
                    SystemProcess *next_proc = remove_from_queue(swapped_out_queue);
                    perform_swap_in(next_proc, active_process_queue, swapped_out_queue);
                }
            }
        }
    }

    // Print simulation results
    printf("+++ Page access summary\n");
    printf("Total number of page accesses = %d\n", total_page_accesses);
    printf("Total number of page faults = %d\n", total_page_misses);
    printf("Total number of swaps = %d\n", total_swap_operations);
    printf("Degree of multiprogramming = %d\n", min_active_processes);

    // Clean up
    free(system_processes);
    free(process_page_tables);
    free(active_process_queue->process_list);
    free(active_process_queue);
    free(swapped_out_queue->process_list);
    free(swapped_out_queue);

    return 0;
}