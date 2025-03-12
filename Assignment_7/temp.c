#include <stdio.h>
#include <pthread.h>

int counter = 0;

void* increment(void* arg) {
    counter++;
    printf("Counter: %d\n", counter);
    return NULL;
}

void* decrement(void* arg) {
    counter--;
    printf("Counter: %d\n", counter);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, decrement, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}