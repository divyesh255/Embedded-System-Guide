/**
 * 01_naive_spinlock.c - Naive Spinlock (BROKEN!)
 * 
 * Demonstrates why a naive spinlock implementation has race conditions.
 * This code is intentionally broken to show the problem.
 * 
 * Compile: gcc -pthread 01_naive_spinlock.c -o 01_naive_spinlock
 * Run: ./01_naive_spinlock
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define INCREMENTS 100000

/* Naive spinlock (BROKEN!) */
typedef int spinlock_t;

void spin_lock(spinlock_t *lock) {
    /* RACE CONDITION: Two threads can both see lock == 0 */
    while (*lock == 1) {
        /* Spin */
    }
    *lock = 1;  /* Not atomic! */
}

void spin_unlock(spinlock_t *lock) {
    *lock = 0;
}

/* Shared data */
spinlock_t lock = 0;
int counter = 0;

void* increment_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < INCREMENTS; i++) {
        spin_lock(&lock);
        counter++;  /* Critical section */
        spin_unlock(&lock);
    }
    
    printf("Thread %d finished\n", thread_id);
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("=== Naive Spinlock (BROKEN!) ===\n\n");
    printf("Starting %d threads, each incrementing %d times\n", 
           NUM_THREADS, INCREMENTS);
    printf("Expected final value: %d\n\n", NUM_THREADS * INCREMENTS);
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, increment_thread, &thread_ids[i]);
    }
    
    /* Wait for threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Results ===\n");
    printf("Expected: %d\n", NUM_THREADS * INCREMENTS);
    printf("Actual:   %d\n", counter);
    
    if (counter == NUM_THREADS * INCREMENTS) {
        printf("✅ Correct (got lucky!)\n");
    } else {
        printf("❌ RACE CONDITION! Lost %d increments\n", 
               NUM_THREADS * INCREMENTS - counter);
    }
    
    printf("\n=== Why This Fails ===\n");
    printf("1. Thread A checks: lock == 0 ✓\n");
    printf("2. Thread B checks: lock == 0 ✓ (before A sets it!)\n");
    printf("3. Thread A sets: lock = 1\n");
    printf("4. Thread B sets: lock = 1\n");
    printf("5. Both threads enter critical section! 💥\n");
    
    printf("\n=== The Fix ===\n");
    printf("Use atomic operations: see 02_atomic_spinlock.c\n");
    
    return 0;
}

/*
 * WHY THIS FAILS:
 * 
 * The check-and-set is not atomic:
 * 
 * Time | Thread A          | Thread B          | lock
 * -----|-------------------|-------------------|------
 * t0   | while(*lock==1)   |                   | 0
 * t1   | (false, exit)     |                   | 0
 * t2   |                   | while(*lock==1)   | 0
 * t3   |                   | (false, exit)     | 0
 * t4   | *lock = 1         |                   | 1
 * t5   |                   | *lock = 1         | 1
 * t6   | CRITICAL SECTION  | CRITICAL SECTION  | 1  ❌
 * 
 * Both threads are in critical section!
 */
