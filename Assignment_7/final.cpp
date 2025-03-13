#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>


int m, n; // m boats, n visitors remaining
pthread_mutex_t bmtx; 
pthread_cond_t *boat_cond; 
pthread_mutex_t *boat_mutex; 
pthread_mutex_t status_mutex; 
int *BA, *BC, *BT; // Boat availability, capacity, visitor ID assigned
pthread_barrier_t EOS; 
int total_visitors;  
int simulation_done = 0; 
int visitors_served = 0; 



void *boat_thread(void *arg) {
    int boat_id = *((int *)arg);
    free(arg);

    while (1) {
        
        pthread_mutex_lock(&status_mutex);
        if (simulation_done) {
            pthread_mutex_unlock(&status_mutex);
            break;
        }
        pthread_mutex_unlock(&status_mutex);
        

        pthread_mutex_lock(&bmtx);
        BA[boat_id] = 1;
        BT[boat_id] = -1;
        printf("Boat    %d ready\n", boat_id);
        pthread_mutex_unlock(&bmtx);

        
        pthread_mutex_lock(&boat_mutex[boat_id]);
        while (BT[boat_id] == -1) {
            
            pthread_mutex_lock(&status_mutex);
            if (simulation_done) {
                pthread_mutex_unlock(&status_mutex);
                pthread_mutex_unlock(&boat_mutex[boat_id]);
                goto exit_boat;
            }
            pthread_mutex_unlock(&status_mutex);
            
            // Used a timed wait to avoid permanent deadlock
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            int ret = pthread_cond_timedwait(&boat_cond[boat_id], &boat_mutex[boat_id], &ts);
            
            if (ret == ETIMEDOUT) {
                pthread_mutex_lock(&status_mutex);
                if (simulation_done) {
                    pthread_mutex_unlock(&status_mutex);
                    pthread_mutex_unlock(&boat_mutex[boat_id]);
                    goto exit_boat;
                }
                pthread_mutex_unlock(&status_mutex);
                continue;
            }
        }
        
        int visitor_id = BT[boat_id];
        printf("Boat    %d start of ride for visitor %d\n", boat_id, visitor_id);
        pthread_mutex_unlock(&boat_mutex[boat_id]);

        int rtime = rand() % 5 + 1;
        sleep(rtime); 

        
        printf("Boat    %d end of ride for visitor %d (ride time = %d)\n", boat_id, visitor_id, rtime);
        
        pthread_mutex_lock(&boat_mutex[boat_id]);
        BT[boat_id] = -1;
        pthread_cond_signal(&boat_cond[boat_id]); 
        pthread_mutex_unlock(&boat_mutex[boat_id]);

        pthread_mutex_lock(&status_mutex);
        visitors_served++;
        int remaining = total_visitors - visitors_served;
        // printf("No of visitors remaining: %d\n", remaining);
        
        if (remaining == 0) {
            printf("All visitors served\n");
            simulation_done = 1;
            pthread_mutex_unlock(&status_mutex);
            break;
        }
        pthread_mutex_unlock(&status_mutex);
    }

exit_boat:
    printf("Boat %d: No more visitors left, finished serving visitors\n", boat_id);
    return NULL;
}

