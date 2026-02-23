/**
 * 01_binary_semaphore.c - Binary Semaphore (Like Mutex)
 * 
 * Demonstrates binary semaphore for mutual exclusion.
 * 
 * Compile: gcc -pthread -o 01_binary_semaphore 01_binary_semaphore.c
 * Run: ./01_binary_semaphore
 * 
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 3
#define INCREMENTS 100000

int counter = 0;
sem_t binary_sem;  /* Binary semaphore (initial value = 1) */

void *increment_counter(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < INCREMENTS; i++) {
        sem_wait(&binary_sem);  /* Lock */
        counter++;
        sem_post(&binary_sem);  /* Unlock */
    }
    
    printf("[Thread %d] Finished\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS] = {1, 2, 3};
    int expected = NUM_THREADS * INCREMENTS;
    
    printf("=== Binary Semaphore Demo ===\n\n");
    
    /* Initialize binary semaphore (value = 1) */
    if (sem_init(&binary_sem, 0, 1) != 0) {
        perror("sem_init failed");
        return 1;
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, increment_counter, &ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal counter: %d\n", counter);
    printf("Expected: %d\n", expected);
    
    if (counter == expected) {
        printf("✓ Correct! Binary semaphore protected the counter.\n");
    } else {
        printf("✗ Wrong! Lost updates: %d\n", expected - counter);
    }
    
    sem_destroy(&binary_sem);
    return 0;
}

/*
 * Binary semaphore acts like a mutex:
 * - Initial value = 1 (unlocked)
 * - sem_wait() decrements to 0 (locked)
 * - sem_post() increments to 1 (unlocked)
 * 
 * NEXT: 02_counting_semaphore.c
 */
