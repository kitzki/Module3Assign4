#include <pthread.h>
#include <sched.h>
#include <stdio.h>

// Account structure with priority field
typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
    int priority;
} account_t;

// Transfer function with priority donation
void transfer(account_t *donor, account_t *recipient, float amount, int thread_priority) {
    // Lock accounts in order based on UUID to avoid deadlock
    account_t *first = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t *second = (donor->uuid < recipient->uuid) ? recipient : donor;

    // Acquire first lock
    pthread_mutex_lock(&first->lock);
    // Priority donation: if current thread has higher priority, donate it
    if (first->priority < thread_priority)
        first->priority = thread_priority;

    // Acquire second lock
    pthread_mutex_lock(&second->lock);
    if (second->priority < thread_priority)
        second->priority = thread_priority;

    // Perform transfer
    if (donor->balance < amount) {
        printf("Insufficient funds in account %ld.\n", donor->uuid);
    } else {
        donor->balance -= amount;
        recipient->balance += amount;
        printf("Transferred %.2f from account %ld to %ld (thread priority %d)\n",
               amount, donor->uuid, recipient->uuid, thread_priority);
    }

    // Reset priorities after unlock
    second->priority = 0;
    first->priority = 0;

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

// Thread wrapper for transfer
void* transfer_thread(void* arg) {
    struct thread_args {
        account_t* donor;
        account_t* recipient;
        float amount;
        int priority;
    };
    struct thread_args* params = (struct thread_args*)arg;

    transfer(params->donor, params->recipient, params->amount, params->priority);
    return NULL;
}

int main() {
    // Initialize accounts
    account_t acc1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1, 0};
    account_t acc2 = {PTHREAD_MUTEX_INITIALIZER, 500, 2, 0};

    // Create threads
    pthread_t t1, t2;
    struct {
        account_t* donor;
        account_t* recipient;
        float amount;
        int priority;
    } t1_args = {&acc1, &acc2, 200, 2}, // higher priority
      t2_args = {&acc2, &acc1, 100, 1}; // lower priority

    pthread_create(&t1, NULL, transfer_thread, &t1_args);
    pthread_create(&t2, NULL, transfer_thread, &t2_args);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final balances: acc1 = %d, acc2 = %d\n", acc1.balance, acc2.balance);
    return 0;
}
