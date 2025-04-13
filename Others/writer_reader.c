#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t rw_mutex;      // Controls access to the shared resource
sem_t mutex;         // Protects updates to read_count
sem_t write_wait;    // Blocks new readers when a writer is waiting
int read_count = 0;  // Number of active readers
int write_count = 0; // Number of waiting writers
int shared_data = 0; // Shared resource

void* reader(void* arg) {
    int reader_id = *(int*)arg;
    while (1) {
        sem_wait(&write_wait); // Wait if a writer is waiting
        sem_wait(&mutex);
        
        read_count++;
        if (read_count == 1) 
            sem_wait(&rw_mutex); // First reader locks writers
        
        sem_post(&mutex);
        sem_post(&write_wait); // Allow other readers if no writer is waiting

        // Reading section
        printf("Reader %d is reading: %d\n", reader_id, shared_data);
        usleep(100000); // Simulate reading time

        sem_wait(&mutex);
        read_count--;
        if (read_count == 0) 
            sem_post(&rw_mutex); // Last reader unlocks writer access
        sem_post(&mutex);

        usleep(200000); // Simulate time between readings
    }
}

void* writer(void* arg) {
    int writer_id = *(int*)arg;
    while (1) {
        sem_wait(&mutex);
        write_count++; // Indicate a waiting writer
        if (write_count == 1) 
            sem_wait(&write_wait); // First waiting writer blocks new readers
        sem_post(&mutex);

        sem_wait(&rw_mutex); // Lock for exclusive writing

        // Writing section
        shared_data++;
        printf("Writer %d is writing: %d\n", writer_id, shared_data);
        usleep(150000); // Simulate writing time

        sem_post(&rw_mutex); // Release lock

        sem_wait(&mutex);
        write_count--; // Writer finished
        if (write_count == 0) 
            sem_post(&write_wait); // If no waiting writers, allow readers
        sem_post(&mutex);

        usleep(300000); // Simulate time between writings
    }
}

int main() {
    pthread_t readers[3], writers[2];
    int reader_ids[3] = {1, 2, 3}, writer_ids[2] = {1, 2};

    sem_init(&rw_mutex, 0, 1);
    sem_init(&mutex, 0, 1);
    sem_init(&write_wait, 0, 1);

    for (int i = 0; i < 3; i++)
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    
    for (int i = 0; i < 2; i++)
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);

    for (int i = 0; i < 3; i++)
        pthread_join(readers[i], NULL);
    
    for (int i = 0; i < 2; i++)
        pthread_join(writers[i], NULL);

    sem_destroy(&rw_mutex);
    sem_destroy(&mutex);
    sem_destroy(&write_wait);
    return 0;
}