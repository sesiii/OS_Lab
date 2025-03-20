// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>
// #include <string.h>
// #include <stdarg.h>

// #define MAX_M 20
// #define MAX_N 100
// #define MAX_REQUESTS 1000 // Arbitrary limit for requests per thread

// // Structure for a single request
// typedef struct {
//     int delay;
//     char type; // 'R' for resource request, 'Q' for quit
//     int req[MAX_M];
// } Request;

// // Structure for thread data
// typedef struct {
//     int max_need[MAX_M];
//     Request requests[MAX_REQUESTS];
//     int num_requests;
// } ThreadData;

// // Global variables
// int m, n;
// int available[MAX_M];
// ThreadData thread_data[MAX_N];

// // Debug function to print arrays with formatted string
// void print_array(int arr[], int size, const char* format, ...) {
//     va_list args;
//     va_start(args, format);
//     vprintf(format, args);
//     va_end(args);
//     printf(": ");
//     for (int i = 0; i < size; i++) {
//         printf("%d ", arr[i]);
//     }
//     printf("\n");
// }

// void parse_system_file() {
//     FILE *fp = fopen("input/system.txt", "r");
//     if (!fp) {
//         perror("Failed to open system.txt");
//         exit(1);
//     }
//     fscanf(fp, "%d %d", &m, &n);
//     //debug
//     printf("//debug: m = %d, n = %d\n", m, n);
//     for (int i = 0; i < m; i++) {
//         fscanf(fp, "%d", &available[i]);
//     }
//     //debug
//     print_array(available, m, "//debug: Initial AVAILABLE");
//     fclose(fp);
// }

// void parse_thread_file(int tid) {
//     char filename[20];
//     sprintf(filename, "input/thread%02d.txt", tid);
//     FILE *fp = fopen(filename, "r");
//     if (!fp) {
//         perror("Failed to open thread file");
//         exit(1);
//     }

//     ThreadData *td = &thread_data[tid];
//     for (int j = 0; j < m; j++) {
//         fscanf(fp, "%d", &td->max_need[j]);
//     }
//     //debug
//     print_array(td->max_need, m, "//debug: Thread %d MAX_NEED", tid);

//     int req_idx = 0;
//     char line[1024];
//     while (fgets(line, sizeof(line), fp)) {
//         Request *req = &td->requests[req_idx];
//         if (sscanf(line, "%d Q", &req->delay) == 1) {
//             req->type = 'Q';
//             //debug
//             printf("//debug: Thread %d, Request %d: DELAY %d Q\n", tid, req_idx, req->delay);
//             req_idx++;
//             break;
//         } else if (sscanf(line, "%d R", &req->delay) == 1) {
//             req->type = 'R';
//             char *token = strtok(line, " ");
//             token = strtok(NULL, " "); // Skip DELAY
//             token = strtok(NULL, " "); // Skip R
//             for (int j = 0; j < m; j++) {
//                 req->req[j] = atoi(token);
//                 token = strtok(NULL, " ");
//             }
//             //debug
//             print_array(req->req, m, "//debug: Thread %d, Request %d REQ", tid, req_idx);
//             req_idx++;
//         }
//     }
//     td->num_requests = req_idx;
//     //debug
//     printf("//debug: Thread %d has %d requests\n", tid, td->num_requests);
//     fclose(fp);
// }

// int main() {
//     parse_system_file();
//     for (int i = 0; i < n; i++) {
//         parse_thread_file(i);
//     }
//     //debug
//     printf("//debug: Input parsing complete\n");
//     return 0;
// }


// //milestone 2
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>
// #include <string.h>
// #include <stdarg.h>

// #define MAX_M 20
// #define MAX_N 100
// #define MAX_REQUESTS 1000

// typedef struct {
//     int delay;
//     char type; // 'R' for resource request, 'Q' for quit
//     int req[MAX_M];
// } Request;

// typedef struct {
//     int max_need[MAX_M];
//     Request requests[MAX_REQUESTS];
//     int num_requests;
// } ThreadData;

// // Global variables
// int m, n;
// int available[MAX_M];
// ThreadData thread_data[MAX_N];

