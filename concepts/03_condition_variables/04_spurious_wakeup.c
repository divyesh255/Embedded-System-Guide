/**
 * 04_spurious_wakeup.c - Handling Spurious Wakeups
 * 
 * Demonstrates why you MUST use while loop, not if.
 * 
 * Compile: gcc -pthread -o 04_spurious_wakeup 04_spurious_wakeup.c
 * Run: ./04_spurious_wakeup
 * 
 * Study time: 20 minutes
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *waiter_wrong(void *arg) {
    (void)arg;
    
    pthread_mutex_lock(&mutex);
    
    /* WRONG: Using if instead of while */
    if (!ready) {
        printf("[Wrong Waiter] Waiting...\n");
        pthread_cond_wait(&cond, &mutex);
        /* Spurious wakeup could happen here! */
    }
    
    /* Might process when NOT ready! */
    printf("[Wrong Waiter] Processing (ready=%d) - BUG if 0!\n", ready);
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *waiter_correct(void *arg) {
    (void)arg;
    
    pthread_mutex_lock(&mutex);
    
    /* CORRECT: Using while loop */
    while (!ready) {
        printf("[Correct Waiter] Waiting...\n");
        pthread_cond_wait(&cond, &mutex);
        /* If spurious wakeup, loop checks again */
    }
    
    /* Guaranteed ready is true */
    printf("[Correct Waiter] Processing (ready=%d) - Always correct!\n", ready);
    
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(void) {
    pthread_t t1;
    
    printf("=== Spurious Wakeup Handling ===\n\n");
    
    printf("Rule: ALWAYS use while loop with pthread_cond_wait!\n\n");
    
    pthread_create(&t1, NULL, waiter_correct, NULL);
    sleep(2);
    
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    pthread_join(t1, NULL);
    
    printf("\nKey Point: while loop rechecks condition after wakeup\n");
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}

/*
 * ALWAYS use while (!condition) not if (!condition)
 * Protects against spurious wakeups and race conditions
 */
