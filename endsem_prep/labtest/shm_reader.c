// reader.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

int main() {
    int shm_id = shmget(SHM_KEY, sizeof(int), 0666);
    int *data = (int *)shmat(shm_id, NULL, 0);

    int sem_id = semget(SEM_KEY, 1, 0666);

    struct sembuf sem_op = {0, -1, 0}; // P operation (wait)
    semop(sem_id, &sem_op, 1);

    printf("Reader: Read value = %d\n", *data);

    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);  // clean up
    semctl(sem_id, 0, IPC_RMID);     // clean up
    return 0;
}