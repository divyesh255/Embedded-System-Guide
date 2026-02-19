/**
 * 01_race_condition.c - Demonstrating Race Condition
 * 
 * This program shows what happens when multiple threads access shared
 * data without synchronization. Run it multiple times - you'll get
 * different (wrong) results each time!
 * 
 * Compile: gcc -pthread -o 01_race_condition 01_race_condition.c
 * Run: ./01_race_condition
 * 
 * Study time: 15 minutes
 * Difficulty: Beginner
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2
#define INCREMENTS 1000000

/* Shared variable - DANGER ZONE! */
int counter = 0;

/**
 * increment_counter - Increments shared counter (UNSAFE!)
 * 
 * This function has a race condition. The counter++ operation
 * is NOT atomic - it's actually three operations:
 * 1. Read counter value
 * 2. Add 1
 * 3. Write back
 * 
 * Multiple threads can interleave these operations, causing lost updates.
 */
void *increment_counter(void *arg)
{
    (void)arg;  /* Suppress unused warning */
    int i;
    
    printf("[Thread %lu] Starting to increment...\n", pthread_self());
    
    for (i = 0; i < INCREMENTS; i++) {
        counter++;  /* RACE CONDITION HERE! */
    }
    
    printf("[Thread %lu] Finished incrementing\n", pthread_self());
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    int i;
    int expected = NUM_THREADS * INCREMENTS;
    
    printf("=== Race Condition Demonstration ===\n\n");
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
    printf("Lost updates: %d\n", expected - counter);
    
    if (counter == expected) {
        printf("\n✓ Correct! (Got lucky this time)\n");
    } else {
        printf("\n✗ WRONG! Race condition caused lost updates!\n");
        printf("\nWhy this happened:\n");
        printf("- counter++ is NOT atomic\n");
        printf("- Threads interleaved their read-modify-write operations\n");
        printf("- Some increments were lost\n");
    }
    
    printf("\nTry running again - you'll likely get a different result!\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT (varies each run!):
 * -----------------------------------
 * === Race Condition Demonstration ===
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
 * Final counter value: 1234567  ← WRONG!
 * Expected value: 2000000
 * Lost updates: 765433
 * 
 * ✗ WRONG! Race condition caused lost updates!
 * 
 * Why this happened:
 * - counter++ is NOT atomic
 * - Threads interleaved their read-modify-write operations
 * - Some increments were lost
 * 
 * Try running again - you'll likely get a different result!
 * 
 * 
 * WHAT'S HAPPENING:
 * -----------------
 * 
 * Thread 1                    Thread 2
 * --------                    --------
 * Read counter (0)
 *                             Read counter (0)  ← Both read 0!
 * Add 1 (0 + 1 = 1)
 *                             Add 1 (0 + 1 = 1) ← Both compute 1!
 * Write 1
 *                             Write 1           ← Lost one increment!
 * 
 * This happens thousands of times, causing many lost updates.
 * 
 * 
 * KEY CONCEPTS:
 * -------------
 * 1. Race Condition: Multiple threads accessing shared data unsafely
 * 2. Non-Atomic Operation: counter++ is three separate operations
 * 3. Lost Updates: Concurrent writes overwrite each other
 * 4. Non-Deterministic: Results vary each run
 * 5. Hard to Debug: May work sometimes, fail others
 * 
 * 
 * TRY THIS:
 * ---------
 * 1. Run the program 5 times - notice different results
 * 2. Change NUM_THREADS to 4 or 8 - more lost updates
 * 3. Change INCREMENTS to 10000 - fewer lost updates (why?)
 * 4. Add printf inside loop - race becomes more visible
 * 5. Run with valgrind --tool=helgrind to detect race
 * 
 * 
 * WHY RESULTS VARY:
 * -----------------
 * - Thread scheduling is non-deterministic
 * - CPU speed affects interleaving
 * - System load changes timing
 * - Cache effects vary
 * - No guarantees without synchronization!
 * 
 * 
 * REAL-WORLD IMPACT:
 * ------------------
 * Race conditions cause:
 * - Incorrect calculations (like this)
 * - Data corruption
 * - Crashes (accessing freed memory)
 * - Security vulnerabilities
 * - Intermittent bugs (hardest to debug!)
 * 
 * 
 * THE SOLUTION:
 * -------------
 * Use a MUTEX to protect the shared counter!
 * See 02_mutex_solution.c for the fix.
 * 
 * 
 * NEXT STEP:
 * ----------
 * → 02_mutex_solution.c - See how mutex fixes this problem
 */
