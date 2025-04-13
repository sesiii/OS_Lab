#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock;
int arr[10] = {1,2,3,4,5,6,7,8,9,10};
int j = 0;

void* split_and_add(void* arg) {
    pthread_mutex_lock(&lock);
    
    int sum = 0;
    printf("My array: ");
    for(int k = 0; k < 3 && j < 10; k++, j++) {
        printf("%d ", arr[j]);
        sum += arr[j];
    }
    printf("\nSum value of the array: %d\n", sum);
    
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t threads[4];
    pthread_mutex_init(&lock, NULL);

    for(int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, split_and_add, NULL);
    }

    for(int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}