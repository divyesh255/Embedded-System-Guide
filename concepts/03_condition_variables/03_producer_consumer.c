/**
 * 03_producer_consumer.c - Classic Producer-Consumer Pattern
 * 
 * Demonstrates the classic producer-consumer problem solved with
 * condition variables. Multiple producers and consumers.
 * 
 * Compile: gcc -pthread -o 03_producer_consumer 03_producer_consumer.c
 * Run: ./03_producer_consumer
 * 
 * Study time: 30 minutes
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_ITEMS 20

int buffer[BUFFER_SIZE];
int count = 0;
int in = 0;
int out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS / 2; i++) {
        int item = id * 100 + i;
        
        pthread_mutex_lock(&mutex);
        
        while (count == BUFFER_SIZE) {
            printf("[Producer %d] Buffer full, waiting...\n", id);
            pthread_cond_wait(&not_full, &mutex);
        }
        
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        printf("[Producer %d] Produced %d (count=%d)\n", id, item, count);
        
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
        
        usleep(100000);  /* Simulate work */
    }
    
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < NUM_ITEMS / 2; i++) {
        pthread_mutex_lock(&mutex);
        
        while (count == 0) {
            printf("[Consumer %d] Buffer empty, waiting...\n", id);
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        printf("[Consumer %d] Consumed %d (count=%d)\n", id, item, count);
        
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        usleep(150000);  /* Simulate work */
    }
    
    return NULL;
}

int main(void) {
    pthread_t prod1, prod2, cons1, cons2;
    int id1 = 1, id2 = 2;
    
    printf("=== Producer-Consumer Pattern ===\n");
    printf("Buffer size: %d\n", BUFFER_SIZE);
    printf("Total items: %d\n\n", NUM_ITEMS);
    
    pthread_create(&prod1, NULL, producer, &id1);
    pthread_create(&prod2, NULL, producer, &id2);
    pthread_create(&cons1, NULL, consumer, &id1);
    pthread_create(&cons2, NULL, consumer, &id2);
    
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    
    printf("\nAll done!\n");
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
    
    return 0;
}

/*
 * KEY PATTERN:
 * - Two condition variables (not_empty, not_full)
 * - Producers wait when buffer full
 * - Consumers wait when buffer empty
 * - Signal opposite condition after operation
 * 
 * NEXT: 04_spurious_wakeup.c
 */
