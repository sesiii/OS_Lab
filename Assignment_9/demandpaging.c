#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TOTAL_MEMORY_FRAMES 16384
#define USER_MEMORY_FRAMES 12288
#define ESSENTIAL_SEGMENT_PAGES 10
#define PAGE_SIZE_BYTES 4096
#define INTEGERS_PER_PAGE 1024
#define VIRTUAL_PAGES_PER_PROCESS 2048
#define MAX_PROCESS_COUNT 500
#define MAX_SEARCH_COUNT 100

// Process structure
typedef struct {
    int process_id;              
    int additional_pages_loaded; 
    int current_search_number;   
    int array_size;              
    int search_keys[MAX_SEARCH_COUNT]; 
    int is_swapped_out;          
} SystemProcess;

//pagetable
typedef struct {
    unsigned short page_entries[VIRTUAL_PAGES_PER_PROCESS]; 
}pagetable;

SystemProcess *system_processes;
pagetable *process_page_tables;
int total_processes, total_searches;
int total_page_accesses = 0;
int total_page_misses = 0;
int total_swap_operations = 0;
int free_frame_count = USER_MEMORY_FRAMES;
int min_active_processes = INT_MAX;
int finished_process_count = 0;

typedef struct {
    SystemProcess **process_list;
    int queue_front, queue_rear, queue_size, queue_capacity;
} ProcessQueue;

// Queue functions
ProcessQueue* initialize_queue(int capacity) {
    ProcessQueue *queue = (ProcessQueue*)malloc(sizeof(ProcessQueue));
    queue->process_list = (SystemProcess**)malloc(capacity * sizeof(SystemProcess*));
    queue->queue_front = queue->queue_rear = -1;
    queue->queue_size = 0;
    queue->queue_capacity = capacity;
    return queue;
}

void add_to_queue(ProcessQueue *queue, SystemProcess *proc) {
    if (queue->queue_size == queue->queue_capacity) return;
    if (queue->queue_front == -1) queue->queue_front = 0;
    queue->queue_rear = (queue->queue_rear + 1) % queue->queue_capacity;
    queue->process_list[queue->queue_rear] = proc;
    queue->queue_size++;
}

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

//valid-invalid bit functions
void set_page_valid(pagetable *page_tables, int proc_id, int page_index) {
    if (!(page_tables[proc_id].page_entries[page_index] & (1 << 15))) {
        page_tables[proc_id].page_entries[page_index] |= (1 << 15);
        if (page_index >= ESSENTIAL_SEGMENT_PAGES) system_processes[proc_id].additional_pages_loaded++;
        free_frame_count--;
    }
}

void clear_page_valid(pagetable *page_tables, int proc_id, int page_index) {
    if (page_tables[proc_id].page_entries[page_index] & (1 << 15)) {
        page_tables[proc_id].page_entries[page_index] &= ~(1 << 15);
        if (page_index >= ESSENTIAL_SEGMENT_PAGES) system_processes[proc_id].additional_pages_loaded--;
        free_frame_count++;
    }
}

int is_page_valid(pagetable *page_tables, int proc_id, int page_index) {
    return page_tables[proc_id].page_entries[page_index] >> 15;
}

//swap out function
void perform_swap_out(SystemProcess *proc, ProcessQueue *ready_queue, ProcessQueue *swapped_queue) {
    proc->is_swapped_out = 1;

    for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++) {
        if (is_page_valid(process_page_tables, proc->process_id, i)) {
            clear_page_valid(process_page_tables, proc->process_id, i);
        }
    }
    proc->additional_pages_loaded = 0;

    add_to_queue(swapped_queue, proc);

    int active_process_count = total_processes - swapped_queue->queue_size - finished_process_count;
    printf("+++ Swapping out process %3d [%d active processes]\n", proc->process_id, active_process_count);

    total_swap_operations++;
    if (active_process_count < min_active_processes) min_active_processes = active_process_count;
}

//swap function
void perform_swap_in(SystemProcess *proc, ProcessQueue *ready_queue, ProcessQueue *swapped_queue) {
    proc->is_swapped_out = 0;

    for (int i = 0; i < ESSENTIAL_SEGMENT_PAGES; i++) {
        set_page_valid(process_page_tables, proc->process_id, i);
    }

    ProcessQueue *temp_queue = initialize_queue(ready_queue->queue_capacity);
    add_to_queue(temp_queue, proc);

    while (ready_queue->queue_size > 0) {
        SystemProcess *temp_proc = remove_from_queue(ready_queue);
        add_to_queue(temp_queue, temp_proc);
    }

    memcpy(ready_queue->process_list, temp_queue->process_list, temp_queue->queue_capacity * sizeof(SystemProcess*));
    ready_queue->queue_front = 0;
    ready_queue->queue_rear = temp_queue->queue_size - 1;
    ready_queue->queue_size = temp_queue->queue_size;

    free(temp_queue->process_list);
    free(temp_queue);

    int active_process_count = total_processes - finished_process_count - swapped_queue->queue_size;
    printf("+++ Swapping in process %3d [%d active processes]\n", proc->process_id, active_process_count);
}

