

// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// // #include <pthread.h>
// // #include <unistd.h>

// // // Structure for user thread requests
// // struct UserRequest {
// //     int delay;
// //     char type;
// //     int* req;
// //     struct UserRequest* next;
// // };

// // // Structure for queued requests
// // struct QueuedRequest {
// //     int thread_id;
// //     int* req;
// //     struct QueuedRequest* next;
// // };

// // // Global synchronization tools
// // pthread_barrier_t BOS;
// // pthread_barrier_t REQB;
// // pthread_barrier_t* ACKB;
// // pthread_mutex_t rmtx;
// // pthread_mutex_t pmtx;
// // pthread_mutex_t* grant_mutex;
// // pthread_cond_t* grant_cond;
// // int* done;
// // int* pending_additional;

// // // Global resource state
// // int m, n;
// // int* available;
// // int** need;
// // int** allocation;

// // // Shared memory for requests
// // struct Request {
// //     char type;
// //     int thread_id;
// //     int* req;
// // } shared_request;

// // // Struct for thread data
// // struct ThreadData {
// //     int thread_id;
// //     int* done;
// //     int* pending_additional;
// // };

// // // Read system.txt
// // int read_system(const char* filename) {
// //     FILE* file = fopen(filename, "r");
// //     if (!file) return 1;
// //     if (fscanf(file, "%d %d", &m, &n) != 2) { fclose(file); return 1; }
// //     available = malloc(m * sizeof(int));
// //     need = malloc(n * sizeof(int*));
// //     allocation = malloc(n * sizeof(int*));
// //     for (int i = 0; i < n; i++) {
// //         need[i] = malloc(m * sizeof(int));
// //         allocation[i] = malloc(m * sizeof(int));
// //         memset(allocation[i], 0, m * sizeof(int));
// //     }
// //     for (int j = 0; j < m; j++) {
// //         fscanf(file, "%d", &available[j]);
// //     }
// //     fclose(file);
// //     printf("System initialized: m=%d, n=%d\n", m, n);
// //     return 0;
// // }

// // // Read thread file
// // struct UserRequest* read_thread(const char* filename, int thread_id) {
// //     FILE* file = fopen(filename, "r");
// //     if (!file) return NULL;
// //     for (int j = 0; j < m; j++) {
// //         fscanf(file, "%d", &need[thread_id][j]);
// //     }
// //     struct UserRequest* head = NULL;
// //     struct UserRequest* tail = NULL;
// //     int delay;
// //     char type;
// //     while (fscanf(file, "%d %c", &delay, &type) == 2) {
// //         struct UserRequest* req = malloc(sizeof(struct UserRequest));
// //         req->delay = delay;
// //         req->type = type;
// //         req->req = type == 'Q' ? NULL : malloc(m * sizeof(int));
// //         req->next = NULL;
// //         if (type == 'R') {
// //             for (int j = 0; j < m; j++) {
// //                 fscanf(file, "%d", &req->req[j]);
// //             }
// //         }
// //         if (!head) head = tail = req;
// //         else { tail->next = req; tail = req; }
// //     }
// //     fclose(file);
// //     return head;
// // }

// // void free_user_requests(struct UserRequest* head) {
// //     while (head) {
// //         struct UserRequest* next = head->next;
// //         if (head->req) free(head->req);
// //         free(head);
// //         head = next;
// //     }
// // }

// // // User thread function
// // void* user_thread(void* arg) {
// //     struct ThreadData* data = (struct ThreadData*)arg;
// //     int id = data->thread_id;
// //     int* done = data->done;
// //     int* pending_additional = data->pending_additional;

// //     pthread_mutex_lock(&pmtx);
// //     printf("\tThread %2d born\n", id);
// //     pthread_mutex_unlock(&pmtx);

// //     char filename[50];
// //     sprintf(filename, "../input/thread%02d.txt", id);
// //     struct UserRequest* requests = read_thread(filename, id);
// //     if (!requests) pthread_exit(NULL);

// //     pthread_barrier_wait(&BOS);

// //     struct UserRequest* curr = requests;
// //     int* holding = calloc(m, sizeof(int));
// //     while (curr) {
// //         usleep(curr->delay * 1000);

// //         pthread_mutex_lock(&rmtx);
// //         shared_request.thread_id = id;
// //         shared_request.type = curr->type == 'Q' ? 'Q' : 'R';
// //         if (curr->type == 'Q') {
// //             shared_request.req = malloc(m * sizeof(int));
// //             for (int j = 0; j < m; j++) {
// //                 shared_request.req[j] = -holding[j];
// //             }
// //         } else {
// //             shared_request.req = curr->req;
// //             int is_additional = 0;
// //             for (int j = 0; j < m; j++) {
// //                 if (curr->req[j] > 0) { is_additional = 1; break; }
// //             }
// //             pthread_mutex_lock(&pmtx);
// //             printf("\tThread %2d sends resource request: type = %s\n", id, is_additional ? "ADDITIONAL" : "RELEASE");
// //             pthread_mutex_unlock(&pmtx);
// //             if (is_additional) pending_additional[id] = 1;
// //         }

// //         pthread_barrier_wait(&REQB);
// //         pthread_barrier_wait(&ACKB[id]);
// //         pthread_mutex_unlock(&rmtx);

