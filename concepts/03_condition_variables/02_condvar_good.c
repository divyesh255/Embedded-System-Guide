/**
 * 02_condvar_good.c - Condition Variable Solution
 * 
 * Shows efficient waiting with condition variables. CPU usage near 0%!
 * 
 * Compile: gcc -pthread -o 02_condvar_good 02_condvar_good.c
 * Run: ./02_condvar_good
 * 
 * Study time: 20 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *waiter_thread(void *arg) {
    (void)arg;
    
    printf("[Waiter] Waiting for data (using condition variable)...\n");
    printf("[Waiter] I'll sleep efficiently - 0%% CPU!\n");
    
    pthread_mutex_lock(&mutex);
    
    /* GOOD: Efficient waiting with condition variable */
    while (!data_ready) {
        pthread_cond_wait(&cond, &mutex);  /* Sleep here! */
    }
    
    pthread_mutex_unlock(&mutex);
    
    printf("[Waiter] Woke up! Data is ready! Processing...\n");
    return NULL;
}

void *producer_thread(void *arg) {
    (void)arg;
    
    printf("[Producer] Working for 5 seconds...\n");
    sleep(5);
    
    pthread_mutex_lock(&mutex);
    data_ready = 1;
    pthread_cond_signal(&cond);  /* Wake up waiter! */
    pthread_mutex_unlock(&mutex);
    
    printf("[Producer] Data ready, signaled waiter!\n");
    return NULL;
}

int main(void) {
    pthread_t waiter, producer;
    
    printf("=== Condition Variable Demonstration ===\n");
    printf("Watch CPU usage - should be near 0%%!\n\n");
    
    if (pthread_create(&waiter, NULL, waiter_thread, NULL) != 0) {
        perror("create waiter failed");
        return 1;
    }
    
    if (pthread_create(&producer, NULL, producer_thread, NULL) != 0) {
        perror("create producer failed");
        return 1;
    }
    
    pthread_join(waiter, NULL);
    pthread_join(producer, NULL);
    
    printf("\nDone! CPU usage was minimal.\n");
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}

/*
 * KEY POINTS:
 * - pthread_cond_wait() atomically unlocks mutex and sleeps
 * - Thread uses 0% CPU while waiting
 * - pthread_cond_signal() wakes up the waiting thread
 * - Always use while loop (handles spurious wakeups)
 * 
 * NEXT: 03_producer_consumer.c - Classic pattern
 */
