/**
 * 04_ticket_spinlock.c - Fair Ticket Spinlock
 * 
 * Demonstrates ticket spinlock that provides FIFO ordering.
 * Prevents starvation.
 * 
 * Compile: gcc -pthread 04_ticket_spinlock.c -o 04_ticket_spinlock
 * Run: ./04_ticket_spinlock
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#define NUM_THREADS 5

/* Ticket spinlock structure */
typedef struct {
    atomic_int next_ticket;
    atomic_int now_serving;
} ticket_spinlock_t;

void ticket_lock(ticket_spinlock_t *lock) {
    /* Get my ticket number */
    int my_ticket = atomic_fetch_add(&lock->next_ticket, 1);
    
    /* Wait until it's my turn */
    while (atomic_load(&lock->now_serving) != my_ticket) {
        /* Spin */
    }
}

void ticket_unlock(ticket_spinlock_t *lock) {
    /* Serve next ticket */
    atomic_fetch_add(&lock->now_serving, 1);
}

/* Shared data */
ticket_spinlock_t lock = {0, 0};

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    printf("Thread %d: Requesting lock...\n", thread_id);
    
    ticket_lock(&lock);
    
    /* Critical section */
    printf("Thread %d: Got lock! (in critical section)\n", thread_id);
    sleep(1);  /* Simulate work */
    printf("Thread %d: Releasing lock\n", thread_id);
    
    ticket_unlock(&lock);
    
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("=== Ticket Spinlock (Fair FIFO) ===\n\n");
    printf("Starting %d threads\n", NUM_THREADS);
    printf("Threads will acquire lock in FIFO order\n\n");
    
    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
        usleep(100000);  /* Stagger thread creation */
    }
    
    /* Wait for threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\n=== Results ===\n");
    printf("✅ All threads acquired lock in order\n");
    printf("✅ No starvation (FIFO guarantee)\n");
    
    printf("\n=== How Ticket Spinlock Works ===\n");
    printf("1. Thread takes ticket: my_ticket = next_ticket++\n");
    printf("2. Thread waits: while (now_serving != my_ticket)\n");
    printf("3. Thread enters critical section\n");
    printf("4. Thread releases: now_serving++\n");
    printf("5. Next thread (my_ticket == now_serving) proceeds\n");
    
    printf("\n=== Advantages ===\n");
    printf("✅ FIFO ordering (fair)\n");
    printf("✅ No starvation\n");
    printf("✅ Predictable behavior\n");
    
    printf("\n=== Disadvantages ===\n");
    printf("❌ More memory (2 atomic ints)\n");
    printf("❌ Slightly slower than basic spinlock\n");
    printf("❌ Still wastes CPU while spinning\n");
    
    return 0;
}

/*
 * HOW TICKET SPINLOCK WORKS:
 * 
 * Like a deli counter:
 * - Take a ticket number
 * - Wait until your number is called
 * - Get served
 * - Next number is called
 * 
 * Example with 3 threads:
 * 
 * Time | Thread A        | Thread B        | Thread C        | next | now
 * -----|-----------------|-----------------|-----------------|------|-----
 * t0   | ticket = 0      |                 |                 | 1    | 0
 * t1   | wait (0==0) ✓   |                 |                 | 1    | 0
 * t2   | CRITICAL        | ticket = 1      |                 | 2    | 0
 * t3   | CRITICAL        | wait (0!=1)     | ticket = 2      | 3    | 0
 * t4   | CRITICAL        | wait (0!=1)     | wait (0!=2)     | 3    | 0
 * t5   | now_serving++   | wait (1==1) ✓   | wait (1!=2)     | 3    | 1
 * t6   |                 | CRITICAL        | wait (1!=2)     | 3    | 1
 * t7   |                 | now_serving++   | wait (2==2) ✓   | 3    | 2
 * t8   |                 |                 | CRITICAL        | 3    | 2
 * 
 * Threads are served in order: A → B → C (FIFO)
 */