// //         if (curr->type == 'R') {
// //             int is_additional = pending_additional[id];
// //             if (is_additional) {
// //                 pthread_mutex_lock(&grant_mutex[id]);
// //                 while (!done[id]) {
// //                     pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
// //                 }
// //                 pthread_mutex_lock(&pmtx);
// //                 printf("\tThread %2d is granted its last resource request\n", id);
// //                 pthread_mutex_unlock(&pmtx);
// //                 done[id] = 0;
// //                 pending_additional[id] = 0;
// //                 pthread_mutex_unlock(&grant_mutex[id]);
// //             }
// //             for (int j = 0; j < m; j++) {
// //                 holding[j] += curr->req[j];
// //             }
// //         } else if (curr->type == 'Q') {
// //             if (pending_additional[id]) {
// //                 pthread_mutex_lock(&grant_mutex[id]);
// //                 while (!done[id]) {
// //                     pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
// //                 }
// //                 pthread_mutex_lock(&pmtx);
// //                 printf("\tThread %2d is granted its last resource request\n", id);
// //                 pthread_mutex_unlock(&pmtx);
// //                 done[id] = 0;
// //                 pending_additional[id] = 0;
// //                 pthread_mutex_unlock(&grant_mutex[id]);
// //             }
// //             pthread_mutex_lock(&pmtx);
// //             printf("\tThread %2d going to quit\n", id);
// //             pthread_mutex_unlock(&pmtx);
// //             break; // Master will free shared_request.req
// //         }

// //         curr = curr->next;
// //     }

// //     free(holding);
// //     free_user_requests(requests);
// //     pthread_exit(NULL);
// // }

// // // Main function (master thread)
// // int main() {
// //     if (read_system("../input/system.txt")) return 1;

// //     struct QueuedRequest* queue_head = NULL;
// //     struct QueuedRequest* queue_tail = NULL;

// //     pthread_barrier_init(&BOS, NULL, n + 1);
// //     pthread_barrier_init(&REQB, NULL, 2);
// //     ACKB = malloc(n * sizeof(pthread_barrier_t));
// //     grant_mutex = malloc(n * sizeof(pthread_mutex_t));
// //     grant_cond = malloc(n * sizeof(pthread_cond_t));
// //     done = calloc(n, sizeof(int));
// //     pending_additional = calloc(n, sizeof(int));
// //     for (int i = 0; i < n; i++) {
// //         pthread_barrier_init(&ACKB[i], NULL, 2);
// //         pthread_mutex_init(&grant_mutex[i], NULL);
// //         pthread_cond_init(&grant_cond[i], NULL);
// //     }
// //     pthread_mutex_init(&rmtx, NULL);
// //     pthread_mutex_init(&pmtx, NULL);

// //     shared_request.req = NULL; // Initialize to NULL, allocated per request

// //     pthread_t* threads = malloc(n * sizeof(pthread_t));
// //     struct ThreadData* thread_data = malloc(n * sizeof(struct ThreadData));
// //     for (int i = 0; i < n; i++) {
// //         thread_data[i].thread_id = i;
// //         thread_data[i].done = done;
// //         thread_data[i].pending_additional = pending_additional;
// //         pthread_create(&threads[i], NULL, user_thread, &thread_data[i]);
// //     }

// //     pthread_barrier_wait(&BOS);

// //     int active_threads = n;
// //     while (active_threads > 0) {
// //         pthread_barrier_wait(&REQB);

// //         int tid = shared_request.thread_id;
// //         char type = shared_request.type;
// //         int* req = malloc(m * sizeof(int));
// //         memcpy(req, shared_request.req, m * sizeof(int));

// //         pthread_barrier_wait(&ACKB[tid]);

// //         for (int j = 0; j < m; j++) {
// //             if (req[j] < 0) {
// //                 available[j] -= req[j];
// //                 allocation[tid][j] += req[j];
// //                 need[tid][j] -= req[j];
// //                 req[j] = 0;
// //             }
// //         }

// //         if (type == 'Q') {
// //             active_threads--;
// //             pthread_mutex_lock(&pmtx);
// //             printf("Master thread releases resources of thread %2d\n", tid);
// //             if (queue_head) {
// //                 printf("\t\tWaiting threads:");
// //                 struct QueuedRequest* q = queue_head;
// //                 while (q) {
// //                     printf(" %d", q->thread_id);
// //                     q = q->next;
// //                 }
// //                 printf("\n");
// //             } else {
// //                 printf("\t\tWaiting threads:\n");
// //             }
// //             printf("%d threads left\n", active_threads);
// //             printf("Available resources: ");
// //             for (int j = 0; j < m; j++) {
// //                 printf("%d ", available[j]);
// //             }
// //             printf("\n");
// //             pthread_mutex_unlock(&pmtx);
// //             free(req); // Master owns this memory
// //         } else {
// //             int is_additional = 0;
// //             for (int j = 0; j < m; j++) {
// //                 if (req[j] > 0) { is_additional = 1; break; }
// //             }
// //             if (is_additional) {
// //                 struct QueuedRequest* qr = malloc(sizeof(struct QueuedRequest));
// //                 qr->thread_id = tid;
// //                 qr->req = req;
// //                 qr->next = NULL;
// //                 if (!queue_head) queue_head = queue_tail = qr;
// //                 else { queue_tail->next = qr; queue_tail = qr; }
// //                 pthread_mutex_lock(&pmtx);
// //                 printf("Master thread stores resource request of thread %2d\n", tid);
// //                 printf("\t\tWaiting threads:");
// //                 struct QueuedRequest* q = queue_head;
// //                 while (q) {
// //                     printf(" %d", q->thread_id);
// //                     q = q->next;
// //                 }
// //                 printf("\n");
// //                 pthread_mutex_unlock(&pmtx);
// //             } else {
// //                 free(req);
// //             }
// //         }

// //         pthread_mutex_lock(&pmtx);
// //         printf("Master thread tries to grant pending requests\n");
// //         pthread_mutex_unlock(&pmtx);

