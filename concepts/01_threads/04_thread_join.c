/**
 * 04_thread_join.c - Thread Synchronization with Join
 * 
 * Demonstrates pthread_join() for synchronization and collecting results.
 * 
 * Compile: gcc -pthread -o 04_thread_join 04_thread_join.c
 * Run: ./04_thread_join
 * 
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *compute_sum(void *arg)
{
    int n = *(int *)arg;
    int *result = malloc(sizeof(int));
    *result = (n * (n + 1)) / 2;  /* Sum of 1 to n */
    
    printf("[Thread] Computing sum of 1 to %d...\n", n);
    sleep(2);
    printf("[Thread] Result: %d\n", *result);
    
    return result;
}

int main(void)
{
    pthread_t thread;
    int n = 100;
    void *result;
    
    printf("Main: Creating thread to compute sum...\n");
    pthread_create(&thread, NULL, compute_sum, &n);
    
    printf("Main: Doing other work while thread computes...\n");
    sleep(1);
    printf("Main: Still working...\n");
    
    printf("Main: Waiting for thread result...\n");
    pthread_join(thread, &result);
    
    printf("Main: Thread returned: %d\n", *(int *)result);
    free(result);
    
    return 0;
}

/* 
 * KEY POINTS:
 * - pthread_join() blocks until thread finishes
 * - Second parameter receives thread's return value
 * - Always free() returned heap memory
 * - Main thread can do work while waiting
 */
