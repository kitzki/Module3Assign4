#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t lock;
pthread_cond_t reader_cv, writer_cv;
int reader_count = 0;       // Active readers
int writer_waiting = 0;     // Writers waiting
int shared_data = 0;        // Shared resource

void* reader(void* arg) {
    pthread_mutex_lock(&lock);

    // Wait if there is a writer waiting
    while (writer_waiting > 0) {
        pthread_cond_wait(&reader_cv, &lock);
    }

    reader_count++;
    pthread_mutex_unlock(&lock);

    // Read the shared data
    printf("Reader reads: %d\n", shared_data);
    sleep(1); // Simulate read time

    pthread_mutex_lock(&lock);
    reader_count--;
    if (reader_count == 0) {
        // Last reader signals writers
        pthread_cond_signal(&writer_cv);
    }
    pthread_mutex_unlock(&lock);

    return NULL;
}

void* writer(void* arg) {
    pthread_mutex_lock(&lock);
    writer_waiting++;

    // Wait until no readers are active
    while (reader_count > 0) {
        pthread_cond_wait(&writer_cv, &lock);
    }

    // Writer now has exclusive access
    writer_waiting--;
    shared_data++;
    printf("Writer writes: %d\n", shared_data);
    sleep(1); // Simulate write time

    // Give priority back to waiting writers
    if (writer_waiting > 0) {
        pthread_cond_signal(&writer_cv);
    } else {
        // If no writers waiting, allow readers
        pthread_cond_broadcast(&reader_cv);
    }

    pthread_mutex_unlock(&lock);
    return NULL;
}

int main() {
    pthread_t readers[3], writers[2];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&reader_cv, NULL);
    pthread_cond_init(&writer_cv, NULL);

    // Create reader threads
    for (int i = 0; i < 3; i++)
        pthread_create(&readers[i], NULL, reader, NULL);

    // Create writer threads
    for (int i = 0; i < 2; i++)
        pthread_create(&writers[i], NULL, writer, NULL);

    // Wait for threads to finish
    for (int i = 0; i < 3; i++)
        pthread_join(readers[i], NULL);

    for (int i = 0; i < 2; i++)
        pthread_join(writers[i], NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&reader_cv);
    pthread_cond_destroy(&writer_cv);

    return 0;
}