// //         struct QueuedRequest* new_head = NULL;
// //         struct QueuedRequest* new_tail = NULL;
// //         struct QueuedRequest* curr = queue_head;
// //         queue_head = NULL;
// //         queue_tail = NULL;

// //         while (curr) {
// //             int tid = curr->thread_id;
// //             int* req = curr->req;
// //             int can_grant = 1;
// //             for (int j = 0; j < m; j++) {
// //                 if (req[j] > available[j]) {
// //                     can_grant = 0;
// //                     break;
// //                 }
// //             }

// //             struct QueuedRequest* next = curr->next;
// //             if (can_grant) {
// //                 for (int j = 0; j < m; j++) {
// //                     available[j] -= req[j];
// //                     allocation[tid][j] += req[j];
// //                     need[tid][j] -= req[j];
// //                 }
// //                 pthread_mutex_lock(&pmtx);
// //                 printf("Master thread grants resource request for thread %2d\n", tid);
// //                 pthread_mutex_unlock(&pmtx);
// //                 pthread_mutex_lock(&grant_mutex[tid]);
// //                 done[tid] = 1;
// //                 pthread_cond_signal(&grant_cond[tid]);
// //                 pthread_mutex_unlock(&grant_mutex[tid]);
// //                 free(req);
// //                 free(curr);
// //             } else {
// //                 pthread_mutex_lock(&pmtx);
// //                 printf("    +++ Insufficient resources to grant request of thread %2d\n", tid);
// //                 pthread_mutex_unlock(&pmtx);
// //                 curr->next = NULL;
// //                 if (!new_head) new_head = new_tail = curr;
// //                 else { new_tail->next = curr; new_tail = curr; }
// //             }
// //             curr = next;
// //         }
// //         queue_head = new_head;
// //         queue_tail = new_tail;

// //         pthread_mutex_lock(&pmtx);
// //         printf("\t\tWaiting threads:");
// //         struct QueuedRequest* q = queue_head;
// //         while (q) {
// //             printf(" %d", q->thread_id);
// //             q = q->next;
// //         }
// //         printf("\n");
// //         pthread_mutex_unlock(&pmtx);
// //     }

// //     for (int i = 0; i < n; i++) {
// //         pthread_join(threads[i], NULL);
// //         pthread_barrier_destroy(&ACKB[i]);
// //         pthread_mutex_destroy(&grant_mutex[i]);
// //         pthread_cond_destroy(&grant_cond[i]);
// //     }

// //     free(threads);
// //     free(thread_data);
// //     free(done);
// //     free(pending_additional);
// //     while (queue_head) {
// //         struct QueuedRequest* temp = queue_head;
// //         queue_head = queue_head->next;
// //         free(temp->req);
// //         free(temp);
// //     }
// //     pthread_barrier_destroy(&BOS);
// //     pthread_barrier_destroy(&REQB);
// //     pthread_mutex_destroy(&rmtx);
// //     pthread_mutex_destroy(&pmtx);
// //     free(ACKB);
// //     free(grant_mutex);
// //     free(grant_cond);
// //     if (shared_request.req) free(shared_request.req); // Only if not already freed
// //     free(available);
// //     for (int i = 0; i < n; i++) {
// //         free(need[i]);
// //         free(allocation[i]);
// //     }
// //     free(need);
// //     free(allocation);

// //     return 0;
// // }



// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
// #include <unistd.h>

// // Structure for user thread requests
// struct UserRequest {
//     int delay;
//     char type;
//     int* req;
//     struct UserRequest* next;
// };

// // Structure for queued requests
// struct QueuedRequest {
//     int thread_id;
//     int* req;
//     struct QueuedRequest* next;
// };

// // Global synchronization tools
// pthread_barrier_t BOS;
// pthread_barrier_t REQB;
// pthread_barrier_t* ACKB;
// pthread_mutex_t rmtx;
// pthread_mutex_t pmtx;
// pthread_mutex_t* grant_mutex;
// pthread_cond_t* grant_cond;
// int* done;
// int* pending_additional;

// // Global resource state
// int m, n;
// int* available;
// int** need;
// int** allocation;

// // Shared memory for requests
// struct Request {
//     char type;
//     int thread_id;
//     int* req;
// } shared_request;

// // Struct for thread data
// struct ThreadData {
//     int thread_id;
//     int* done;
//     int* pending_additional;
// };

// // Read system.txt
// int read_system(const char* filename) {
//     FILE* file = fopen(filename, "r");
//     if (!file) return 1;
//     if (fscanf(file, "%d %d", &m, &n) != 2) { fclose(file); return 1; }
//     available = malloc(m * sizeof(int));
//     need = malloc(n * sizeof(int*));
//     allocation = malloc(n * sizeof(int*));
//     for (int i = 0; i < n; i++) {
//         need[i] = malloc(m * sizeof(int));
//         allocation[i] = malloc(m * sizeof(int));
//         memset(allocation[i], 0, m * sizeof(int));
//     }
//     for (int j = 0; j < m; j++) {
//         fscanf(file, "%d", &available[j]);
//     }
//     fclose(file);
//     printf("System initialized: m=%d, n=%d\n", m, n);
//     return 0;
// }

