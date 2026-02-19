/**
 * 04_trylock.c - Non-blocking Mutex with trylock
 * 
 * Demonstrates pthread_mutex_trylock() for non-blocking lock attempts.
 * Useful when you don't want to wait for a lock.
 * 
 * Compile: gcc -pthread -o 04_trylock 04_trylock.c
 * Run: ./04_trylock
 * 
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t resource_lock = PTHREAD_MUTEX_INITIALIZER;
int shared_resource = 0;

void *worker_thread(void *arg)
{
    int id = *(int *)arg;
    int attempts = 0;
    int successes = 0;
    
    for (int i = 0; i < 5; i++) {
        attempts++;
        
        /* Try to lock - doesn't block! */
        int result = pthread_mutex_trylock(&resource_lock);
        
        if (result == 0) {
            /* Got the lock! */
            printf("[Worker %d] Got lock on attempt %d\n", id, attempts);
            shared_resource++;
            sleep(1);  /* Simulate work */
            pthread_mutex_unlock(&resource_lock);
            successes++;
        } else if (result == EBUSY) {
            /* Lock was busy - do something else */
            printf("[Worker %d] Lock busy, doing other work...\n", id);
            usleep(500000);  /* Do other work for 0.5 sec */
        } else {
            fprintf(stderr, "[Worker %d] trylock error\n", id);
        }
    }
    
    printf("[Worker %d] Success rate: %d/%d\n", id, successes, attempts);
    return NULL;
}

int main(void)
{
    pthread_t threads[3];
    int ids[3] = {1, 2, 3};
    
    printf("=== Trylock Demonstration ===\n\n");
    
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, &ids[i]) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nFinal resource value: %d\n", shared_resource);
    pthread_mutex_destroy(&resource_lock);
    
    return 0;
}

/*
 * KEY POINTS:
 * - trylock returns immediately (doesn't block)
 * - Returns 0 if lock acquired, EBUSY if locked
 * - Useful for avoiding deadlock
 * - Thread can do other work if lock busy
 * 
 * NEXT: 05_exercises.md - Practice problems
 */