// // Synchronization primitives
// pthread_barrier_t bos; // Beginning of session barrier
// pthread_mutex_t rmtx;  // Mutex for request communication
// pthread_barrier_t reqb; // Request barrier
// pthread_barrier_t ackb; // Acknowledgment barrier
// pthread_mutex_t pmtx;   // Mutex for printing

// // Structure for user thread arguments
// typedef struct {
//     int tid;
// } ThreadArg;

// // Debug function
// void print_array(int arr[], int size, const char* format, ...) {
//     va_list args;
//     va_start(args, format);
//     vprintf(format, args);
//     va_end(args);
//     printf(": ");
//     for (int i = 0; i < size; i++) printf("%d ", arr[i]);
//     printf("\n");
// }

// void parse_system_file() {
//     FILE *fp = fopen("input/system.txt", "r");
//     if (!fp) { perror("Failed to open system.txt"); exit(1); }
//     fscanf(fp, "%d %d", &m, &n);
//     printf("//debug: m = %d, n = %d\n", m, n);
//     for (int i = 0; i < m; i++) fscanf(fp, "%d", &available[i]);
//     print_array(available, m, "//debug: Initial AVAILABLE");
//     fclose(fp);
// }

// void parse_thread_file(int tid) {
//     char filename[20];
//     sprintf(filename, "input/thread%02d.txt", tid);
//     FILE *fp = fopen(filename, "r");
//     if (!fp) { perror("Failed to open thread file"); exit(1); }

//     ThreadData *td = &thread_data[tid];
//     for (int j = 0; j < m; j++) fscanf(fp, "%d", &td->max_need[j]);
//     print_array(td->max_need, m, "//debug: Thread %d MAX_NEED", tid);

//     int req_idx = 0;
//     char line[1024];
//     while (fgets(line, sizeof(line), fp)) {
//         Request *req = &td->requests[req_idx];
//         if (sscanf(line, "%d Q", &req->delay) == 1) {
//             req->type = 'Q';
//             printf("//debug: Thread %d, Request %d: DELAY %d Q\n", tid, req_idx, req->delay);
//             req_idx++;
//             break;
//         } else if (sscanf(line, "%d R", &req->delay) == 1) {
//             req->type = 'R';
//             char *token = strtok(line, " ");
//             token = strtok(NULL, " "); // Skip DELAY
//             token = strtok(NULL, " "); // Skip R
//             for (int j = 0; j < m; j++) {
//                 req->req[j] = atoi(token);
//                 token = strtok(NULL, " ");
//             }
//             print_array(req->req, m, "//debug: Thread %d, Request %d REQ", tid, req_idx);
//             req_idx++;
//         }
//     }
//     td->num_requests = req_idx;
//     printf("//debug: Thread %d has %d requests\n", tid, td->num_requests);
//     fclose(fp);
// }

// void* user_thread(void* arg) {
//     ThreadArg *ta = (ThreadArg*)arg;
//     int tid = ta->tid;

//     // Wait at BOS barrier
//     pthread_barrier_wait(&bos);
//     printf("//debug: Thread %d started\n", tid);

//     // Placeholder for request handling (Milestone 3)
//     pthread_exit(NULL);
// }

// int main() {
//     // Parse inputs
//     parse_system_file();
//     for (int i = 0; i < n; i++) parse_thread_file(i);

//     // Initialize synchronization primitives
//     pthread_barrier_init(&bos, NULL, n + 1); // n user threads + master
//     pthread_mutex_init(&rmtx, NULL);
//     pthread_barrier_init(&reqb, NULL, 2); // Master + 1 user thread
//     pthread_barrier_init(&ackb, NULL, 2); // Master + 1 user thread
//     pthread_mutex_init(&pmtx, NULL);

//     // Create user threads
//     pthread_t threads[MAX_N];
//     ThreadArg args[MAX_N];
//     for (int i = 0; i < n; i++) {
//         args[i].tid = i;
//         if (pthread_create(&threads[i], NULL, user_thread, &args[i]) != 0) {
//             perror("Failed to create thread");
//             exit(1);
//         }
//         printf("//debug: Created thread %d\n", i);
//     }

//     // Master thread waits at BOS
//     pthread_barrier_wait(&bos);
//     printf("//debug: Master thread started simulation\n");

