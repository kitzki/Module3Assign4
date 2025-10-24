#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 4

pthread_mutex_t lock;
pthread_cond_t cv;
int count = 0;

void barrier() {
    pthread_mutex_lock(&lock);
    count++;

    if (count < NUM_THREADS) {
        // Wait until all threads reach the barrier
        pthread_cond_wait(&cv, &lock);
    } else {
        // Last thread reached barrier: reset count and wake everyone
        count = 0;
        pthread_cond_broadcast(&cv);
    }

    pthread_mutex_unlock(&lock);
}

void* worker(void* arg) {
    int id = *(int*)arg;
    printf("Thread %d: Before barrier\n", id);
    barrier();
    printf("Thread %d: After barrier\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cv);

    return 0;
}
