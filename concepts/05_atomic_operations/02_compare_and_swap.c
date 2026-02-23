/**
 * 02_compare_and_swap.c - Compare-And-Swap (CAS)
 * 
 * Demonstrates CAS operation for lock-free algorithms.
 * 
 * Compile: gcc -std=c11 -pthread -o 02_compare_and_swap 02_compare_and_swap.c
 * Run: ./02_compare_and_swap
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

atomic_int value = ATOMIC_VAR_INIT(0);

void *cas_worker(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < 5; i++) {
        int expected = atomic_load(&value);
        int desired = expected + 1;
        
        /* Try to swap: if value == expected, set to desired */
        if (atomic_compare_exchange_strong(&value, &expected, desired)) {
            printf("[Thread %d] CAS success: %d → %d\n", id, expected, desired);
        } else {
            printf("[Thread %d] CAS failed: expected %d, was %d\n", 
                   id, expected - 1, expected);
        }
        
        sleep(0.1);
    }
    
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;
    
    printf("=== Compare-And-Swap Demo ===\n\n");
    
    pthread_create(&t1, NULL, cas_worker, &id1);
    pthread_create(&t2, NULL, cas_worker, &id2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\nFinal value: %d\n", atomic_load(&value));
    
    return 0;
}

/* CAS is the foundation of lock-free algorithms */