//     // Join threads
//     for (int i = 0; i < n; i++) {
//         pthread_join(threads[i], NULL);
//         printf("//debug: Thread %d joined\n", i);
//     }

//     // Cleanup
//     pthread_barrier_destroy(&bos);
//     pthread_mutex_destroy(&rmtx);
//     pthread_barrier_destroy(&reqb);
//     pthread_barrier_destroy(&ackb);
//     pthread_mutex_destroy(&pmtx);

//     printf("//debug: Simulation complete\n");
//     return 0;
// }


// //milestone 3
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>
// #include <string.h>
// #include <stdarg.h>

// #define MAX_M 20
// #define MAX_N 100
// #define MAX_REQUESTS 1000

// typedef struct {
//     int delay;
//     char type; // 'R' for resource request, 'Q' for quit
//     int req[MAX_M];
// } Request;

// typedef struct {
//     int max_need[MAX_M];
//     Request requests[MAX_REQUESTS];
//     int num_requests;
// } ThreadData;

// typedef struct {
//     int tid;
// } ThreadArg;

// // Global variables
// int m, n;
// int available[MAX_M];
// ThreadData thread_data[MAX_N];

// // Synchronization primitives (only BOS and pmtx for now)
// pthread_barrier_t bos;
// pthread_mutex_t pmtx;

// void print_array(int arr[], int size, const char* format, ...) {
//     va_list args;
//     va_start(args, format);
//     vprintf(format, args);
//     va_end(args);
//     printf(": ");
//     for (int i = 0; i < size; i++) printf("%d ", arr[i]);
//     printf("\n");
// }

// void parse_system_file() {
//     FILE *fp = fopen("input/system.txt", "r");
//     if (!fp) { perror("Failed to open system.txt"); exit(1); }
//     fscanf(fp, "%d %d", &m, &n);
//     printf("//debug: m = %d, n = %d\n", m, n);
//     for (int i = 0; i < m; i++) fscanf(fp, "%d", &available[i]);
//     print_array(available, m, "//debug: Initial AVAILABLE");
//     fclose(fp);
// }

// void parse_thread_file(int tid) {
//     char filename[20];
//     sprintf(filename, "input/thread%02d.txt", tid);
//     FILE *fp = fopen(filename, "r");
//     if (!fp) { perror("Failed to open thread file"); exit(1); }

//     ThreadData *td = &thread_data[tid];
//     for (int j = 0; j < m; j++) fscanf(fp, "%d", &td->max_need[j]);
//     print_array(td->max_need, m, "//debug: Thread %d MAX_NEED", tid);

//     int req_idx = 0;
//     char line[1024];
//     while (fgets(line, sizeof(line), fp)) {
//         Request *req = &td->requests[req_idx];
//         if (sscanf(line, "%d Q", &req->delay) == 1) {
//             req->type = 'Q';
//             printf("//debug: Thread %d, Request %d: DELAY %d Q\n", tid, req_idx, req->delay);
//             req_idx++;
//             break;
//         } else if (sscanf(line, "%d R", &req->delay) == 1) {
//             req->type = 'R';
//             char *token = strtok(line, " ");
//             token = strtok(NULL, " "); // Skip DELAY
//             token = strtok(NULL, " "); // Skip R
//             for (int j = 0; j < m; j++) {
//                 req->req[j] = atoi(token);
//                 token = strtok(NULL, " ");
//             }
//             print_array(req->req, m, "//debug: Thread %d, Request %d REQ", tid, req_idx);
//             req_idx++;
//         }
//     }
//     td->num_requests = req_idx;
//     printf("//debug: Thread %d has %d requests\n", tid, td->num_requests);
//     fclose(fp);
// }

// void* user_thread(void* arg) {
//     ThreadArg *ta = (ThreadArg*)arg;
//     int tid = ta->tid;
//     ThreadData *td = &thread_data[tid];

//     pthread_barrier_wait(&bos);
//     pthread_mutex_lock(&pmtx);
//     printf("//debug: Thread %d started\n", tid);
//     pthread_mutex_unlock(&pmtx);

//     for (int i = 0; i < td->num_requests; i++) {
//         Request *req = &td->requests[i];
//         usleep(req->delay * 1000); // Simulate delay in microseconds (ms * 1000)

