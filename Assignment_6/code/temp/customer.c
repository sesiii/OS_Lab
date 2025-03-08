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

void cmain(int id, int arrival, int count) {
    sem_wait(semid, 0);
    if (arrival > 240) {
        print_time(arrival);
        printf(" Customer %d leaves (late arrival)\n", id);
        fflush(stdout);
        sem_signal(semid, 0);
        return;
    }
    if (M->time < arrival) M->time = arrival;
    print_time(M->time);
    printf(" Customer %d arrives (count = %d)\n", id, count);
    fflush(stdout);

    if (M->empty_tables == 0) {
        print_time(M->time);
        printf(" Customer %d leaves (no empty table)\n", id);
        fflush(stdout);
        sem_signal(semid, 0);
        return;
    }
    M->empty_tables--;
    int waiter_id = M->next_waiter;
    M->next_waiter = (M->next_waiter + 1) % 5;

    int back = M->waiter_queues[waiter_id][199];
    M->waiter_queues[waiter_id][back] = id;
    M->waiter_queues[waiter_id][back + 1] = count;
    M->waiter_queues[waiter_id][199] += 2;
    M->waiter_queues[waiter_id][198]++;

    print_time(M->time);
    printf(" Customer %d: Order placed to Waiter %c\n", id, 'U' + waiter_id);
    fflush(stdout);
    sem_signal(semid, 0);

    sem_signal(semid, 2 + waiter_id); // Signal waiter

    sem_wait(semid, 7 + id - 1); // Wait for food
    sem_wait(semid, 0);
    print_time(M->time);
    int wait_time = M->time - arrival;
    printf(" Customer %d gets food [Waiting time = %d]\n", id, wait_time);
    fflush(stdout);
    sem_signal(semid, 0);

    int curr_time = M->time;
    usleep(3000000); // 30 min eating time
    sem_wait(semid, 0);
    if (curr_time + 30 > M->time)
        M->time = curr_time + 30;
    print_time(M->time);
    printf(" Customer %d finishes eating and leaves\n", id);
    fflush(stdout);
    M->empty_tables++;
    sem_signal(semid, 0);

    sem_signal(semid, 2 + waiter_id); // Signal waiter to check for new customers
}

int main() {
    signal(SIGINT, cleanup);

    key_t key = ftok("cook.c", 65);

    shmid = shmget(key, sizeof(struct shared_memory), 0666);
    if (shmid != -1) shmctl(shmid, IPC_RMID, NULL);
    semid = semget(key, 262, 0666);
    if (semid != -1) semctl(semid, 0, IPC_RMID);

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

    FILE *fp = fopen("customers.txt", "r");
    if (!fp) { perror("fopen failed"); exit(1); }

    int id, arrival, count, prev_arrival = 0;
    while (fscanf(fp, "%d", &id) == 1 && id != -1) {
        fscanf(fp, "%d %d", &arrival, &count);

        if (arrival > prev_arrival) {
            usleep((arrival - prev_arrival) * 100000);
            prev_arrival = arrival;
        }

        if (fork() == 0) {
            cmain(id, arrival, count);
            exit(0);
        }
    }

    while (wait(NULL) > 0);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    fclose(fp);
    return 0;
}