// // Read thread file
// struct UserRequest* read_thread(const char* filename, int thread_id) {
//     FILE* file = fopen(filename, "r");
//     if (!file) return NULL;
//     for (int j = 0; j < m; j++) {
//         fscanf(file, "%d", &need[thread_id][j]);
//     }
//     struct UserRequest* head = NULL;
//     struct UserRequest* tail = NULL;
//     int delay;
//     char type;
//     while (fscanf(file, "%d %c", &delay, &type) == 2) {
//         struct UserRequest* req = malloc(sizeof(struct UserRequest));
//         req->delay = delay;
//         req->type = type;
//         req->req = type == 'Q' ? NULL : malloc(m * sizeof(int));
//         req->next = NULL;
//         if (type == 'R') {
//             for (int j = 0; j < m; j++) {
//                 fscanf(file, "%d", &req->req[j]);
//             }
//         }
//         if (!head) head = tail = req;
//         else { tail->next = req; tail = req; }
//     }
//     fclose(file);
//     return head;
// }

// void free_user_requests(struct UserRequest* head) {
//     while (head) {
//         struct UserRequest* next = head->next;
//         if (head->req) free(head->req);
//         free(head);
//         head = next;
//     }
// }

// // Banker's algorithm for deadlock avoidance
// #ifdef _DLAVOID
// int banker_algo(int** alloc, int** need, int* avail, int* req, int tid) {
//     int* temp_avail = malloc(m * sizeof(int));
//     int** temp_alloc = malloc(n * sizeof(int*));
//     int** temp_need = malloc(n * sizeof(int*));
//     for (int i = 0; i < n; i++) {
//         temp_alloc[i] = malloc(m * sizeof(int));
//         temp_need[i] = malloc(m * sizeof(int));
//         memcpy(temp_alloc[i], alloc[i], m * sizeof(int));
//         memcpy(temp_need[i], need[i], m * sizeof(int));
//     }
//     memcpy(temp_avail, avail, m * sizeof(int));

//     // Temporarily grant the request
//     for (int j = 0; j < m; j++) {
//         temp_avail[j] -= req[j];
//         temp_alloc[tid][j] += req[j];
//         temp_need[tid][j] -= req[j];
//     }

//     // Simulate resource allocation
//     int* work = malloc(m * sizeof(int));
//     memcpy(work, temp_avail, m * sizeof(int));
//     int* finish = calloc(n, sizeof(int));
//     int finished = 0;

//     while (finished < n) {
//         int found = 0;
//         for (int i = 0; i < n; i++) {
//             if (!finish[i]) {
//                 int can_finish = 1;
//                 for (int j = 0; j < m; j++) {
//                     if (temp_need[i][j] > work[j]) {
//                         can_finish = 0;
//                         break;
//                     }
//                 }
//                 if (can_finish) {
//                     for (int j = 0; j < m; j++) {
//                         work[j] += temp_alloc[i][j];
//                     }
//                     finish[i] = 1;
//                     finished++;
//                     found = 1;
//                 }
//             }
//         }
//         if (!found) {
//             free(work);
//             free(finish);
//             for (int i = 0; i < n; i++) {
//                 free(temp_alloc[i]);
//                 free(temp_need[i]);
//             }
//             free(temp_alloc);
//             free(temp_need);
//             free(temp_avail);
//             return 0; // Unsafe state
//         }
//     }

//     free(work);
//     free(finish);
//     for (int i = 0; i < n; i++) {
//         free(temp_alloc[i]);
//         free(temp_need[i]);
//     }
//     free(temp_alloc);
//     free(temp_need);
//     free(temp_avail);
//     return 1; // Safe state
// }
// #endif

// // User thread function
// void* user_thread(void* arg) {
//     struct ThreadData* data = (struct ThreadData*)arg;
//     int id = data->thread_id;
//     int* done = data->done;
//     int* pending_additional = data->pending_additional;

//     pthread_mutex_lock(&pmtx);
//     printf("\tThread %2d born\n", id);
//     pthread_mutex_unlock(&pmtx);

//     char filename[50];
//     sprintf(filename, "../input/thread%02d.txt", id);
//     struct UserRequest* requests = read_thread(filename, id);
//     if (!requests) pthread_exit(NULL);

//     pthread_barrier_wait(&BOS);

//     struct UserRequest* curr = requests;
//     int* holding = calloc(m, sizeof(int));
//     while (curr) {
//         usleep(curr->delay * 1000);

//         pthread_mutex_lock(&rmtx);
//         shared_request.thread_id = id;
//         shared_request.type = curr->type == 'Q' ? 'Q' : 'R';
//         if (curr->type == 'Q') {
//             shared_request.req = malloc(m * sizeof(int));
//             for (int j = 0; j < m; j++) {
//                 shared_request.req[j] = -holding[j];
//             }
//         } else {
//             shared_request.req = curr->req;
//             int is_additional = 0;
//             for (int j = 0; j < m; j++) {
//                 if (curr->req[j] > 0) { is_additional = 1; break; }
//             }
//             pthread_mutex_lock(&pmtx);
//             printf("\tThread %2d sends resource request: type = %s\n", id, is_additional ? "ADDITIONAL" : "RELEASE");
//             pthread_mutex_unlock(&pmtx);
//             if (is_additional) pending_additional[id] = 1;
//         }

//         pthread_barrier_wait(&REQB);
//         pthread_barrier_wait(&ACKB[id]);
//         pthread_mutex_unlock(&rmtx);