//         pthread_mutex_lock(&pmtx);
//         if (req->type == 'R') {
//             print_array(req->req, m, "//debug: Thread %d processing R request", tid);
//         } else if (req->type == 'Q') {
//             printf("//debug: Thread %d processing Q request\n", tid);
//         }
//         pthread_mutex_unlock(&pmtx);
//     }

//     pthread_mutex_lock(&pmtx);
//     printf("//debug: Thread %d exiting\n", tid);
//     pthread_mutex_unlock(&pmtx);
//     pthread_exit(NULL);
// }

// int main() {
//     parse_system_file();
//     for (int i = 0; i < n; i++) parse_thread_file(i);

//     pthread_barrier_init(&bos, NULL, n + 1);
//     pthread_mutex_init(&pmtx, NULL);

//     pthread_t threads[MAX_N];
//     ThreadArg args[MAX_N];
//     for (int i = 0; i < n; i++) {
//         args[i].tid = i;
//         if (pthread_create(&threads[i], NULL, user_thread, &args[i]) != 0) {
//             perror("Failed to create thread");
//             exit(1);
//         }
//         printf("//debug: Created thread %d\n", i);
//     }

//     pthread_barrier_wait(&bos);
//     printf("//debug: Master thread started simulation\n");

//     for (int i = 0; i < n; i++) {
//         pthread_join(threads[i], NULL);
//         printf("//debug: Thread %d joined\n", i);
//     }

//     pthread_barrier_destroy(&bos);
//     pthread_mutex_destroy(&pmtx);

//     printf("//debug: Simulation complete\n");
//     return 0;
// }


//milestone 4
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define MAX_M 20
#define MAX_N 100
#define MAX_REQUESTS 1000
#define MAX_QUEUE 100

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

typedef struct {
    int tid;
    int req[MAX_M];
} QueueEntry;

// Global variables
int m, n;
int available[MAX_M];
int alloc[MAX_N][MAX_M];
int need[MAX_N][MAX_M];
ThreadData thread_data[MAX_N];

// Synchronization primitives
pthread_barrier_t bos;
pthread_mutex_t rmtx;
pthread_barrier_t reqb;
pthread_barrier_t ackb;
pthread_mutex_t pmtx;
pthread_cond_t cv[MAX_N];
pthread_mutex_t cmtx[MAX_N];

// Global request and queue
typedef struct {
    int tid;
    char type;
    int req[MAX_M];
} GlobalRequest;
GlobalRequest global_req;
QueueEntry queue[MAX_QUEUE];
int q_front = 0, q_rear = 0, q_size = 0;

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
    memcpy(need[tid], td->max_need, m * sizeof(int)); // Initialize NEED

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

void enqueue(int tid, int req[]) {
    if (q_size < MAX_QUEUE) {
        queue[q_rear].tid = tid;
        memcpy(queue[q_rear].req, req, m * sizeof(int));
        q_rear = (q_rear + 1) % MAX_QUEUE;
        q_size++;
        printf("//debug: Enqueued request for Thread %d\n", tid);
    }
}

void process_queue() {
    while (q_size > 0) {
        int tid = queue[q_front].tid;
        int* req = queue[q_front].req;
        int can_grant = 1;
        for (int j = 0; j < m; j++) {
            if (req[j] > available[j]) {
                can_grant = 0;
                break;
            }
        }
        if (can_grant) {
            for (int j = 0; j < m; j++) {
                available[j] -= req[j];
                alloc[tid][j] += req[j];
                need[tid][j] -= req[j];
            }
            q_front = (q_front + 1) % MAX_QUEUE;
            q_size--;
            pthread_mutex_lock(&pmtx);
            print_array(available, m, "//debug: Granted request for Thread %d, AVAILABLE", tid);
            pthread_mutex_unlock(&pmtx);
            pthread_cond_signal(&cv[tid]);
        } else {
            break; // Wait for more resources
        }
    }
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
        usleep(req->delay * 1000);

        pthread_mutex_lock(&rmtx);
        global_req.tid = tid;
        global_req.type = req->type;
        if (req->type == 'R') memcpy(global_req.req, req->req, m * sizeof(int));
        else memset(global_req.req, 0, m * sizeof(int));
        pthread_mutex_lock(&pmtx);
        if (req->type == 'R') print_array(global_req.req, m, "//debug: Thread %d sending R request", tid);
        else printf("//debug: Thread %d sending Q request\n", tid);
        pthread_mutex_unlock(&pmtx);

        pthread_barrier_wait(&reqb);
        pthread_barrier_wait(&ackb);

        int is_additional = (req->type == 'R' && memchr(req->req, 1, m * sizeof(int)) != NULL);
        if (is_additional) {
            pthread_mutex_lock(&cmtx[tid]);
            pthread_cond_wait(&cv[tid], &cmtx[tid]);
            pthread_mutex_unlock(&cmtx[tid]);
            pthread_mutex_lock(&pmtx);
            printf("//debug: Thread %d ADDITIONAL request granted\n", tid);
            pthread_mutex_unlock(&pmtx);
        }
        pthread_mutex_unlock(&rmtx);
    }

    pthread_mutex_lock(&pmtx);
    printf("//debug: Thread %d exiting\n", tid);
    pthread_mutex_unlock(&pmtx);
    pthread_exit(NULL);
}

