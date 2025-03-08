#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

struct shared_memory
{
    int time;
    int empty_tables;
    int next_waiter;
    int pending_orders;
    int reserved[96];
    int waiter_queues[5][200];
    int cook_queue[602];
};

struct shared_memory *M;
int shmid, semid;


void wait_mutex()
{
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}
void signal_mutex()
{
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}
void wait_cook()
{
    struct sembuf sb = {1, -1, 0};
    semop(semid, &sb, 1);
}
void signal_cook()
{
    struct sembuf sb = {1, 1, 0};
    semop(semid, &sb, 1);
}
void wait_waiter(int i)
{
    struct sembuf sb = {2 + i, -1, 0};
    semop(semid, &sb, 1);
}
void signal_waiter(int i)
{
    struct sembuf sb = {2 + i, 1, 0};
    semop(semid, &sb, 1);
}
void wait_customer(int i)
{
    struct sembuf sb = {7 + i - 1, -1, 0};
    semop(semid, &sb, 1);
}

void signal_customer(int i)
{
    struct sembuf sb = {7 + i - 1, 1, 0};
    semop(semid, &sb, 1);
}

void print_time(int minutes)
{
    int hour = 11 + (minutes / 60);
    int min = minutes % 60;
    if (hour > 12)
        hour -= 12;
    printf("[%02d:%02d] ", hour, min);
}

void cmain(int id, int arrival, int count)
{
    wait_mutex(); 
    if (arrival >= 240) // 3:00pm
    { 
        print_time(arrival);
        printf("Customer %d leaves (late arrival)\n", id);
        signal_mutex(); 
        return;
    }

    if (M->time < arrival)
        M->time = arrival;
    print_time(M->time);
    printf("Customer %d arrives (count = %d)\n", id, count);

    if (M->empty_tables == 0)
    {
        print_time(M->time);
        printf("Customer %d leaves (no empty table)\n", id);
        signal_mutex(); 
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
    printf("Customer %d: Order placed to Waiter %c\n", id, 'U' + waiter_id);
    signal_mutex(); 

    signal_waiter(waiter_id); 

    wait_customer(id); 
    wait_mutex();      
    print_time(M->time);
    int wait_time = M->time - arrival;
    printf("Customer %d gets food [Waiting time = %d]\n", id, wait_time);
    signal_mutex(); 

    int curr_time = M->time;
    usleep(3000000); 

    wait_mutex();
    if (curr_time + 30 > M->time)
        M->time = curr_time + 30;
    print_time(M->time);
    printf("Customer %d finishes eating and leaves\n", id);
    M->empty_tables++;
    signal_mutex(); 
}

int main()
{
    key_t key = ftok("cook.c", 65);
    shmid = shmget(key, sizeof(struct shared_memory), 0666);
    if (shmid == -1)
    {
        perror("shmget failed");
        exit(1);
    }
    M = shmat(shmid, NULL, 0);
    if (M == (void *)-1)
    {
        perror("shmat failed");
        exit(1);
    }

    semid = semget(key, 207, 0666);
    if (semid == -1)
    {
        perror("semget failed");
        exit(1);
    }

    FILE *fp = fopen("customers.txt", "r");
    if (!fp)
    {
        perror("fopen failed");
        exit(1);
    }

    int id, arrival, count, prev_arrival = 0;
    while (fscanf(fp, "%d", &id) == 1 && id != -1)
    {
        fscanf(fp, "%d %d", &arrival, &count);
        if (arrival > prev_arrival)
        {
            usleep((arrival - prev_arrival) * 100000);
            prev_arrival = arrival;
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            cmain(id, arrival, count);
            exit(0);
        }
    }
    fclose(fp);

    while (wait(NULL) > 0)
        ;
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    return 0;
}