//         if (curr->type == 'R') {
//             int is_additional = pending_additional[id];
//             if (is_additional) {
//                 pthread_mutex_lock(&grant_mutex[id]);
//                 while (!done[id]) {
//                     pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
//                 }
//                 pthread_mutex_lock(&pmtx);
//                 printf("\tThread %2d is granted its last resource request\n", id);
//                 pthread_mutex_unlock(&pmtx);
//                 done[id] = 0;
//                 pending_additional[id] = 0;
//                 pthread_mutex_unlock(&grant_mutex[id]);
//             }
//             for (int j = 0; j < m; j++) {
//                 holding[j] += curr->req[j];
//             }
//         } else if (curr->type == 'Q') {
//             if (pending_additional[id]) {
//                 pthread_mutex_lock(&grant_mutex[id]);
//                 while (!done[id]) {
//                     pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
//                 }
//                 pthread_mutex_lock(&pmtx);
//                 printf("\tThread %2d is granted its last resource request\n", id);
//                 pthread_mutex_unlock(&pmtx);
//                 done[id] = 0;
//                 pending_additional[id] = 0;
//                 pthread_mutex_unlock(&grant_mutex[id]);
//             }
//             pthread_mutex_lock(&pmtx);
//             printf("\tThread %2d going to quit\n", id);
//             pthread_mutex_unlock(&pmtx);
//             break;
//         }

//         curr = curr->next;
//     }

//     free(holding);
//     free_user_requests(requests);
//     pthread_exit(NULL);
// }

// // Main function (master thread)
// int main() {
//     if (read_system("../input/system.txt")) return 1;

//     struct QueuedRequest* queue_head = NULL;
//     struct QueuedRequest* queue_tail = NULL;

//     pthread_barrier_init(&BOS, NULL, n + 1);
//     pthread_barrier_init(&REQB, NULL, 2);
//     ACKB = malloc(n * sizeof(pthread_barrier_t));
//     grant_mutex = malloc(n * sizeof(pthread_mutex_t));
//     grant_cond = malloc(n * sizeof(pthread_cond_t));
//     done = calloc(n, sizeof(int));
//     pending_additional = calloc(n, sizeof(int));
//     for (int i = 0; i < n; i++) {
//         pthread_barrier_init(&ACKB[i], NULL, 2);
//         pthread_mutex_init(&grant_mutex[i], NULL);
//         pthread_cond_init(&grant_cond[i], NULL);
//     }
//     pthread_mutex_init(&rmtx, NULL);
//     pthread_mutex_init(&pmtx, NULL);

//     shared_request.req = NULL;

//     pthread_t* threads = malloc(n * sizeof(pthread_t));
//     struct ThreadData* thread_data = malloc(n * sizeof(struct ThreadData));
//     for (int i = 0; i < n; i++) {
//         thread_data[i].thread_id = i;
//         thread_data[i].done = done;
//         thread_data[i].pending_additional = pending_additional;
//         pthread_create(&threads[i], NULL, user_thread, &thread_data[i]);
//     }

//     pthread_barrier_wait(&BOS);

//     int active_threads = n;
//     while (active_threads > 0) {
//         pthread_barrier_wait(&REQB);

//         int tid = shared_request.thread_id;
//         char type = shared_request.type;
//         int* req = malloc(m * sizeof(int));
//         memcpy(req, shared_request.req, m * sizeof(int));

//         pthread_barrier_wait(&ACKB[tid]);

//         for (int j = 0; j < m; j++) {
//             if (req[j] < 0) {
//                 available[j] -= req[j];
//                 allocation[tid][j] += req[j];
//                 need[tid][j] -= req[j];
//                 req[j] = 0;
//             }
//         }

//         if (type == 'Q') {
//             active_threads--;
//             pthread_mutex_lock(&pmtx);
//             printf("Master thread releases resources of thread %2d\n", tid);
//             if (queue_head) {
//                 printf("\t\tWaiting threads:");
//                 struct QueuedRequest* q = queue_head;
//                 while (q) {
//                     printf(" %d", q->thread_id);
//                     q = q->next;
//                 }
//                 printf("\n");
//             } else {
//                 printf("\t\tWaiting threads:\n");
//             }
//             printf("%d threads left\n", active_threads);
//             printf("Available resources: ");
//             for (int j = 0; j < m; j++) {
//                 printf("%d ", available[j]);
//             }
//             printf("\n");
//             pthread_mutex_unlock(&pmtx);
//             free(req);
//         } else {
//             int is_additional = 0;
//             for (int j = 0; j < m; j++) {
//                 if (req[j] > 0) { is_additional = 1; break; }
//             }
//             if (is_additional) {
//                 struct QueuedRequest* qr = malloc(sizeof(struct QueuedRequest));
//                 qr->thread_id = tid;
//                 qr->req = req;
//                 qr->next = NULL;
//                 if (!queue_head) queue_head = queue_tail = qr;
//                 else { queue_tail->next = qr; queue_tail = qr; }
//                 pthread_mutex_lock(&pmtx);
//                 printf("Master thread stores resource request of thread %2d\n", tid);
//                 printf("\t\tWaiting threads:");
//                 struct QueuedRequest* q = queue_head;
//                 while (q) {
//                     printf(" %d", q->thread_id);
//                     q = q->next;
//                 }
//                 printf("\n");
//                 pthread_mutex_unlock(&pmtx);
//             } else {
//                 free(req);
//             }
//         }

//         pthread_mutex_lock(&pmtx);
//         printf("Master thread tries to grant pending requests\n");
//         pthread_mutex_unlock(&pmtx);

//         struct QueuedRequest* new_head = NULL;
//         struct QueuedRequest* new_tail = NULL;
//         struct QueuedRequest* curr = queue_head;
//         queue_head = NULL;
//         queue_tail = NULL;

