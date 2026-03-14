/**
 * 02_thread_notification.c - Thread Completion Notification
 * 
 * Multiple workers signal completion via eventfd.
 * 
 * Compile: gcc -pthread 02_thread_notification.c -o 02_thread_notification
 * Run: ./02_thread_notification
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>

#define NUM_WORKERS 5

int efd;

void* worker(void* arg) {
    int id = *(int*)arg;
    
    printf("[Worker %d] Starting task...\n", id);
    sleep(1 + id % 3);  /* Simulate variable work time */
    
    printf("[Worker %d] Task complete! Signaling...\n", id);
    uint64_t done = 1;
    write(efd, &done, sizeof(done));
    
    return NULL;
}

int main(void) {
    printf("=== Thread Notification with eventfd ===\n\n");
    
    /* Create eventfd */
    efd = eventfd(0, 0);
    if (efd == -1) {
        perror("eventfd");
        exit(EXIT_FAILURE);
    }
    
    /* Create worker threads */
    pthread_t workers[NUM_WORKERS];
    int ids[NUM_WORKERS];
    
    printf("Starting %d workers...\n\n", NUM_WORKERS);
    for (int i = 0; i < NUM_WORKERS; i++) {
        ids[i] = i;
        pthread_create(&workers[i], NULL, worker, &ids[i]);
    }
    
    /* Wait for all workers to complete */
    printf("[Main] Waiting for workers to complete...\n\n");
    
    uint64_t total_completions;
    ssize_t s = read(efd, &total_completions, sizeof(total_completions));
    if (s != sizeof(total_completions)) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    
    printf("\n[Main] Received %lu completion signals!\n", total_completions);
    printf("[Main] All workers done!\n");
    
    /* Join threads */
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
    
    close(efd);
    
    printf("\n=== How It Works ===\n");
    printf("1. Each worker writes 1 to eventfd when done\n");
    printf("2. eventfd counter accumulates: 1+1+1+1+1 = 5\n");
    printf("3. Main thread reads once, gets total count\n");
    printf("4. Counter resets to 0\n");
    
    printf("\n=== Advantages ===\n");
    printf("✅ Single read gets all notifications\n");
    printf("✅ No need for N separate reads\n");
    printf("✅ Counter accumulates events\n");
    printf("✅ Very efficient!\n");
    
    return 0;
}

/*
 * KEY CONCEPT: Event Accumulation
 * 
 * Unlike condition variables or semaphores,
 * eventfd ACCUMULATES events:
 * 
 * Worker 1: write(1) → counter = 1
 * Worker 2: write(1) → counter = 2
 * Worker 3: write(1) → counter = 3
 * Worker 4: write(1) → counter = 4
 * Worker 5: write(1) → counter = 5
 * 
 * Main: read() → gets 5, counter = 0
 * 
 * This is perfect for counting completions!
 */
