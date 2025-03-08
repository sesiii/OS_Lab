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
    int cook_queue[602];
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

void wmain(int id) {
    char waiter_name = 'U' + id;
    sem_wait(semid, 0);
    print_time(M->time);
    printf(" Waiter %c is ready\n", waiter_name);
    fflush(stdout);
    sem_signal(semid, 0);

    int front = 0;

    while (1) {
        sem_wait(semid, 2 + id); // waiter_sem[id]

        sem_wait(semid, 0);
        if (M->time > 240 && M->waiter_queues[id][198] == 0 && M->waiter_queues[id][197] == 0) {
            print_time(M->time);
            printf(" Waiter %c leaving (no more customers to serve)\n", waiter_name);
            fflush(stdout);
            sem_signal(semid, 0);
            break;
        }

        if (M->waiter_queues[id][197] > 0) { // Food ready to serve
            int cust_id = M->waiter_queues[id][197];
            M->waiter_queues[id][197] = 0;
            print_time(M->time);
            printf(" Waiter %c: Serving food to Customer %d\n", waiter_name, cust_id);
            fflush(stdout);
            sem_signal(semid, 0);
            sem_signal(semid, 7 + cust_id - 1); // Signal customer to eat
        } else if (M->waiter_queues[id][198] > 0) { // Place order
            int cust_id = M->waiter_queues[id][front];
            int count = M->waiter_queues[id][front + 1];
            front += 2;
            M->waiter_queues[id][198]--;

            print_time(M->time);
            printf(" Waiter %c: Placing order for Customer %d (count = %d)\n",
                   waiter_name, cust_id, count);
            fflush(stdout);
            sem_signal(semid, 0);

            int curr_time = M->time;
            usleep(100000); // 1 min
            sem_wait(semid, 0);
            if (curr_time + 1 > M->time)
                M->time = curr_time + 1;

            int back = M->cook_queue[601];
            M->cook_queue[back] = id;
            M->cook_queue[back + 1] = cust_id;
            M->cook_queue[back + 2] = count;
            M->cook_queue[601] += 3;
            M->pending_orders++;
            sem_signal(semid, 0);
            sem_signal(semid, 1); // Signal cook
        } else {
            sem_signal(semid, 0);
        }
    }
}

int main() {
    signal(SIGINT, cleanup);

    key_t key = ftok("cook.c", 65);
    shmid = shmget(key, sizeof(struct shared_memory), 0666);
    if (shmid == -1) { perror("shmget failed"); exit(1); }
    M = shmat(shmid, NULL, 0);
    if (M == (void *)-1) { perror("shmat failed"); exit(1); }
    semid = semget(key, 262, 0666);
    if (semid == -1) { perror("semget failed"); exit(1); }

    for (int i = 0; i < 5; i++) {
        if (fork() == 0) {
            wmain(i);
            exit(0);
        }
    }
    for (int i = 0; i < 5; i++)
        wait(NULL);
    return 0;
}