//         while (curr) {
//             int tid = curr->thread_id;
//             int* req = curr->req;
//             int can_grant = 1;
//             for (int j = 0; j < m; j++) {
//                 if (req[j] > available[j]) {
//                     can_grant = 0;
//                     break;
//                 }
//             }
// #ifdef _DLAVOID
//             if (can_grant) {
//                 can_grant = banker_algo(allocation, need, available, req, tid);
//             }
// #endif
//             struct QueuedRequest* next = curr->next;
//             if (can_grant) {
//                 for (int j = 0; j < m; j++) {
//                     available[j] -= req[j];
//                     allocation[tid][j] += req[j];
//                     need[tid][j] -= req[j];
//                 }
//                 pthread_mutex_lock(&pmtx);
//                 printf("Master thread grants resource request for thread %2d\n", tid);
//                 pthread_mutex_unlock(&pmtx);
//                 pthread_mutex_lock(&grant_mutex[tid]);
//                 done[tid] = 1;
//                 pthread_cond_signal(&grant_cond[tid]);
//                 pthread_mutex_unlock(&grant_mutex[tid]);
//                 free(req);
//                 free(curr);
//             } else {
//                 pthread_mutex_lock(&pmtx);
//                 printf("    +++ Insufficient resources to grant request of thread %2d\n", tid);
//                 pthread_mutex_unlock(&pmtx);
//                 curr->next = NULL;
//                 if (!new_head) new_head = new_tail = curr;
//                 else { new_tail->next = curr; new_tail = curr; }
//             }
//             curr = next;
//         }
//         queue_head = new_head;
//         queue_tail = new_tail;

//         pthread_mutex_lock(&pmtx);
//         printf("\t\tWaiting threads:");
//         struct QueuedRequest* q = queue_head;
//         while (q) {
//             printf(" %d", q->thread_id);
//             q = q->next;
//         }
//         printf("\n");
//         pthread_mutex_unlock(&pmtx);
//     }

//     for (int i = 0; i < n; i++) {
//         pthread_join(threads[i], NULL);
//         pthread_barrier_destroy(&ACKB[i]);
//         pthread_mutex_destroy(&grant_mutex[i]);
//         pthread_cond_destroy(&grant_cond[i]);
//     }

//     free(threads);
//     free(thread_data);
//     free(done);
//     free(pending_additional);
//     while (queue_head) {
//         struct QueuedRequest* temp = queue_head;
//         queue_head = queue_head->next;
//         free(temp->req);
//         free(temp);
//     }
//     pthread_barrier_destroy(&BOS);
//     pthread_barrier_destroy(&REQB);
//     pthread_mutex_destroy(&rmtx);
//     pthread_mutex_destroy(&pmtx);
//     free(ACKB);
//     free(grant_mutex);
//     free(grant_cond);
//     if (shared_request.req) free(shared_request.req);
//     free(available);
//     for (int i = 0; i < n; i++) {
//         free(need[i]);
//         free(allocation[i]);
//     }
//     free(need);
//     free(allocation);

//     return 0;
// }



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Structure for user thread requests
struct UserRequest {
    int delay;
    char type;
    int* req;
    struct UserRequest* next;
};

// Structure for queued requests
struct QueuedRequest {
    int thread_id;
    int* req;
    struct QueuedRequest* next;
};

// Global synchronization tools
pthread_barrier_t BOS;
pthread_barrier_t REQB;
pthread_barrier_t* ACKB;
pthread_mutex_t rmtx;
pthread_mutex_t pmtx;
pthread_mutex_t* grant_mutex;
pthread_cond_t* grant_cond;
int* done;
int* pending_additional;

// Global resource state
int m, n;
int* available;
int** need;
int** allocation;

// Shared memory for requests
struct Request {
    char type;
    int thread_id;
    int* req;
} shared_request;

// Struct for thread data
struct ThreadData {
    int thread_id;
    int* done;
    int* pending_additional;
};

// Reads system.txt
int read_system(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 1;
    if (fscanf(file, "%d %d", &m, &n) != 2) { fclose(file); return 1; }
    available = malloc(m * sizeof(int));
    need = malloc(n * sizeof(int*));
    allocation = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        need[i] = malloc(m * sizeof(int));
        allocation[i] = malloc(m * sizeof(int));
        memset(allocation[i], 0, m * sizeof(int));
    }
    for (int j = 0; j < m; j++) {
        fscanf(file, "%d", &available[j]);
    }
    fclose(file);
    printf("System initialized: m=%d, n=%d\n", m, n);
    return 0;
}

// Reads thread file
struct UserRequest* read_thread(const char* filename, int thread_id) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    for (int j = 0; j < m; j++) {
        fscanf(file, "%d", &need[thread_id][j]);
    }
    struct UserRequest* head = NULL;
    struct UserRequest* tail = NULL;
    int delay;
    char type;
    while (fscanf(file, "%d %c", &delay, &type) == 2) {
        struct UserRequest* req = malloc(sizeof(struct UserRequest));
        req->delay = delay;
        req->type = type;
        req->req = type == 'Q' ? NULL : malloc(m * sizeof(int));
        req->next = NULL;
        if (type == 'R') {
            for (int j = 0; j < m; j++) {
                fscanf(file, "%d", &req->req[j]);
            }
        }
        if (!head) head = tail = req;
        else { tail->next = req; tail = req; }
    }
    fclose(file);
    return head;
}

void free_user_requests(struct UserRequest* head) {
    while (head) {
        struct UserRequest* next = head->next;
        if (head->req) free(head->req);
        free(head);
        head = next;
    }
}

