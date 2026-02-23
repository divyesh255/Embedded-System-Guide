/**
 * 02_counting_semaphore.c - Resource Pool Management
 * 
 * Demonstrates counting semaphore limiting concurrent access.
 * 
 * Compile: gcc -pthread -o 02_counting_semaphore 02_counting_semaphore.c
 * Run: ./02_counting_semaphore
 * 
 * Study time: 20 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_WORKERS 10
#define MAX_RESOURCES 3

sem_t resource_pool;

void *worker(void *arg) {
    int id = *(int *)arg;
    
    printf("[Worker %d] Waiting for resource...\n", id);
    
    sem_wait(&resource_pool);  /* Acquire resource */
    
    int value;
    sem_getvalue(&resource_pool, &value);
    printf("[Worker %d] Got resource! (Available: %d)\n", id, value);
    
    sleep(2);  /* Use resource */
    
    printf("[Worker %d] Releasing resource\n", id);
    sem_post(&resource_pool);  /* Release resource */
    
    return NULL;
}

int main(void) {
    pthread_t workers[NUM_WORKERS];
    int ids[NUM_WORKERS];
    
    printf("=== Counting Semaphore Demo ===\n");
    printf("Max concurrent resources: %d\n", MAX_RESOURCES);
    printf("Total workers: %d\n\n", NUM_WORKERS);
    
    /* Initialize counting semaphore */
    if (sem_init(&resource_pool, 0, MAX_RESOURCES) != 0) {
        perror("sem_init failed");
        return 1;
    }
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        ids[i] = i + 1;
        pthread_create(&workers[i], NULL, worker, &ids[i]);
        usleep(100000);  /* Stagger starts */
    }
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
    
    printf("\nAll workers done!\n");
    sem_destroy(&resource_pool);
    
    return 0;
}

/*
 * Counting semaphore limits concurrent access:
 * - Initial value = MAX_RESOURCES
 * - Only MAX_RESOURCES threads can proceed
 * - Others wait until a resource is released
 * 
 * NEXT: 03_producer_consumer.c
 */
