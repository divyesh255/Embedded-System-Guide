/**
 * 01_busy_wait_bad.c - Busy-Waiting Problem
 * 
 * Demonstrates why busy-waiting is inefficient. Watch CPU usage!
 * 
 * Compile: gcc -pthread -o 01_busy_wait_bad 01_busy_wait_bad.c
 * Run: ./01_busy_wait_bad (watch CPU usage with 'top' or 'htop')
 * 
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *waiter_thread(void *arg) {
    (void)arg;
    
    printf("[Waiter] Waiting for data (busy-waiting)...\n");
    printf("[Waiter] Check CPU usage - I'm wasting 100%% of a core!\n");
    
    /* BAD: Busy-waiting loop */
    pthread_mutex_lock(&mutex);
    while (!data_ready) {
        pthread_mutex_unlock(&mutex);
        /* Spinning here wastes CPU! */
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
    
    printf("[Waiter] Data is ready! Processing...\n");
    return NULL;
}

void *producer_thread(void *arg) {
    (void)arg;
    
    printf("[Producer] Working for 5 seconds...\n");
    sleep(5);
    
    pthread_mutex_lock(&mutex);
    data_ready = 1;
    pthread_mutex_unlock(&mutex);
    
    printf("[Producer] Data is ready!\n");
    return NULL;
}

int main(void) {
    pthread_t waiter, producer;
    
    printf("=== Busy-Waiting Demonstration ===\n");
    printf("Watch CPU usage while this runs!\n\n");
    
    pthread_create(&waiter, NULL, waiter_thread, NULL);
    pthread_create(&producer, NULL, producer_thread, NULL);
    
    pthread_join(waiter, NULL);
    pthread_join(producer, NULL);
    
    printf("\nDone! Notice how much CPU was wasted.\n");
    pthread_mutex_destroy(&mutex);
    
    return 0;
}

/*
 * PROBLEM: Waiter thread spins at 100% CPU for 5 seconds!
 * SOLUTION: Use condition variable (see 02_condvar_good.c)
 */
