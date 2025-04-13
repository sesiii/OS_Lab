// writer.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    int *data = (int *)shmat(shm_id, NULL, 0);

    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, 0); // Initialize semaphore to 0

    printf("Writer: Enter a number: ");
    scanf("%d", data); // directly writing into shared memory

    struct sembuf sem_op = {0, 1, 0}; // V operation (signal)
    semop(sem_id, &sem_op, 1);

    printf("Writer: Data written. Exiting.\n");

    shmdt(data);
    return 0;
}
