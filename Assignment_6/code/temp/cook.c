#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

struct shared_memory {
    int time;
    int empty_tables;
    int next_waiter;
    int pending_orders;
    int waiter_queues[5][200]; // [i][0-196] = queue, [i][197] = FR, [i][198] = PO, [i][199] = back
    int cook_queue[602];       // [600] = front, [601] = back
};

struct shared_memory *M;
int shmid, semid;

void sem_wait(int sem_id, int sem_num) {
    struct sembuf sb = {sem_num, -1, 0};
    semop(sem_id, &sb, 1);
}

void sem_signal(int sem_id, int sem_num) {
    struct sembuf sb = {sem_num, 1, 0};
    semop(sem_id, &sb, 1);
}

void print_time(int minutes) {
    int hour = 11 + (minutes / 60);
    int min = minutes % 60;
    char *period = (hour >= 12) ? "pm" : "am";
    if (hour > 12) hour -= 12;
    printf("[%02d:%02d %s]", hour, min, period);
}

void cleanup(int signum) {
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    exit(0);
}

void cmain(int cook_id) {
    char cook_name = (cook_id == 0) ? 'C' : 'D';
    sem_wait(semid, 0);
    print_time(M->time);
    printf(" Cook %c is ready\n", cook_name);
    fflush(stdout);
    sem_signal(semid, 0);

    while (1) {
        sem_wait(semid, 1); // cook_sem

        sem_wait(semid, 0);
        if (M->time > 240 && M->pending_orders == 0 && M->cook_queue[600] >= M->cook_queue[601]) {
            sem_signal(semid, 0);
            break;
        }
        int front = M->cook_queue[600];
        int back = M->cook_queue[601];
        if (front >= back || M->pending_orders <= 0) { // Queue empty or no orders
            sem_signal(semid, 0);
            sem_signal(semid, 1); // Re-signal cook_sem to avoid deadlock
            continue;
        }
        int waiter_id = M->cook_queue[front];
        int cust_id = M->cook_queue[front + 1];
        int count = M->cook_queue[front + 2];
        M->cook_queue[600] += 3;
        M->pending_orders--;

        print_time(M->time);
        printf(" Cook %c: Preparing order (Waiter %c, Customer %d, Count %d)\n",
               cook_name, 'U' + waiter_id, cust_id, count);
        fflush(stdout);
        sem_signal(semid, 0);

        int curr_time = M->time;
        usleep(count * 500000); // 5 min × 100ms
        sem_wait(semid, 0);
        if (curr_time + (count * 5) > M->time)
            M->time = curr_time + (count * 5);
        print_time(M->time);
        printf(" Cook %c: Prepared order (Waiter %c, Customer %d, Count %d)\n",
               cook_name, 'U' + waiter_id, cust_id, count);
        fflush(stdout);

        M->waiter_queues[waiter_id][197] = cust_id; // Set FR cell
        sem_signal(semid, 0);

        sem_signal(semid, 2 + waiter_id); // Signal waiter
    }

    sem_wait(semid, 0);
    print_time(M->time);
    printf(" Cook %c: Leaving\n", cook_name);
    fflush(stdout);
    sem_signal(semid, 0);

    if (cook_id == 1) { // Last cook wakes waiters
        for (int i = 0; i < 5; i++)
            sem_signal(semid, 2 + i);
    }
}

int main() {
    signal(SIGINT, cleanup);

    key_t key = ftok("cook.c", 65);
    shmid = shmget(key, sizeof(struct shared_memory), 0666 | IPC_CREAT);
    if (shmid == -1) { perror("shmget failed"); exit(1); }
    M = shmat(shmid, NULL, 0);
    if (M == (void *)-1) { perror("shmat failed"); exit(1); }

    semid = semget(key, 262, 0666 | IPC_CREAT);
    if (semid == -1) { perror("semget failed"); exit(1); }

    M->time = 0;
    M->empty_tables = 10;
    M->next_waiter = 0;
    M->pending_orders = 0;
    for (int i = 0; i < 5; i++) {
        M->waiter_queues[i][199] = 0;
        M->waiter_queues[i][198] = 0;
        M->waiter_queues[i][197] = 0;
    }
    M->cook_queue[600] = 0;
    M->cook_queue[601] = 0;

    semctl(semid, 0, SETVAL, 1);  // mutex
    semctl(semid, 1, SETVAL, 0);  // cook_sem
    for (int i = 0; i < 5; i++)
        semctl(semid, 2 + i, SETVAL, 0); // waiter_sem
    for (int i = 0; i < 256; i++)
        semctl(semid, 7 + i, SETVAL, 0); // customer_sem

    for (int i = 0; i < 2; i++) {
        if (fork() == 0) {
            cmain(i);
            exit(0);
        }
    }
    wait(NULL);
    wait(NULL);
    return 0;
}