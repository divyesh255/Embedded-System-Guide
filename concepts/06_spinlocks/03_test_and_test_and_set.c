/**
 * 03_test_and_test_and_set.c - Optimized Spinlock
 * 
 * Demonstrates test-and-test-and-set spinlock that reduces cache traffic.
 * 
 * Compile: gcc -pthread 03_test_and_test_and_set.c -o 03_test_and_test_and_set
 * Run: ./03_test_and_test_and_set
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 8
#define INCREMENTS 500000

typedef atomic_int spinlock_t;

/* Basic test-and-set spinlock */
void spin_lock_tas(spinlock_t *lock) {
    while (atomic_exchange(lock, 1) == 1) {
        /* Spin - keeps writing to lock */
    }
}

/* Optimized test-and-test-and-set spinlock */
void spin_lock_ttas(spinlock_t *lock) {
    while (1) {
        /* First test: read-only, no cache invalidation */
        while (atomic_load(lock) == 1) {
            /* Spin without writing */
        }
        
        /* Then test-and-set: try to acquire */
        if (atomic_exchange(lock, 1) == 0) {
            break;  /* Got the lock! */
        }
    }
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* Shared data */
spinlock_t lock_tas = 0;
spinlock_t lock_ttas = 0;
int counter = 0;

void* increment_tas(void* arg) {
    for (int i = 0; i < INCREMENTS; i++) {
        spin_lock_tas(&lock_tas);
        counter++;
        spin_unlock(&lock_tas);
    }
    return NULL;
}

void* increment_ttas(void* arg) {
    for (int i = 0; i < INCREMENTS; i++) {
        spin_lock_ttas(&lock_ttas);
        counter++;
        spin_unlock(&lock_ttas);
    }
    return NULL;
}

double benchmark(void* (*thread_func)(void*), const char *name) {
    pthread_t threads[NUM_THREADS];
    counter = 0;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                    (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("%s: %.3f seconds (counter=%d)\n", name, elapsed, counter);
    return elapsed;
}

int main(void) {
    printf("=== Test-and-Test-and-Set Optimization ===\n\n");
    printf("Running %d threads, %d increments each\n\n", 
           NUM_THREADS, INCREMENTS);
    
    double time_tas = benchmark(increment_tas, "Test-and-Set (TAS)");
    double time_ttas = benchmark(increment_ttas, "Test-and-Test-and-Set (TTAS)");
    
    printf("\n=== Performance Comparison ===\n");
    printf("TAS:  %.3f seconds\n", time_tas);
    printf("TTAS: %.3f seconds\n", time_ttas);
    printf("Speedup: %.2fx faster\n", time_tas / time_ttas);
    
    printf("\n=== Why TTAS is Faster ===\n");
    printf("TAS:  Every spin writes to lock → cache invalidation\n");
    printf("TTAS: Spins with reads only → no cache traffic\n");
    printf("      Only writes when lock appears free\n");
    
    printf("\n=== Cache Coherence Impact ===\n");
    printf("On multicore systems:\n");
    printf("- TAS: High cache coherence traffic (writes)\n");
    printf("- TTAS: Low cache coherence traffic (reads)\n");
    printf("- TTAS reduces bus contention significantly\n");
    
    return 0;
}

/*
 * WHY TEST-AND-TEST-AND-SET IS BETTER:
 * 
 * Test-and-Set (TAS):
 * while (atomic_exchange(lock, 1) == 1) { }
 * 
 * Every iteration:
 * 1. Writes 1 to lock
 * 2. Invalidates cache line on all cores
 * 3. High bus traffic
 * 
 * Test-and-Test-and-Set (TTAS):
 * while (1) {
 *     while (atomic_load(lock) == 1) { }  // Read-only
 *     if (atomic_exchange(lock, 1) == 0) break;
 * }
 * 
 * Most iterations:
 * 1. Only reads lock (cached locally)
 * 2. No cache invalidation
 * 3. Low bus traffic
 * 
 * Only writes when lock appears free!
 */
