//milestone 3
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define MAX_M 20
#define MAX_N 100
#define MAX_REQUESTS 1000

typedef struct {
    int delay;
    char type; // 'R' for resource request, 'Q' for quit
    int req[MAX_M];
} Request;

typedef struct {
    int max_need[MAX_M];
    Request requests[MAX_REQUESTS];
    int num_requests;
} ThreadData;

typedef struct {
    int tid;
} ThreadArg;

// Global variables
int m, n;
int available[MAX_M];
ThreadData thread_data[MAX_N];

// Synchronization primitives (only BOS and pmtx for now)
pthread_barrier_t bos;
pthread_mutex_t pmtx;

void print_array(int arr[], int size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(": ");
    for (int i = 0; i < size; i++) printf("%d ", arr[i]);
    printf("\n");
}

void parse_system_file() {
    FILE *fp = fopen("input/system.txt", "r");
    if (!fp) { perror("Failed to open system.txt"); exit(1); }
    fscanf(fp, "%d %d", &m, &n);
    printf("//debug: m = %d, n = %d\n", m, n);
    for (int i = 0; i < m; i++) fscanf(fp, "%d", &available[i]);
    print_array(available, m, "//debug: Initial AVAILABLE");
    fclose(fp);
}

void parse_thread_file(int tid) {
    char filename[20];
    sprintf(filename, "input/thread%02d.txt", tid);
    FILE *fp = fopen(filename, "r");
    if (!fp) { perror("Failed to open thread file"); exit(1); }

    ThreadData *td = &thread_data[tid];
    for (int j = 0; j < m; j++) fscanf(fp, "%d", &td->max_need[j]);
    print_array(td->max_need, m, "//debug: Thread %d MAX_NEED", tid);

    int req_idx = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        Request *req = &td->requests[req_idx];
        if (sscanf(line, "%d Q", &req->delay) == 1) {
            req->type = 'Q';
            printf("//debug: Thread %d, Request %d: DELAY %d Q\n", tid, req_idx, req->delay);
            req_idx++;
            break;
        } else if (sscanf(line, "%d R", &req->delay) == 1) {
            req->type = 'R';
            char *token = strtok(line, " ");
            token = strtok(NULL, " "); // Skip DELAY
            token = strtok(NULL, " "); // Skip R
            for (int j = 0; j < m; j++) {
                req->req[j] = atoi(token);
                token = strtok(NULL, " ");
            }
            print_array(req->req, m, "//debug: Thread %d, Request %d REQ", tid, req_idx);
            req_idx++;
        }
    }
    td->num_requests = req_idx;
    printf("//debug: Thread %d has %d requests\n", tid, td->num_requests);
    fclose(fp);
}

void* user_thread(void* arg) {
    ThreadArg *ta = (ThreadArg*)arg;
    int tid = ta->tid;
    ThreadData *td = &thread_data[tid];

    pthread_barrier_wait(&bos);
    pthread_mutex_lock(&pmtx);
    printf("//debug: Thread %d started\n", tid);
    pthread_mutex_unlock(&pmtx);

    for (int i = 0; i < td->num_requests; i++) {
        Request *req = &td->requests[i];
        usleep(req->delay * 1000); // Simulate delay in microseconds (ms * 1000)

        pthread_mutex_lock(&pmtx);
        if (req->type == 'R') {
            print_array(req->req, m, "//debug: Thread %d processing R request", tid);
        } else if (req->type == 'Q') {
            printf("//debug: Thread %d processing Q request\n", tid);
        }
        pthread_mutex_unlock(&pmtx);
    }

    pthread_mutex_lock(&pmtx);
    printf("//debug: Thread %d exiting\n", tid);
    pthread_mutex_unlock(&pmtx);
    pthread_exit(NULL);
}

int main() {
    parse_system_file();
    for (int i = 0; i < n; i++) parse_thread_file(i);

    pthread_barrier_init(&bos, NULL, n + 1);
    pthread_mutex_init(&pmtx, NULL);

    pthread_t threads[MAX_N];
    ThreadArg args[MAX_N];
    for (int i = 0; i < n; i++) {
        args[i].tid = i;
        if (pthread_create(&threads[i], NULL, user_thread, &args[i]) != 0) {
            perror("Failed to create thread");
            exit(1);
        }
        printf("//debug: Created thread %d\n", i);
    }

    pthread_barrier_wait(&bos);
    printf("//debug: Master thread started simulation\n");

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
        printf("//debug: Thread %d joined\n", i);
    }

    pthread_barrier_destroy(&bos);
    pthread_mutex_destroy(&pmtx);

    printf("//debug: Simulation complete\n");
    return 0;
}