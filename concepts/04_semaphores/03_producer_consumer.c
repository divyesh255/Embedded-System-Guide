/**
 * 03_producer_consumer.c - Producer-Consumer with Semaphores
 * 
 * Classic pattern using three semaphores.
 * 
 * Compile: gcc -pthread -o 03_producer_consumer 03_producer_consumer.c
 * Run: ./03_producer_consumer
 * 
 * Study time: 25 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 15

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

sem_t empty;  /* Count of empty slots */
sem_t full;   /* Count of full slots */
sem_t mutex;  /* Protect buffer access */

void *producer(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS / 2; i++) {
        int item = id * 100 + i;
        
        sem_wait(&empty);  /* Wait for empty slot */
        sem_wait(&mutex);  /* Lock buffer */
        
        buffer[in] = item;
        printf("[Producer %d] Produced %d at index %d\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        
        sem_post(&mutex);  /* Unlock buffer */
        sem_post(&full);   /* Signal full slot */
        
        usleep(100000);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS / 2; i++) {
        sem_wait(&full);   /* Wait for full slot */
        sem_wait(&mutex);  /* Lock buffer */
        
        int item = buffer[out];
        printf("[Consumer %d] Consumed %d from index %d\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        
        sem_post(&mutex);  /* Unlock buffer */
        sem_post(&empty);  /* Signal empty slot */
        
        usleep(150000);
    }
    
    return NULL;
}

int main(void) {
    pthread_t prod1, prod2, cons1, cons2;
    int id1 = 1, id2 = 2;
    
    printf("=== Producer-Consumer with Semaphores ===\n");
    printf("Buffer size: %d\n\n", BUFFER_SIZE);
    
    sem_init(&empty, 0, BUFFER_SIZE);  /* All slots empty */
    sem_init(&full, 0, 0);             /* No slots full */
    sem_init(&mutex, 0, 1);            /* Binary semaphore */
    
    pthread_create(&prod1, NULL, producer, &id1);
    pthread_create(&prod2, NULL, producer, &id2);
    pthread_create(&cons1, NULL, consumer, &id1);
    pthread_create(&cons2, NULL, consumer, &id2);
    
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    
    printf("\nAll done!\n");
    
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    return 0;
}

/*
 * Three semaphores pattern:
 * - empty: tracks empty slots
 * - full: tracks full slots
 * - mutex: protects buffer
 * 
 * NEXT: 04_rate_limiter.c
 */