// Banker's algorithm for deadlock avoidance
#ifdef _DLAVOID
int banker_algo(int** alloc, int** need, int* avail, int* req, int tid) {
    int* temp_avail = malloc(m * sizeof(int));
    int** temp_alloc = malloc(n * sizeof(int*));
    int** temp_need = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        temp_alloc[i] = malloc(m * sizeof(int));
        temp_need[i] = malloc(m * sizeof(int));
        memcpy(temp_alloc[i], alloc[i], m * sizeof(int));
        memcpy(temp_need[i], need[i], m * sizeof(int));
    }
    memcpy(temp_avail, avail, m * sizeof(int));

    for (int j = 0; j < m; j++) {
        temp_avail[j] -= req[j];
        temp_alloc[tid][j] += req[j];
        temp_need[tid][j] -= req[j];
    }

    int* work = malloc(m * sizeof(int));
    memcpy(work, temp_avail, m * sizeof(int));
    int* finish = calloc(n, sizeof(int));
    int finished = 0;

    while (finished < n) {
        int found = 0;
        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                int can_finish = 1;
                for (int j = 0; j < m; j++) {
                    if (temp_need[i][j] > work[j]) {
                        can_finish = 0;
                        break;
                    }
                }
                if (can_finish) {
                    for (int j = 0; j < m; j++) {
                        work[j] += temp_alloc[i][j];
                    }
                    finish[i] = 1;
                    finished++;
                    found = 1;
                }
            }
        }
        if (!found) {
            free(work);
            free(finish);
            for (int i = 0; i < n; i++) {
                free(temp_alloc[i]);
                free(temp_need[i]);
            }
            free(temp_alloc);
            free(temp_need);
            free(temp_avail);
            return 0;
        }
    }

    free(work);
    free(finish);
    for (int i = 0; i < n; i++) {
        free(temp_alloc[i]);
        free(temp_need[i]);
    }
    free(temp_alloc);
    free(temp_need);
    free(temp_avail);
    return 1;
}
#endif

// User thread function
void* user_thread(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    int id = data->thread_id;
    int* done = data->done;
    int* pending_additional = data->pending_additional;

    pthread_mutex_lock(&pmtx);
    printf("\tThread %2d born\n", id);
    pthread_mutex_unlock(&pmtx);

    char filename[50];
    sprintf(filename, "../input/thread%02d.txt", id);
    struct UserRequest* requests = read_thread(filename, id);
    if (!requests) pthread_exit(NULL);

    pthread_barrier_wait(&BOS);

    struct UserRequest* curr = requests;
    int* holding = calloc(m, sizeof(int));
    while (curr) {
        usleep(curr->delay * 1000);

        pthread_mutex_lock(&rmtx);
        shared_request.thread_id = id;
        shared_request.type = curr->type == 'Q' ? 'Q' : 'R';
        if (curr->type == 'Q') {
            shared_request.req = malloc(m * sizeof(int));
            for (int j = 0; j < m; j++) {
                shared_request.req[j] = -holding[j];
            }
        } else {
            shared_request.req = curr->req;
            int is_additional = 0;
            for (int j = 0; j < m; j++) {
                if (curr->req[j] > 0) { is_additional = 1; break; }
            }
            pthread_mutex_lock(&pmtx);
            printf("\tThread %2d sends resource request: type = %s\n", id, is_additional ? "ADDITIONAL" : "RELEASE");
            pthread_mutex_unlock(&pmtx);
            if (is_additional) pending_additional[id] = 1;
        }

        pthread_barrier_wait(&REQB);
        pthread_barrier_wait(&ACKB[id]);
        pthread_mutex_unlock(&rmtx);

        if (curr->type == 'R') {
            int is_additional = pending_additional[id];
            if (is_additional) {
                pthread_mutex_lock(&grant_mutex[id]);
                while (!done[id]) {
                    pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
                }
                pthread_mutex_lock(&pmtx);
                printf("\tThread %2d is granted its last resource request\n", id);
                pthread_mutex_unlock(&pmtx);
                done[id] = 0;
                pending_additional[id] = 0;
                pthread_mutex_unlock(&grant_mutex[id]);
            } else {
                pthread_mutex_lock(&pmtx);
                printf("\tThread %2d is done with its resource release request\n", id);
                pthread_mutex_unlock(&pmtx);
            }
            for (int j = 0; j < m; j++) {
                holding[j] += curr->req[j];
            }
        } else if (curr->type == 'Q') {
            if (pending_additional[id]) {
                pthread_mutex_lock(&grant_mutex[id]);
                while (!done[id]) {
                    pthread_cond_wait(&grant_cond[id], &grant_mutex[id]);
                }
                pthread_mutex_lock(&pmtx);
                printf("\tThread %2d is granted its last resource request\n", id);
                pthread_mutex_unlock(&pmtx);
                done[id] = 0;
                pending_additional[id] = 0;
                pthread_mutex_unlock(&grant_mutex[id]);
            }
            pthread_mutex_lock(&pmtx);
            printf("\tThread %2d going to quit\n", id);
            pthread_mutex_unlock(&pmtx);
            break;
        }

        curr = curr->next;
    }

    free(holding);
    free_user_requests(requests);
    pthread_exit(NULL);
}

