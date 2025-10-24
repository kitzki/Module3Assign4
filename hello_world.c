#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock;
pthread_cond_t cv;
int hello = 0;

void* print_hello(void* arg) {
    pthread_mutex_lock(&lock);
    hello += 1;
    printf("First line (hello=%d)\n", hello);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

int main() {
    pthread_t thread;

    // Initialize mutex and condition variable
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv, NULL);

    // Create the child thread
    pthread_create(&thread, NULL, print_hello, NULL);

    // Wait for the child thread to increment hello
    pthread_mutex_lock(&lock);
    while (hello < 1) {
        pthread_cond_wait(&cv, &lock);
    }
    printf("Second line (hello=%d)\n", hello);
    pthread_mutex_unlock(&lock);

    // Join and clean up
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cv);

    return 0;
}