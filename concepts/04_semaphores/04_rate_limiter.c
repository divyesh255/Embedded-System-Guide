/**
 * 04_rate_limiter.c - Rate Limiting with Semaphore
 * 
 * Practical example: limit API requests per second.
 * 
 * Compile: gcc -pthread -o 04_rate_limiter 04_rate_limiter.c
 * Run: ./04_rate_limiter
 * 
 * Study time: 20 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_REQUESTS 10
#define MAX_CONCURRENT 3

sem_t rate_limiter;

void *make_request(void *arg) {
    int id = *(int *)arg;
    
    printf("[Request %d] Waiting for rate limiter...\n", id);
    
    sem_wait(&rate_limiter);  /* Acquire token */
    
    time_t now = time(NULL);
    printf("[Request %d] Making API call at %s", id, ctime(&now));
    
    sleep(1);  /* Simulate API call */
    
    printf("[Request %d] Completed\n", id);
    
    sem_post(&rate_limiter);  /* Release token */
    
    return NULL;
}

int main(void) {
    pthread_t requests[NUM_REQUESTS];
    int ids[NUM_REQUESTS];
    
    printf("=== Rate Limiter Demo ===\n");
    printf("Max concurrent requests: %d\n\n", MAX_CONCURRENT);
    
    sem_init(&rate_limiter, 0, MAX_CONCURRENT);
    
    for (int i = 0; i < NUM_REQUESTS; i++) {
        ids[i] = i + 1;
        pthread_create(&requests[i], NULL, make_request, &ids[i]);
        usleep(50000);  /* Stagger requests */
    }
    
    for (int i = 0; i < NUM_REQUESTS; i++) {
        pthread_join(requests[i], NULL);
    }
    
    printf("\nAll requests completed!\n");
    sem_destroy(&rate_limiter);
    
    return 0;
}

/*
 * Rate limiting pattern:
 * - Semaphore limits concurrent operations
 * - Useful for API rate limits, connection pools
 * - Prevents overwhelming external services
 * 
 * NEXT: 05_exercises.md
 */