//master thread
int main() {
    if (read_system("../input/system.txt")) return 1;

    struct QueuedRequest* queue_head = NULL;
    struct QueuedRequest* queue_tail = NULL;

    pthread_barrier_init(&BOS, NULL, n + 1);
    pthread_barrier_init(&REQB, NULL, 2);
    ACKB = malloc(n * sizeof(pthread_barrier_t));
    grant_mutex = malloc(n * sizeof(pthread_mutex_t));
    grant_cond = malloc(n * sizeof(pthread_cond_t));
    done = calloc(n, sizeof(int));
    pending_additional = calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        pthread_barrier_init(&ACKB[i], NULL, 2);
        pthread_mutex_init(&grant_mutex[i], NULL);
        pthread_cond_init(&grant_cond[i], NULL);
    }
    pthread_mutex_init(&rmtx, NULL);
    pthread_mutex_init(&pmtx, NULL);

    shared_request.req = NULL;

    pthread_t* threads = malloc(n * sizeof(pthread_t));
    struct ThreadData* thread_data = malloc(n * sizeof(struct ThreadData));
    for (int i = 0; i < n; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].done = done;
        thread_data[i].pending_additional = pending_additional;
        pthread_create(&threads[i], NULL, user_thread, &thread_data[i]);
    }

    pthread_barrier_wait(&BOS);

    int active_threads = n;
    while (active_threads > 0) {
        pthread_barrier_wait(&REQB);

        int tid = shared_request.thread_id;
        char type = shared_request.type;
        int* req = malloc(m * sizeof(int));
        memcpy(req, shared_request.req, m * sizeof(int));

        pthread_barrier_wait(&ACKB[tid]);

        for (int j = 0; j < m; j++) {
            if (req[j] < 0) {
                available[j] -= req[j];
                allocation[tid][j] += req[j];
                need[tid][j] -= req[j];
                req[j] = 0;
            }
        }

        if (type == 'Q') {
            active_threads--;
            pthread_mutex_lock(&pmtx);
            printf("Master thread releases resources of thread %2d\n", tid);
            if (queue_head) {
                printf("\t\tWaiting threads:");
                struct QueuedRequest* q = queue_head;
                while (q) {
                    printf(" %d", q->thread_id);
                    q = q->next;
                }
                printf("\n");
            } else {
                printf("\t\tWaiting threads:\n");
            }
            printf("%d threads left\n", active_threads);
            printf("Available resources: ");
            for (int j = 0; j < m; j++) {
                printf("%d ", available[j]);
            }
            printf("\n");
            pthread_mutex_unlock(&pmtx);
            free(req);
        } else {
            int is_additional = 0;
            for (int j = 0; j < m; j++) {
                if (req[j] > 0) { is_additional = 1; break; }
            }
            if (is_additional) {
                struct QueuedRequest* qr = malloc(sizeof(struct QueuedRequest));
                qr->thread_id = tid;
                qr->req = req;
                qr->next = NULL;
                if (!queue_head) queue_head = queue_tail = qr;
                else { queue_tail->next = qr; queue_tail = qr; }
                pthread_mutex_lock(&pmtx);
                printf("Master thread stores resource request of thread %2d\n", tid);
                printf("\t\tWaiting threads:");
                struct QueuedRequest* q = queue_head;
                while (q) {
                    printf(" %d", q->thread_id);
                    q = q->next;
                }
                printf("\n");
                pthread_mutex_unlock(&pmtx);
            } else {
                free(req);
            }
        }

        pthread_mutex_lock(&pmtx);
        printf("Master thread tries to grant pending requests\n");
        pthread_mutex_unlock(&pmtx);

        struct QueuedRequest* new_head = NULL;
        struct QueuedRequest* new_tail = NULL;
        struct QueuedRequest* curr = queue_head;
        queue_head = NULL;
        queue_tail = NULL;

        while (curr) {
            int tid = curr->thread_id;
            int* req = curr->req;
            int can_grant = 1;
            for (int j = 0; j < m; j++) {
                if (req[j] > available[j]) {
                    can_grant = 0;
                    break;
                }
            }
#ifdef _DLAVOID
            if (can_grant) {
                can_grant = banker_algo(allocation, need, available, req, tid);
            }
#endif
            struct QueuedRequest* next = curr->next;
            if (can_grant) {
                for (int j = 0; j < m; j++) {
                    available[j] -= req[j];
                    allocation[tid][j] += req[j];
                    need[tid][j] -= req[j];
                }
                pthread_mutex_lock(&pmtx);
                printf("Master thread grants resource request for thread %2d\n", tid);
                pthread_mutex_unlock(&pmtx);
                pthread_mutex_lock(&grant_mutex[tid]);
                done[tid] = 1;
                pthread_cond_signal(&grant_cond[tid]);
                pthread_mutex_unlock(&grant_mutex[tid]);
                free(req);
                free(curr);
            } else {
                pthread_mutex_lock(&pmtx);
                printf("    +++ Insufficient resources to grant request of thread %2d\n", tid);
                pthread_mutex_unlock(&pmtx);
                curr->next = NULL;
                if (!new_head) new_head = new_tail = curr;
                else { new_tail->next = curr; new_tail = curr; }
            }
            curr = next;
        }
        queue_head = new_head;
        queue_tail = new_tail;

        pthread_mutex_lock(&pmtx);
        printf("\t\tWaiting threads:");
        struct QueuedRequest* q = queue_head;
        while (q) {
            printf(" %d", q->thread_id);
            q = q->next;
        }
        printf("\n");
        pthread_mutex_unlock(&pmtx);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
        pthread_barrier_destroy(&ACKB[i]);
        pthread_mutex_destroy(&grant_mutex[i]);
        pthread_cond_destroy(&grant_cond[i]);
    }

    free(threads);
    free(thread_data);
    free(done);
    free(pending_additional);
    while (queue_head) {
        struct QueuedRequest* temp = queue_head;
        queue_head = queue_head->next;
        free(temp->req);
        free(temp);
    }
    pthread_barrier_destroy(&BOS);
    pthread_barrier_destroy(&REQB);
    pthread_mutex_destroy(&rmtx);
    pthread_mutex_destroy(&pmtx);
    free(ACKB);
    free(grant_mutex);
    free(grant_cond);
    if (shared_request.req) free(shared_request.req);
    free(available);
    for (int i = 0; i < n; i++) {
        free(need[i]);
        free(allocation[i]);
    }
    free(need);
    free(allocation);

    return 0;
}