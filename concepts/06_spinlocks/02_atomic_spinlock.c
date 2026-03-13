/**
 * 02_atomic_spinlock.c - Correct Spinlock with Atomics
 * 
 * Demonstrates a correct spinlock implementation using atomic operations.
 * 
 * Compile: gcc -pthread 02_atomic_spinlock.c -o 02_atomic_spinlock
 * Run: ./02_atomic_spinlock
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#define NUM_THREADS 4
#define INCREMENTS 1000000

/* Correct spinlock using atomics */
typedef atomic_int spinlock_t;

void spin_lock(spinlock_t *lock) {
    int expected = 0;
    /* Keep trying to swap 0 → 1 atomically */
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;  /* Reset for next iteration */
        /* Spin! */
    }
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
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
    
    printf("=== Atomic Spinlock (CORRECT!) ===\n\n");
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
        printf("✅ CORRECT! No race conditions\n");
    } else {
        printf("❌ ERROR! Lost %d increments\n", 
               NUM_THREADS * INCREMENTS - counter);
    }
    
    printf("\n=== How It Works ===\n");
    printf("1. atomic_compare_exchange_weak(lock, &expected, 1):\n");
    printf("   - If *lock == expected (0), set *lock = 1, return true\n");
    printf("   - If *lock != expected, set expected = *lock, return false\n");
    printf("2. Only ONE thread can successfully swap 0 → 1\n");
    printf("3. Other threads keep spinning until lock is released\n");
    
    printf("\n=== Performance Note ===\n");
    printf("Spinlocks waste CPU cycles while spinning.\n");
    printf("Use only for very short critical sections!\n");
    
    return 0;
}

/*
 * HOW ATOMIC COMPARE-EXCHANGE WORKS:
 * 
 * Time | Thread A                    | Thread B                    | lock
 * -----|-----------------------------|-----------------------------|------
 * t0   | expected = 0                |                             | 0
 * t1   | CAS(lock, &exp, 1)          |                             | 0
 * t2   | *lock==0? YES → *lock=1     |                             | 1
 * t3   | return true, got lock!      |                             | 1
 * t4   |                             | expected = 0                | 1
 * t5   |                             | CAS(lock, &exp, 1)          | 1
 * t6   |                             | *lock==0? NO → exp=1        | 1
 * t7   |                             | return false, spin!         | 1
 * t8   | CRITICAL SECTION            | while(!CAS(...)) { spin }   | 1
 * t9   | atomic_store(lock, 0)       |                             | 0
 * t10  |                             | CAS succeeds, got lock!     | 1
 * 
 * Only ONE thread can be in critical section at a time! ✅
 */
