/**
 * 03_multiple_threads.c - Managing Multiple Threads
 * 
 * Demonstrates creating and managing multiple threads simultaneously.
 * Shows how to use arrays to handle many threads efficiently.
 * 
 * Compile: gcc -pthread -o 03_multiple_threads 03_multiple_threads.c
 * Run: ./03_multiple_threads
 * 
 * Study time: 20 minutes
 * Difficulty: Intermediate
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

struct worker_data {
    int id;
    int work_amount;
};

/**
 * worker_thread - Simulates a worker doing some task
 */
void *worker_thread(void *arg)
{
    struct worker_data *data = (struct worker_data *)arg;
    
    printf("[Worker %d] Starting work (%d units)...\n", 
           data->id, data->work_amount);
    
    /* Simulate work */
    sleep(data->work_amount);
    
    printf("[Worker %d] Completed!\n", data->id);
    
    return NULL;
}

int main(void)
{
    pthread_t threads[NUM_THREADS];
    struct worker_data *data[NUM_THREADS];
    int i;
    
    printf("Creating %d worker threads...\n\n", NUM_THREADS);
    
    /* Create all threads */
    for (i = 0; i < NUM_THREADS; i++) {
        data[i] = malloc(sizeof(struct worker_data));
        data[i]->id = i + 1;
        data[i]->work_amount = (i % 3) + 1;  /* 1-3 seconds */
        
        if (pthread_create(&threads[i], NULL, worker_thread, data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    
    printf("All threads created. Waiting for completion...\n\n");
    
    /* Wait for all threads */
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        free(data[i]);
    }
    
    printf("\nAll workers completed!\n");
    
    return 0;
}

/*
 * KEY CONCEPTS:
 * - Use arrays to manage multiple threads
 * - Create threads in a loop
 * - Join threads in a loop
 * - Each thread gets unique data
 * 
 * TRY THIS:
 * 1. Change NUM_THREADS to 10, 20, 100
 * 2. Make all threads do same amount of work
 * 3. Add a counter to track completed threads
 * 4. Return results from each thread
 * 
 * NEXT: 04_thread_join.c - Advanced synchronization
 */
