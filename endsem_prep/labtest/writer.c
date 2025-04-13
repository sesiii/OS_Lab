// // #include<stdio.h>
// // #include<stdlib.h>
// // #include<sys/sem.h>


// // #define SHM_KEY 0x1234
// // #define SEM_KEY 0x5678

// // int main()
// // {
// //     int shm_id=shmget(SHM_KEY,sizeof(int),IPC_CREAT| 0666);
// //     int sem_id=semget(SEM_KEY,1,IPC_CREAT |06660);
    
    
// //     int *data=(int *)shmat(shm_id,NULL,0);
// //     semctl(sem_id,0,SETVAL,0);

// //     printf("enter a number: \n");
// //     scanf("%d",data);

// //     struct sembuf sem_op={0,1,0};
// //     semop(sem_id,&sem_op,1);
// //     printf("writer: data written\n");
// //     shmdt(data);
// //     return 0;

// // }


// #include<stdio.h>
// #include<stdlib.h>
// #define SHM_KEY 1024
// #define SEM_KEY 1023
// #include<string.h>
// #include<sys/sem.h>

// int main()
// {
//     int shm_id=shmget(SHM_KEY,sizeof(int),IPC_CREAT | 0666);
//     int sem_id=semget(SEM_KEY,1,IPC_CREAT | 0666);

//     int *data=(int *)shmat(shm_id,NULL,0);
    // semctl(sem_id,0,SETVAL,0);
    // printf("Writer: Enter a number: ");
    // scanf("%d", data); // directly writing into shared memory

    // struct sembuf sem_op = {0, 1, 0}; // V operation (signal)
    // semop(sem_id, &sem_op, 1);

    // printf("Writer: Data written. Exiting.\n");
    // shmdt(data);
    // return 0;
// } 


#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/sem.h>
#include<sys/shm.h>

#define SHM_KEY 1024
#define SEM_KEY 1024

int main()
{
    int shm_id=shmget(SHM_KEY,sizeof(int),IPC_CREAT |0666);
    int sem_id=semget(SEM_KEY,1,IPC_CREAT | 0666);

    int *data=(int *)shmat(shm_id,NULL,0);

    semctl(sem_id,0,SETVAL,0);
    // printf(sem_id,0,16,0);
    // semctl(sem_id,0,SETVAL,0);
    printf("Writer: Enter a number: ");
    scanf("%d", data); // directly writing into shared memory

    struct sembuf sem_ops[3] = {
        {0, 1, 0},  // 1st V operation (increment)
        {0, -1, 0},  // 2nd V operation
        {0, 1, 0}   // 3rd V operation
    };
    
    semop(sem_id, sem_ops, 3);  // Perform all 3 at once
    
    int sem_value = semctl(sem_id, 0, GETVAL);
    printf("Current semaphore value: %d\n", sem_value);
    // printf("Current semaphore value: %d\n", sem_value);
    printf("Writer: Data written. Exiting.\n");
    shmdt(data);
    return 0;

}