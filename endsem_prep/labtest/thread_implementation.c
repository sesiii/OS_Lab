// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>

// pthread_mutex_t lock;
// int arr[10] = {1,2,3,4,5,6,7,8,9,10};
// int j = 0;

// void* split_and_add(void* arg) {
//     pthread_mutex_lock(&lock);
    
//     int sum = 0;
//     printf("My array: ");
//     for(int k = 0; k < 3 && j < 10; k++, j++) {
//         printf("%d ", arr[j]);
//         sum += arr[j];
//     }
//     printf("\nSum value of the array: %d\n", sum);
    
//     pthread_mutex_unlock(&lock);
//     return NULL;
// }

// int main() {
//     pthread_t threads[4];
//     pthread_mutex_init(&lock, NULL);

//     for(int i = 0; i < 4; i++) {
//         pthread_create(&threads[i], NULL, split_and_add, NULL);
//     }

//     for(int i = 0; i < 4; i++) {
//         pthread_join(threads[i], NULL);
//     }

//     pthread_mutex_destroy(&lock);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int count = 0;  // Shared variable

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* threadA_func(void* arg) {
    printf("Thread A: Waiting for count to reach 5...\n");

    pthread_mutex_lock(&lock);
    while (count < 5) {
        pthread_cond_wait(&cond, &lock); // releases lock, waits to be signaled
    }
    printf("Thread A: Woken up! count = %d\n", count);
    pthread_mutex_unlock(&lock);

    printf("Thread A: Continuing execution...\n");
    return NULL;
}

void* threadB_func(void* arg) {
    for (int i = 0; i < 5; ++i) {
        sleep(1); // Simulate work
        pthread_mutex_lock(&lock);
        count++;
        printf("Thread B: Incremented count to %d\n", count);
        if (count >= 5) {
            pthread_cond_signal(&cond); // Wake up Thread A
        }
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main() {
    pthread_t threadA, threadB;

    pthread_create(&threadA, NULL, threadA_func, NULL);
    pthread_create(&threadB, NULL, threadB_func, NULL);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}