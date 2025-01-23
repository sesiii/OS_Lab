#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_BURSTS 20
#define MAX_PROCESSES 1000

#define READY 0
#define RUNNING 1
#define WAITING 2
#define FINISHED 3

#define ARRIVAL 0
#define CPU_COMPLETION 1
#define IO_COMPLETION 2
#define CPU_TIMEOUT 3

typedef struct {
    int id;
    int arrival_time;
    int bursts[MAX_BURSTS];
    int num_bursts;
    int current_burst;
    int remaining_time;
    int state;
    int wait_time;
    int turnaround_time;
    int running_time;
    int last_time;
} Process;

typedef struct {
    int process_idx;
    int time;
    int type;
} Event;

typedef struct {
    int items[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} Queue;

typedef struct {
    Event items[MAX_PROCESSES * MAX_BURSTS];
    int size;
} EventQueue;

Process processes[MAX_PROCESSES];
int num_processes;
Queue ready_queue;
EventQueue event_queue;
int current_time;
int cpu_idle_time;
int is_cpu_idle;

void init_queue(Queue* q) {
    q->front = q->rear = -1;
    q->size = 0;
}

int is_queue_empty(Queue* q) {
    return q->size == 0;
}

void enqueue(Queue* q, int process_idx) {
    if (q->size == 0) {
        q->front = q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % MAX_PROCESSES;
    }
    q->items[q->rear] = process_idx;
    q->size++;
}

int dequeue(Queue* q) {
    int item = q->items[q->front];
    q->size--;
    if (q->size == 0) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_PROCESSES;
    }
    return item;
}

void init_event_queue(EventQueue* eq) {
    eq->size = 0;
}

void swap_events(Event* a, Event* b) {
    Event temp = *a;
    *a = *b;
    *b = temp;
}

void push_event(EventQueue* eq, int process_idx, int time, int type) {
    int i = eq->size++;
    eq->items[i].process_idx = process_idx;
    eq->items[i].time = time;
    eq->items[i].type = type;

    while (i > 0) {
        int parent = (i - 1) / 2;
        if (eq->items[parent].time > eq->items[i].time ||
            (eq->items[parent].time == eq->items[i].time &&
             processes[eq->items[parent].process_idx].id > processes[eq->items[i].process_idx].id)) {
            swap_events(&eq->items[parent], &eq->items[i]);
            i = parent;
        } else {
            break;
        }
    }
}

Event pop_event(EventQueue* eq) {
    Event min_event = eq->items[0];
    eq->items[0] = eq->items[--eq->size];

    int i = 0;
    while (1) {
        int min_idx = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < eq->size &&
            (eq->items[left].time < eq->items[min_idx].time ||
             (eq->items[left].time == eq->items[min_idx].time &&
              processes[eq->items[left].process_idx].id < processes[eq->items[min_idx].process_idx].id)))
            min_idx = left;

        if (right < eq->size &&
            (eq->items[right].time < eq->items[min_idx].time ||
             (eq->items[right].time == eq->items[min_idx].time &&
              processes[eq->items[right].process_idx].id < processes[eq->items[min_idx].process_idx].id)))
            min_idx = right;

        if (min_idx != i) {
            swap_events(&eq->items[i], &eq->items[min_idx]);
            i = min_idx;
        } else {
            break;
        }
    }

    return min_event;
}

void read_input(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error opening file\n");
        exit(1);
    }

    fscanf(fp, "%d", &num_processes);
    for (int i = 0; i < num_processes; i++) {
        Process* p = &processes[i];
        fscanf(fp, "%d %d", &p->id, &p->arrival_time);
        
        p->num_bursts = 0;
        p->running_time = 0;
        while (1) {
            int burst;
            fscanf(fp, "%d", &burst);
            if (burst == -1) break;
            p->bursts[p->num_bursts++] = burst;
            p->running_time += burst;
            
            fscanf(fp, "%d", &burst);
            if (burst == -1) break;
            p->bursts[p->num_bursts++] = burst;
            p->running_time += burst;
        }
        
        p->current_burst = 0;
        p->remaining_time = p->bursts[0];
        p->state = READY;
        p->wait_time = 0;
        p->last_time = p->arrival_time;
    }
    fclose(fp);
}

void update_wait_times(int current_time) {
    for (int i = 0; i < ready_queue.size; i++) {
        int idx = (ready_queue.front + i) % MAX_PROCESSES;
        int process_idx = ready_queue.items[idx];
        if (processes[process_idx].state == READY) {
            processes[process_idx].wait_time += current_time - processes[process_idx].last_time;
            processes[process_idx].last_time = current_time;
        }
    }
}

