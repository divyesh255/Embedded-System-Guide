/**
 * 01_atomic_counter.c - Lock-Free Atomic Counter
 * 
 * Demonstrates atomic operations for thread-safe counter without locks.
 * 
 * Compile: gcc -std=c11 -pthread -o 01_atomic_counter 01_atomic_counter.c
 * Run: ./01_atomic_counter
 * 
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#define NUM_THREADS 4
#define INCREMENTS 250000

atomic_int counter = ATOMIC_VAR_INIT(0);

void *increment_atomic(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < INCREMENTS; i++) {
        atomic_fetch_add(&counter, 1);  /* Lock-free! */
    }
    
    printf("[Thread %d] Finished\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS] = {1, 2, 3, 4};
    int expected = NUM_THREADS * INCREMENTS;
    
    printf("=== Atomic Counter Demo ===\n\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment_atomic, &ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int final = atomic_load(&counter);
    
    printf("\nFinal counter: %d\n", final);
    printf("Expected: %d\n", expected);
    
    if (final == expected) {
        printf("✓ Correct! Atomic operations are thread-safe.\n");
    } else {
        printf("✗ Wrong! Lost updates: %d\n", expected - final);
    }
    
    return 0;
}

/*
 * Atomic operations are:
 * - Much faster than mutex
 * - Lock-free (no deadlock)
 * - Thread-safe
 * 
 * NEXT: 02_compare_and_swap.c
 */
