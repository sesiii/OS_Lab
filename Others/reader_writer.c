#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t wrt;   // Semaphore for Writer access
sem_t mutex;      // Semaphore for Reader count update
int read_count = 0;
int shared_data = 0; // Shared resource

void* reader(void* arg) {
    int reader_id = *(int*)arg;
    while (1) {
        wait(&mutex); // Lock to update read_count
        read_count++;
        if (read_count == 1) 
            wait(&wrt); // First reader blocks writers
        signal(&mutex); // Release mutex

        // Reading section
        printf("Reader %d is reading: %d\n", reader_id, shared_data);
        usleep(100000); // Simulate reading time

        wait(&mutex);
        read_count--;
        if (read_count == 0) 
            signal(&wrt); // Last reader unblocks writers
        signal(&mutex);

        usleep(200000); // Simulate time between readings
    }
}

void* writer(void* arg) {
    int writer_id = *(int*)arg;
    while (1) {
        wait(&wrt); // Lock for exclusive writing

        // Writing section
        shared_data++;
        printf("Writer %d is writing: %d\n", writer_id, shared_data);
        usleep(150000); // Simulate writing time

        signal(&wrt); // Release lock

        usleep(300000); // Simulate time between writings
    }
}

int main() {
    pthread_t readers[3], writers[2];
    int reader_ids[3] = {1, 2, 3}, writer_ids[2] = {1, 2};

    sem_init(&wrt, 0, 1);
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < 3; i++)
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    
    for (int i = 0; i < 2; i++)
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);

    for (int i = 0; i < 3; i++)
        pthread_join(readers[i], NULL);
    
    for (int i = 0; i < 2; i++)
        pthread_join(writers[i], NULL);

    sem_destroy(&wrt);
    sem_destroy(&mutex);
    return 0;
}

