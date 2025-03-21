#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables for system state (will be local to master thread later)
int m;              // Number of resource types (5 to 20)
int n;              // Number of threads (10 to 100)
int* AVAILABLE;     // Array of available resource instances
int** NEED;         // 2D array of maximum remaining needs per thread
int** ALLOC;        // 2D array of currently allocated resources per thread

// Structure for user thread requests (local to each thread)
struct UserRequest {
    int delay;              // Time to wait before this request (milliseconds)
    char type;              // 'R' for request/release, 'Q' for quit
    int* req;               // Array of m values (NULL for Q)
    struct UserRequest* next; // Next request in sequence
};

// Function to read system.txt and initialize system state and memory allocation fpr NEED and ALLOC
int read_system(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening %s\n", filename);
        return 1;
    }

    if (fscanf(file, "%d %d", &m, &n) != 2) {
        printf("Error: Failed to read m and n from %s\n", filename);
        fclose(file);
        return 1;
    }
    if (m < 5 || m > 20 || n < 10 || n > 100) {
        printf("Error: Invalid m=%d or n=%d (m must be 5-20, n must be 10-100)\n", m, n);
        fclose(file);
        return 1;
    }

    AVAILABLE = (int*)malloc(m * sizeof(int));
    if (!AVAILABLE) {
        printf("Error: Memory allocation failed for AVAILABLE\n");
        fclose(file);
        return 1;
    }

    for (int j = 0; j < m; j++) {
        if (fscanf(file, "%d", &AVAILABLE[j]) != 1) {
            printf("Error: Failed to read resource %d in %s\n", j, filename);
            free(AVAILABLE);
            fclose(file);
            return 1;
        }
        if (AVAILABLE[j] < 0) {
            printf("Error: Negative resource count %d for resource %d\n", AVAILABLE[j], j);
            free(AVAILABLE);
            fclose(file);
            return 1;
        }
    }

    NEED = (int**)malloc(n * sizeof(int*));
    ALLOC = (int**)malloc(n * sizeof(int*));
    if (!NEED || !ALLOC) {
        printf("Error: Memory allocation failed for NEED or ALLOC\n");
        free(AVAILABLE);
        if (NEED) free(NEED);
        if (ALLOC) free(ALLOC);
        fclose(file);
        return 1;
    }
    for (int i = 0; i < n; i++) {
        NEED[i] = (int*)malloc(m * sizeof(int));
        ALLOC[i] = (int*)malloc(m * sizeof(int));
        if (!NEED[i] || !ALLOC[i]) {
            printf("Error: Memory allocation failed for thread %d\n", i);
            fclose(file);
            return 1;
        }
        memset(ALLOC[i], 0, m * sizeof(int));
    }

    fclose(file);
    return 0;
}

// Updated function to read a thread's file
struct UserRequest* read_thread(const char* filename, int thread_id, int* max_needs) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open %s\n", filename);
        return NULL;
    }

    // Read maximum needs
    for (int j = 0; j < m; j++) {
        if (fscanf(file, "%d", &max_needs[j]) != 1) {
            printf("Error: Failed to read max need %d for thread %d in %s\n", j, thread_id, filename);
            fclose(file);
            return NULL;
        }
        if (max_needs[j] < 0) {
            printf("Error: Negative max need %d for resource %d in %s\n", max_needs[j], j, filename);
            fclose(file);
            return NULL;
        }
    }

    // Build request list
    struct UserRequest* head = NULL;
    struct UserRequest* tail = NULL;

    int delay;
    char type;
    while (fscanf(file, "%d %c", &delay, &type) == 2) {
        struct UserRequest* req = (struct UserRequest*)malloc(sizeof(struct UserRequest));
        if (!req) {
            printf("Error: Memory allocation failed for request in %s\n", filename);
            fclose(file);
            return NULL;
        }
        req->delay = delay;
        req->type = type;
        req->next = NULL;

        if (type == 'Q') {
            req->req = NULL;
        } else if (type == 'R') {
            req->req = (int*)malloc(m * sizeof(int));
            if (!req->req) {
                printf("Error: Memory allocation failed for request vector in %s\n", filename);
                free(req);
                fclose(file);
                return NULL;
            }
            for (int j = 0; j < m; j++) {
                if (fscanf(file, "%d", &req->req[j]) != 1) {
                    printf("Error: Failed to read request %d for thread %d in %s\n", j, thread_id, filename);
                    free(req->req);
                    free(req);
                    fclose(file);
                    return NULL;
                }
            }
        } else {
            printf("Error: Unknown request type '%c' in %s\n", type, filename);
            free(req);
            fclose(file);
            return NULL;
        }

        if (!head) {
            head = tail = req;
        } else {
            tail->next = req;
            tail = req;
        }
    }

    fclose(file);
    return head;
}

// Function to free UserRequest list
void free_user_requests(struct UserRequest* head) {
    while (head) {
        struct UserRequest* next = head->next;
        if (head->req) free(head->req);
        free(head);
        head = next;
    }
}

// Main function to test
int main() {
    printf("Assuming input/ directory exists with files from geninput.c\n");
    printf("Run 'gcc -Wall -o geninput geninput.c && ./geninput 20 10' if needed\n");

    if (read_system("../input/system.txt") != 0) {
        printf("Failed to read ../input/system.txt\n");
        return 1;
    }
    printf("System loaded: m=%d, n=%d, AVAILABLE=", m, n);
    for (int j = 0; j < m; j++) {
        printf("%d ", AVAILABLE[j]);
    }
    printf("\n");

    int* max_needs = (int*)malloc(m * sizeof(int));
    if (!max_needs) {
        printf("Error: Memory allocation failed for max_needs\n");
        return 1;
    }
    char filename[50];
    sprintf(filename, "../input/thread%02d.txt", 1);
    struct UserRequest* requests = read_thread(filename, 1, max_needs);
    if (!requests) {
        printf("Failed to read %s\n", filename);
        free(max_needs);
        free(AVAILABLE);
        for (int i = 0; i < n; i++) {
            free(NEED[i]);
            free(ALLOC[i]);
        }
        free(NEED);
        free(ALLOC);
        return 1;
    }

    memcpy(NEED[0], max_needs, m * sizeof(int));
    printf("Thread 0 max needs: ");
    for (int j = 0; j < m; j++) {
        printf("%d ", NEED[0][j]);
    }
    printf("\n");

    printf("Thread 0 requests:\n");
    struct UserRequest* curr = requests;
    while (curr) {
        printf("  Delay: %d, Type: %c", curr->delay, curr->type);
        if (curr->req) {
            printf(", Req=");
            for (int j = 0; j < m; j++) {
                printf("%d ", curr->req[j]);
            }
        }
        printf("\n");
        curr = curr->next;
    }

    free_user_requests(requests);
    free(max_needs);
    free(AVAILABLE);
    for (int i = 0; i < n; i++) {
        free(NEED[i]);
        free(ALLOC[i]);
    }
    free(NEED);
    free(ALLOC);

    printf("Test completed successfully\n");
    return 0;
}