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
    int time; // M[0]
    int empty_tables; // M[1]
    int next_waiter; // M[2]
    int pending_orders;// M[3]
    int reserved[96]; // M[4-99] reserved
    int waiter_queues[5][200]; // M[100-1099]: [i][0-196] queue, [i][197] FR, [i][198] PO, [i][199] back
    int cook_queue[602];  // M[1100-2001]: [0-599] queue(waiter id, customer id, count), [600] front, [601] back
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

void cmain(int cook_id)
{
    char cook_name = cook_id == 0 ? 'C' : 'D';
    wait_mutex();
    print_time(M->time);
    printf("Cook %c is ready\n", cook_name);
    signal_mutex();

    while (1)
    {
        wait_cook();
        wait_mutex();

        if (M->time >= 240 && M->pending_orders == 0)
        { // 3:00pm and no orders
            print_time(M->time);
            printf("Cook %c: Leaving\n", cook_name);
            if (cook_id == 1)
            {
                for (int i = 0; i < 5; i++)
                    signal_waiter(i);
            }
            signal_mutex();
            break;
        }

        if (M->cook_queue[600] < M->cook_queue[601])// front<back--orders in queue
        { 
            int front = M->cook_queue[600];
            int waiter_id = M->cook_queue[front];
            int customer_id = M->cook_queue[front + 1];
            int count = M->cook_queue[front + 2];
            M->cook_queue[600] += 3;
            M->pending_orders--;

            print_time(M->time);
            printf("Cook %c: Preparing order (Waiter %c, Customer %d, Count %d)\n",
                   cook_name, 'U' + waiter_id, customer_id, count);
            signal_mutex();

            int curr_time = M->time;
            usleep(count * 500000);

            wait_mutex();
            if (curr_time + count * 5 > M->time)
                M->time = curr_time + count * 5;

            print_time(M->time);
            printf("Cook %c: Prepared order (Waiter %c, Customer %d, Count %d)\n",
                   cook_name, 'U' + waiter_id, customer_id, count);
            M->waiter_queues[waiter_id][197] = customer_id; // Food Ready (FR)
            signal_mutex();                            

            signal_waiter(waiter_id); 
        }
        else
        {
            signal_mutex(); 
        }
    }
}

int main()
{
    key_t key = ftok("cook.c", 65);
    shmid = shmget(key, sizeof(struct shared_memory), 0666 | IPC_CREAT);
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

    semid = semget(key, 207, 0666 | IPC_CREAT); // 1 mutex + 1 cook + 5 waiters + 200 customers
    if (semid == -1)
    {
        perror("semget failed");
        exit(1);
    }

    M->time = 0;
    M->empty_tables = 10;
    M->next_waiter = 0;
    M->pending_orders = 0;
    M->cook_queue[600] = 0; // front
    M->cook_queue[601] = 0; // back
    for (int i = 0; i < 5; i++)
    {
        M->waiter_queues[i][197] = 0; // Fod ready
        M->waiter_queues[i][198] = 0; // Preparing order
        M->waiter_queues[i][199] = 0; // back
    }

    semctl(semid, 0, SETVAL, 1); // mutex(0)
    semctl(semid, 1, SETVAL, 0); // cook(1)
    for (int i = 0; i < 5; i++)
        semctl(semid, 2 + i, SETVAL, 0); // waiters(2-6)
    for (int i = 0; i < 200; i++)
        semctl(semid, 7 + i, SETVAL, 0); // customers(7-206)

    for (int i = 0; i < 2; i++)
    {
        if (fork() == 0)
        {
            cmain(i);
            exit(0);
        }
    }
    wait(NULL);
    wait(NULL);
    return 0;
}