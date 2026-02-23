/**
 * 03_spinlock.c - Lock-Free Spinlock
 * 
 * Demonstrates spinlock using atomic_flag.
 * 
 * Compile: gcc -std=c11 -pthread -o 03_spinlock 03_spinlock.c
 * Run: ./03_spinlock
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_flag spinlock = ATOMIC_FLAG_INIT;
int shared_counter = 0;

void spin_lock(atomic_flag *lock) {
    while (atomic_flag_test_and_set(lock)) {
        /* Spin (busy-wait) */
    }
}

void spin_unlock(atomic_flag *lock) {
    atomic_flag_clear(lock);
}

void *worker(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < 100000; i++) {
        spin_lock(&spinlock);
        shared_counter++;
        spin_unlock(&spinlock);
    }
    
    printf("[Thread %d] Done\n", id);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    int id1 = 1, id2 = 2;
    
    printf("=== Spinlock Demo ===\n\n");
    
    pthread_create(&t1, NULL, worker, &id1);
    pthread_create(&t2, NULL, worker, &id2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\nCounter: %d (expected 200000)\n", shared_counter);
    
    return 0;
}

/* Spinlock: Fast but wastes CPU while waiting */
