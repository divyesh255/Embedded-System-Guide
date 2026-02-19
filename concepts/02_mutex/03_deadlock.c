/**
 * 03_deadlock.c - Demonstrating Deadlock
 * 
 * Shows how improper lock ordering causes deadlock.
 * WARNING: This program will HANG! Press Ctrl+C to kill it.
 * 
 * Compile: gcc -pthread -o 03_deadlock 03_deadlock.c
 * Run: ./03_deadlock (will hang - press Ctrl+C)
 * 
 * Study time: 20 minutes
 * Difficulty: Intermediate
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1_func(void *arg)
{
    (void)arg;
    
    printf("[Thread 1] Trying to lock lock1...\n");
    pthread_mutex_lock(&lock1);
    printf("[Thread 1] Got lock1!\n");
    
    sleep(1);  /* Give thread 2 time to lock lock2 */
    
    printf("[Thread 1] Trying to lock lock2...\n");
    pthread_mutex_lock(&lock2);  /* DEADLOCK! Thread 2 has this */
    printf("[Thread 1] Got lock2! (will never print)\n");
    
    pthread_mutex_unlock(&lock2);
    pthread_mutex_unlock(&lock1);
    return NULL;
}

void *thread2_func(void *arg)
{
    (void)arg;
    
    printf("[Thread 2] Trying to lock lock2...\n");
    pthread_mutex_lock(&lock2);
    printf("[Thread 2] Got lock2!\n");
    
    sleep(1);  /* Give thread 1 time to lock lock1 */
    
    printf("[Thread 2] Trying to lock lock1...\n");
    pthread_mutex_lock(&lock1);  /* DEADLOCK! Thread 1 has this */
    printf("[Thread 2] Got lock1! (will never print)\n");
    
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    
    printf("=== Deadlock Demonstration ===\n");
    printf("WARNING: This will hang! Press Ctrl+C to exit.\n\n");
    
    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);
    
    pthread_join(t1, NULL);  /* Will wait forever */
    pthread_join(t2, NULL);
    
    printf("Done! (will never print)\n");
    return 0;
}

/*
 * OUTPUT (then hangs):
 * [Thread 1] Trying to lock lock1...
 * [Thread 1] Got lock1!
 * [Thread 2] Trying to lock lock2...
 * [Thread 2] Got lock2!
 * [Thread 1] Trying to lock lock2...
 * [Thread 2] Trying to lock lock1...
 * ← HANGS HERE! Both threads waiting forever
 * 
 * SOLUTION: Always lock in same order!
 * 
 * NEXT: 04_trylock.c - Non-blocking alternative
 */