//main function
int main() {
    FILE *input_file = fopen("search.txt", "r");
    if (!input_file) {
        printf("088 Error: Could not open search.txt\n");
        return 1;
    }

    fscanf(input_file, "%d %d", &total_processes, &total_searches);

    system_processes = (SystemProcess*)malloc(total_processes * sizeof(SystemProcess));
    process_page_tables = (pagetable*)malloc(total_processes * sizeof(pagetable));
    memset(process_page_tables, 0, total_processes * sizeof(pagetable));

    ProcessQueue *active_process_queue = initialize_queue(total_processes);
    ProcessQueue *swapped_out_queue = initialize_queue(total_processes);

    printf("+++ Simulation data read from file\n");

    for (int i = 0; i < total_processes; i++) {
        fscanf(input_file, "%d", &system_processes[i].array_size);
        system_processes[i].process_id = i;
        system_processes[i].current_search_number = 0;
        system_processes[i].is_swapped_out = 0;
        system_processes[i].additional_pages_loaded = 0;

        for (int j = 0; j < ESSENTIAL_SEGMENT_PAGES; j++) {
            set_page_valid(process_page_tables, i, j);
        }

        for (int j = 0; j < total_searches; j++) {
            fscanf(input_file, "%d", &system_processes[i].search_keys[j]);
        }

        add_to_queue(active_process_queue, &system_processes[i]);
    }
    fclose(input_file);

    min_active_processes = total_processes;
    printf("+++ Kernel data initialized\n");

    while (active_process_queue->queue_size > 0 || swapped_out_queue->queue_size > 0) {
        if (active_process_queue->queue_size == 0 && swapped_out_queue->queue_size > 0) {
            SystemProcess *proc = remove_from_queue(swapped_out_queue);
            if (free_frame_count >= ESSENTIAL_SEGMENT_PAGES) {
                perform_swap_in(proc, active_process_queue, swapped_out_queue);
            } else {
                add_to_queue(swapped_out_queue, proc);
                break;
            }
            continue;
        }

        SystemProcess *current_process = remove_from_queue(active_process_queue);

        if (current_process->current_search_number >= total_searches) {
            finished_process_count++;
            for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++) {
                clear_page_valid(process_page_tables, current_process->process_id, i);
            }
            current_process->additional_pages_loaded = 0;

            if (swapped_out_queue->queue_size > 0 && free_frame_count >= ESSENTIAL_SEGMENT_PAGES) {
                SystemProcess *next_proc = remove_from_queue(swapped_out_queue);
                perform_swap_in(next_proc, active_process_queue, swapped_out_queue);
            }
            continue;
        }

        int search_key = current_process->search_keys[current_process->current_search_number];
        int left = 0, right = current_process->array_size - 1;
        int swap_out_triggered = 0;

#ifdef VERBOSE
        printf("\tSearch %d by Process %d\n", current_process->current_search_number + 1, current_process->process_id);
#endif

        while (left < right && !swap_out_triggered) {
            int mid = (left + right) / 2;
            total_page_accesses++;

            int page_index = 10 + (mid / INTEGERS_PER_PAGE);

            if (!is_page_valid(process_page_tables, current_process->process_id, page_index)) {
                total_page_misses++;
                if (free_frame_count <= 0) {
                    perform_swap_out(current_process, active_process_queue, swapped_out_queue);
                    swap_out_triggered = 1;
                    break;
                }
                set_page_valid(process_page_tables, current_process->process_id, page_index);
            }

            if (search_key <= mid) right = mid;
            else left = mid + 1;
        }

        // Update process state
        if (!swap_out_triggered) {
            current_process->current_search_number++;
            if (current_process->current_search_number < total_searches) {
                add_to_queue(active_process_queue, current_process);
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
    

    // Print final statistics
    printf("+++ Page access summary\n");
    printf("Total number of page accesses = %d\n", total_page_accesses);
    printf("Total number of page faults = %d\n", total_page_misses);
    printf("Total number of swaps = %d\n", total_swap_operations);
    printf("Degree of multiprogramming = %d\n", min_active_processes);

    free(system_processes);
    free(process_page_tables);
    free(active_process_queue->process_list);
    free(active_process_queue);
    free(swapped_out_queue->process_list);
    free(swapped_out_queue);

    return 0;
}
