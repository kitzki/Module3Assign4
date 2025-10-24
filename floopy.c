#include <pthread.h>
#include <stdio.h>

// Account structure
typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
} account_t;

// Struct for thread arguments
typedef struct {
    account_t* donor;
    account_t* recipient;
    float amount;
} transfer_args_t;

// Transfer function with consistent lock ordering
void* transfer(void* arg) {
    transfer_args_t* params = (transfer_args_t*) arg;

    account_t* donor = params->donor;
    account_t* recipient = params->recipient;
    float amount = params->amount;

    // Determine lock order based on uuid to prevent deadlock
    account_t* first = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t* second = (donor->uuid < recipient->uuid) ? recipient : donor;

    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    if (donor->balance < amount) {
        printf("Insufficient funds in account %ld.\n", donor->uuid);
    } else {
        donor->balance -= amount;
        recipient->balance += amount;
        printf("Transferred %.2f from account %ld to %ld\n",
               amount, donor->uuid, recipient->uuid);
    }

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);

    return NULL;
}

int main() {
    account_t acc1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1};
    account_t acc2 = {PTHREAD_MUTEX_INITIALIZER, 500, 2};

    pthread_t t1, t2;

    // Argument structs for threads
    transfer_args_t t1_args = {&acc1, &acc2, 200};
    transfer_args_t t2_args = {&acc2, &acc1, 100};

    // Launch transfers in opposite directions
    pthread_create(&t1, NULL, transfer, &t1_args);
    pthread_create(&t2, NULL, transfer, &t2_args);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Final results
    printf("Final balances:\nAccount %ld: %d\nAccount %ld: %d\n",
           acc1.uuid, acc1.balance, acc2.uuid, acc2.balance);

    return 0;
}
