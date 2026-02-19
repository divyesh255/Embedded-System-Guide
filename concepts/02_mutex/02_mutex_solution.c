/**
 * 02_mutex_solution.c - Fixing Race Condition with Mutex
 * 
 * This program shows how to use a mutex to protect shared data.
 * Compare with 01_race_condition.c - this version ALWAYS works correctly!
 * 
 * Compile: gcc -pthread -o 02_mutex_solution 02_mutex_solution.c
 * Run: ./02_mutex_solution
 * 
 * Study time: 15 minutes
 * Difficulty: Beginner
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2
#define INCREMENTS 1000000

/* Shared variable - now protected! */
int counter = 0;

/* Mutex to protect counter */
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * increment_counter - Safely increments shared counter
 * 
 * Uses mutex to ensure only one thread modifies counter at a time.
 * The critical section (counter++) is protected by lock/unlock.
 */
void *increment_counter(void *arg)
{
    (void)arg;
    int i;
    
    printf("[Thread %lu] Starting to increment...\n", pthread_self());
    
    for (i = 0; i < INCREMENTS; i++) {
        /* Lock mutex before accessing shared data */
        if (pthread_mutex_lock(&counter_mutex) != 0) {
            fprintf(stderr, "Error: mutex lock failed\n");
            return NULL;
        }
        
        /* Critical section - only one thread at a time */
        counter++;
        
        /* Unlock mutex after modifying shared data */
        if (pthread_mutex_unlock(&counter_mutex) != 0) {
            fprintf(stderr, "Error: mutex unlock failed\n");
            return NULL;
        }
    }
    
    printf("[Thread %lu] Finished incrementing\n", pthread_self());
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    int i;
    int expected = NUM_THREADS * INCREMENTS;
    
    printf("=== Mutex Solution Demonstration ===\n\n");
    printf("Starting counter: %d\n", counter);
    printf("Expected final value: %d\n", expected);
    printf("(%d threads × %d increments each)\n\n", NUM_THREADS, INCREMENTS);
    
    /* Create threads */
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, increment_counter, NULL) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            return 1;
        }
    }
    
    /* Wait for all threads */
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
            return 1;
        }
    }
    
    /* Check result */
    printf("\n=== Results ===\n");
    printf("Final counter value: %d\n", counter);
    printf("Expected value: %d\n", expected);
    
    if (counter == expected) {
        printf("\n✓ CORRECT! Mutex protected the shared data!\n");
        printf("\nHow mutex helped:\n");
        printf("- Only one thread could increment at a time\n");
        printf("- No lost updates\n");
        printf("- Result is always correct\n");
    } else {
        printf("\n✗ WRONG! This should not happen with mutex!\n");
        printf("Lost updates: %d\n", expected - counter);
    }
    
    /* Clean up mutex */
    pthread_mutex_destroy(&counter_mutex);
    
    printf("\nRun this multiple times - result is ALWAYS correct!\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT (same every time!):
 * ------------------------------------
 * === Mutex Solution Demonstration ===
 * 
 * Starting counter: 0
 * Expected final value: 2000000
 * (2 threads × 1000000 increments each)
 * 
 * [Thread 140234567890] Starting to increment...
 * [Thread 140234567891] Starting to increment...
 * [Thread 140234567890] Finished incrementing
 * [Thread 140234567891] Finished incrementing
 * 
 * === Results ===
 * Final counter value: 2000000  ← CORRECT!
 * Expected value: 2000000
 * 
 * ✓ CORRECT! Mutex protected the shared data!
 * 
 * How mutex helped:
 * - Only one thread could increment at a time
 * - No lost updates
 * - Result is always correct
 * 
 * Run this multiple times - result is ALWAYS correct!
 * 
 * 
 * HOW MUTEX WORKS:
 * ----------------
 * 
 * Thread 1                          Thread 2
 * --------                          --------
 * pthread_mutex_lock(&mutex)
 * counter++ (safe!)
 *                                   pthread_mutex_lock(&mutex)
 *                                   ← BLOCKED! Waits for Thread 1
 * pthread_mutex_unlock(&mutex)
 *                                   ← Now gets lock!
 *                                   counter++ (safe!)
 *                                   pthread_mutex_unlock(&mutex)
 * 
 * Only one thread can hold the lock at a time!
 * 
 * 
 * KEY CONCEPTS:
 * -------------
 * 1. Mutex = Mutual Exclusion Lock
 * 2. pthread_mutex_lock() - Acquire lock (blocks if busy)
 * 3. pthread_mutex_unlock() - Release lock
 * 4. Critical Section - Code between lock/unlock
 * 5. Atomic Execution - Critical section runs without interruption
 * 
 * 
 * MUTEX INITIALIZATION:
 * ---------------------
 * Two ways to initialize:
 * 
 * 1. Static (simple):
 *    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
 * 
 * 2. Dynamic (more control):
 *    pthread_mutex_t lock;
 *    pthread_mutex_init(&lock, NULL);
 *    // ... use lock ...
 *    pthread_mutex_destroy(&lock);
 * 
 * 
 * PERFORMANCE NOTE:
 * -----------------
 * Mutex adds overhead:
 * - Lock/unlock: ~20-100 nanoseconds (uncontended)
 * - Contention: Can be much slower if threads wait
 * 
 * But correctness > speed!
 * Better to be slow and correct than fast and wrong.
 * 
 * 
 * BEST PRACTICES:
 * ---------------
 * 1. Always check return values
 * 2. Keep critical sections small
 * 3. Always unlock (even on error paths)
 * 4. Destroy mutex when done
 * 5. Document which mutex protects which data
 * 
 * 
 * TRY THIS:
 * ---------
 * 1. Run multiple times - always correct!
 * 2. Compare speed with 01_race_condition.c (slower but correct)
 * 3. Move lock/unlock outside loop - much faster! (why?)
 * 4. Comment out unlock - program hangs (deadlock)
 * 5. Add more threads - still correct!
 * 
 * 
 * OPTIMIZATION:
 * -------------
 * Current code locks/unlocks 1 million times per thread.
 * Better approach - lock once, do all increments, unlock once:
 * 
 * pthread_mutex_lock(&counter_mutex);
 * for (i = 0; i < INCREMENTS; i++) {
 *     counter++;
 * }
 * pthread_mutex_unlock(&counter_mutex);
 * 
 * Much faster! But less concurrent (threads wait longer).
 * Trade-off: Fine-grained vs Coarse-grained locking.
 * 
 * 
 * COMMON MISTAKES:
 * ----------------
 * ❌ Forgetting to unlock
 * ❌ Not checking return values
 * ❌ Locking wrong mutex
 * ❌ Holding lock too long
 * ❌ Not destroying mutex
 * 
 * 
 * NEXT STEP:
 * ----------
 * → 03_deadlock.c - Learn about deadlock (what NOT to do!)
 */