void *visitor_thread(void *arg) {
    int visitor_id = *((int *)arg);
    free(arg);

    srand(time(NULL) ^ (visitor_id + 1));
    int vtime = rand() % 5 + 1;
    int rtime = rand() % 5 + 1; 
    
    printf("Visitor %d Starts sightseeing for %d seconds\n", visitor_id, vtime);
    sleep(vtime);

    pthread_mutex_lock(&status_mutex);
    if (simulation_done) {
        pthread_mutex_unlock(&status_mutex);
        printf("Visitor %d: Simulation ended before ride\n", visitor_id);
        pthread_barrier_wait(&EOS);
        return NULL;
    }
    pthread_mutex_unlock(&status_mutex);

    // Find an available boat
    int boat_id = -1;
    printf("Visitor %d Ready to ride a boat (ride time = %d)\n", visitor_id, rtime);
    
    while (boat_id == -1) {
        // Check if simulation is done
        pthread_mutex_lock(&status_mutex);
        if (simulation_done) {
            pthread_mutex_unlock(&status_mutex);
            printf("Visitor %d: Simulation ended while waiting for a boat\n", visitor_id);
            pthread_barrier_wait(&EOS);
            return NULL;
        }
        pthread_mutex_unlock(&status_mutex);
        
        
        pthread_mutex_lock(&bmtx);
        for (int i = 0; i < m; i++) {
            if (BA[i] == 1) {
                BA[i] = 0; //boat busy
                BT[i] = visitor_id; 
                boat_id = i;
                pthread_mutex_lock(&boat_mutex[boat_id]);
                pthread_cond_signal(&boat_cond[boat_id]); 
                pthread_mutex_unlock(&boat_mutex[boat_id]);
                break;
            }
        }
        pthread_mutex_unlock(&bmtx);
        
        if (boat_id == -1) {
            // No boat available, wait a bit before trying again
            usleep(100000); // 100ms wait
        }
    }

    printf("Visitor %d Finds Boat %d\n", visitor_id, boat_id);

    // Wait for ride to complete
    pthread_mutex_lock(&boat_mutex[boat_id]);
    while (BT[boat_id] == visitor_id) {
        // Use a timed wait to avoid permanent deadlock
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 2; 
        int ret = pthread_cond_timedwait(&boat_cond[boat_id], &boat_mutex[boat_id], &ts);
        
        // If timeout, checks if we're still assigned to the boat
        if (ret == ETIMEDOUT) {
            pthread_mutex_lock(&status_mutex);
            if (simulation_done) {
                pthread_mutex_unlock(&status_mutex);
                pthread_mutex_unlock(&boat_mutex[boat_id]);
                goto exit_visitor;
            }
            pthread_mutex_unlock(&status_mutex);
        }
    }
    pthread_mutex_unlock(&boat_mutex[boat_id]);
    
    printf("Visitor %d leaving\n", visitor_id);
    
exit_visitor:
    pthread_barrier_wait(&EOS);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <m> <n>\n", argv[0]);
        return 1;
    }

    m = atoi(argv[1]);
    total_visitors = n = atoi(argv[2]);
    printf("m = %d, n = %d\n", m, n);

    pthread_t *bid, *vid;
    bid = (pthread_t *)malloc(m * sizeof(pthread_t));
    vid = (pthread_t *)malloc(n * sizeof(pthread_t));

    pthread_mutex_init(&bmtx, NULL);
    pthread_mutex_init(&status_mutex, NULL);
    BA = (int *)calloc(m, sizeof(int));
    BC = (int *)calloc(m, sizeof(int));
    BT = (int *)calloc(m, sizeof(int));
    boat_cond = (pthread_cond_t *)malloc(m * sizeof(pthread_cond_t));
    boat_mutex = (pthread_mutex_t *)malloc(m * sizeof(pthread_mutex_t));

    for (int i = 0; i < m; i++) {
        BA[i] = 1;
        BC[i] = 1;
        BT[i] = -1;
        pthread_cond_init(&boat_cond[i], NULL);
        pthread_mutex_init(&boat_mutex[i], NULL);
    }

    pthread_barrier_init(&EOS, NULL, n + 1);

    printf("Main thread initialized\n");

    for (int i = 0; i < m; i++) {
        int *boat_id = malloc(sizeof(int));
        *boat_id = i;
        pthread_create(&bid[i], NULL, boat_thread, boat_id);
    }

    for (int i = 0; i < n; i++) {
        int *visitor_id = malloc(sizeof(int));
        *visitor_id = i;
        pthread_create(&vid[i], NULL, visitor_thread, visitor_id);
    }
    // Wait for all visitors to complete
    pthread_barrier_wait(&EOS);
    
    pthread_mutex_lock(&status_mutex);
    simulation_done = 1;
    pthread_mutex_unlock(&status_mutex);
    
    // printf("Simulation complete, signaling threads to exit...\n");

    for (int i = 0; i < m; i++) {
        pthread_mutex_lock(&boat_mutex[i]);
        pthread_cond_broadcast(&boat_cond[i]);
        pthread_mutex_unlock(&boat_mutex[i]);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(vid[i], NULL);
    }
    
    for (int i = 0; i < m; i++) {
        pthread_join(bid[i], NULL);
    }

    printf("Main thread terminating\n");

    pthread_mutex_destroy(&bmtx);
    pthread_mutex_destroy(&status_mutex);
    
    for (int i = 0; i < m; i++) {
        pthread_cond_destroy(&boat_cond[i]);
        pthread_mutex_destroy(&boat_mutex[i]);
    }
    
    free(boat_cond);
    free(boat_mutex);
    free(BA);
    free(BC);
    free(BT);
    free(bid);
    free(vid);
    pthread_barrier_destroy(&EOS);

    return 0;
}