void* master_thread(void* arg) {
    pthread_barrier_wait(&bos);
    printf("//debug: Master thread started simulation\n");

    int active_threads = n;
    while (active_threads > 0) {
        pthread_barrier_wait(&reqb);
        int tid = global_req.tid;
        pthread_mutex_lock(&pmtx);
        printf("//debug: Master received request from Thread %d, type %c\n", tid, global_req.type);
        pthread_mutex_unlock(&pmtx);

        if (global_req.type == 'Q' || (global_req.type == 'R' && !memchr(global_req.req, 1, m * sizeof(int)))) {
            // RELEASE request
            for (int j = 0; j < m; j++) {
                if (global_req.type == 'R' && global_req.req[j] < 0) {
                    available[j] += -global_req.req[j];
                    alloc[tid][j] -= -global_req.req[j];
                    need[tid][j] += -global_req.req[j];
                } else if (global_req.type == 'Q' && alloc[tid][j] > 0) {
                    available[j] += alloc[tid][j];
                    alloc[tid][j] = 0;
                    need[tid][j] = 0;
                }
            }
            if (global_req.type == 'Q') active_threads--;
            process_queue();
        } else {
            // ADDITIONAL request
            for (int j = 0; j < m; j++) {
                if (global_req.req[j] < 0) {
                    available[j] += -global_req.req[j];
                    alloc[tid][j] -= -global_req.req[j];
                    need[tid][j] += -global_req.req[j];
                    global_req.req[j] = 0;
                }
            }
            enqueue(tid, global_req.req);
            process_queue();
        }
        pthread_barrier_wait(&ackb);
    }
    pthread_exit(NULL);
}

int main() {
    parse_system_file();
    for (int i = 0; i < n; i++) parse_thread_file(i);

    pthread_barrier_init(&bos, NULL, n + 1);
    pthread_mutex_init(&rmtx, NULL);
    pthread_barrier_init(&reqb, NULL, 2);
    pthread_barrier_init(&ackb, NULL, 2);
    pthread_mutex_init(&pmtx, NULL);
    for (int i = 0; i < n; i++) {
        pthread_cond_init(&cv[i], NULL);
        pthread_mutex_init(&cmtx[i], NULL);
    }

    pthread_t threads[MAX_N], master;
    ThreadArg args[MAX_N];
    pthread_create(&master, NULL, master_thread, NULL);
    for (int i = 0; i < n; i++) {
        args[i].tid = i;
        pthread_create(&threads[i], NULL, user_thread, &args[i]);
        printf("//debug: Created thread %d\n", i);
    }

    for (int i = 0; i < n; i++) pthread_join(threads[i], NULL);
    pthread_join(master, NULL);
    printf("//debug: All threads joined\n");

    pthread_barrier_destroy(&bos);
    pthread_mutex_destroy(&rmtx);
    pthread_barrier_destroy(&reqb);
    pthread_barrier_destroy(&ackb);
    pthread_mutex_destroy(&pmtx);
    for (int i = 0; i < n; i++) {
        pthread_cond_destroy(&cv[i]);
        pthread_mutex_destroy(&cmtx[i]);
    }

    printf("//debug: Simulation complete\n");
    return 0;
}