void schedule_process(int quantum) {
    if (is_queue_empty(&ready_queue)) {
        is_cpu_idle = 1;
        return;
    }

    int process_idx = dequeue(&ready_queue);
    Process* p = &processes[process_idx];
    p->state = RUNNING;
    p->last_time = current_time;
    is_cpu_idle = 0;

    int run_time = quantum < p->remaining_time ? quantum : p->remaining_time;
    
    if (run_time == p->remaining_time) {
        push_event(&event_queue, process_idx, current_time + run_time, CPU_COMPLETION);
    } else {
        push_event(&event_queue, process_idx, current_time + run_time, CPU_TIMEOUT);
    }

    #ifdef VERBOSE
    printf("%d      :Process %d is scheduled to run for time %d\n", current_time, p->id, run_time);
    #endif
}

void simulate(int quantum) {
    printf("**** %s Scheduling %s ****\n",
           quantum == INT_MAX ? "FCFS" : "RR",
           quantum == INT_MAX ? "" : (quantum == 10 ? "with q = 10" : "with q = 5"));

    init_queue(&ready_queue);
    init_event_queue(&event_queue);
    current_time = 0;
    cpu_idle_time = 0;
    is_cpu_idle = 1;

    for (int i = 0; i < num_processes; i++) {
        processes[i].state = READY;
        processes[i].current_burst = 0;
        processes[i].remaining_time = processes[i].bursts[0];
        processes[i].wait_time = 0;
        processes[i].last_time = processes[i].arrival_time;
        push_event(&event_queue, i, processes[i].arrival_time, ARRIVAL);
    }

    #ifdef VERBOSE
    printf("0     :Starting\n");
    #endif

    while (event_queue.size > 0) {
        Event event = pop_event(&event_queue);
        
        update_wait_times(event.time);
        
        if (is_cpu_idle) {
            cpu_idle_time += event.time - current_time;
            #ifdef VERBOSE
            if (is_cpu_idle) {
                printf("%d      :CPU goes idle\n", current_time);
            }
            #endif
        }
        current_time = event.time;

        Process* p = &processes[event.process_idx];

        switch (event.type) {
            case ARRIVAL:
            case IO_COMPLETION:
                p->state = READY;
                p->last_time = current_time;
                enqueue(&ready_queue, event.process_idx);
                
                #ifdef VERBOSE
                printf("%d      :Process %d joins ready queue %s\n", 
                       current_time, p->id, 
                       event.type == ARRIVAL ? "upon arrival" : "after IO completion");
                #endif
                
                if (is_cpu_idle) {
                    schedule_process(quantum);
                }
                break;

            case CPU_COMPLETION:
                p->current_burst++;
                if (p->current_burst >= p->num_bursts) {
                    p->state = FINISHED;
                    p->turnaround_time = current_time - p->arrival_time;
                    
                    printf("%d      :Process %d exits. Turnaround time = %d (%d%%), Wait time = %d\n",
                           current_time, p->id, p->turnaround_time,
                           (p->turnaround_time * 100) / p->running_time,
                           p->wait_time);
                } else {
                    p->state = WAITING;
                    p->last_time = current_time;
                    push_event(&event_queue, event.process_idx,
                             current_time + p->bursts[p->current_burst],
                             IO_COMPLETION);
                    p->current_burst++;
                    p->remaining_time = p->bursts[p->current_burst];
                }
                is_cpu_idle = 1;
                schedule_process(quantum);
                break;

            case CPU_TIMEOUT:
                p->remaining_time -= quantum;
                
                #ifdef VERBOSE
                printf("%d     :Process %d joins ready queue after timeout\n", 
                       current_time, p->id);
                #endif
                
                p->state = READY;
                p->last_time = current_time;
                enqueue(&ready_queue, event.process_idx);
                is_cpu_idle = 1;
                schedule_process(quantum);
                break;
        }
    }

    double total_wait_time = 0;
    for (int i = 0; i < num_processes; i++) {
        total_wait_time += processes[i].wait_time;
    }
    printf("\n");
    printf("Average wait time = %.2f\n", total_wait_time / num_processes);
    printf("Total turnaround time = %d\n", current_time);
    printf("CPU idle time = %d\n", cpu_idle_time);
    printf("CPU utilization = %.2f%%\n",
           100.0 * (current_time - cpu_idle_time) / current_time);
    printf("\n");
}

int main() {
    read_input("proc.txt");
    
    simulate(INT_MAX);
    simulate(10);
    simulate(5);

    return 0;
}