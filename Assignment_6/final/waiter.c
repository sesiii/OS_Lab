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

void wmain(int id)
{
    char waiter_name = 'U' + id;
    wait_mutex(); 
    print_time(M->time);
    printf("Waiter %c is ready\n", waiter_name);
    signal_mutex(); 

    while (1)
    {
        wait_waiter(id); 
        wait_mutex();    

        if (M->time >= 240 && M->waiter_queues[id][198] == 0 && M->waiter_queues[id][197] == 0)
        {
            print_time(M->time);
            printf("Waiter %c leaving (no more customers to serve)\n", waiter_name);

            signal_mutex(); 
            break;
        }

        if (M->waiter_queues[id][197] > 0)// Food Ready 
        { 
            int customer_id = M->waiter_queues[id][197];
            M->waiter_queues[id][197] = 0;
            print_time(M->time);
            printf("Waiter %c: Serving food to Customer %d\n", waiter_name, customer_id);
            signal_mutex();           
            signal_customer(customer_id); 
        }
        else if (M->waiter_queues[id][198] > 0)// Pending Order
        { 
            int back = M->waiter_queues[id][199];
            int front = back - (M->waiter_queues[id][198] * 2); 
            if (front < 0)
                front = 0;

            int customer_id = M->waiter_queues[id][front];
            int count = M->waiter_queues[id][front + 1];
            M->waiter_queues[id][198]--; 

            print_time(M->time);

            printf("Waiter %c: Placing order for Customer %d (count = %d)\n",
                   waiter_name, customer_id, count);
            signal_mutex(); 

            int curr_time = M->time;
            usleep(100000); 

            wait_mutex(); 
            if (curr_time + 1 > M->time)
                M->time = curr_time + 1;

            int cook_back = M->cook_queue[601];
            M->cook_queue[cook_back] = id;
            M->cook_queue[cook_back + 1] = customer_id;
            M->cook_queue[cook_back + 2] = count;
            M->cook_queue[601] += 3;
            M->pending_orders++;
            signal_mutex(); 

            signal_cook(); 
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

    for (int i = 0; i < 5; i++)
    {
        if (fork() == 0)
        {
            wmain(i); // Waiter i
            exit(0);
        }
    }
    for (int i = 0; i < 5; i++)
        wait(NULL);
    return 0;
}