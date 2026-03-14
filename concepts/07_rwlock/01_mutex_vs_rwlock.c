/**
 * 01_mutex_vs_rwlock.c - Performance Comparison
 * 
 * Compares mutex vs rwlock performance for read-heavy workload.
 * 
 * Compile: gcc -pthread 01_mutex_vs_rwlock.c -o 01_mutex_vs_rwlock
 * Run: ./01_mutex_vs_rwlock
 */

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define NUM_READERS 8
#define NUM_WRITERS 2
#define ITERATIONS 100000

/* Shared data */
int shared_data = 0;

/* Mutex version */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* reader_mutex(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        volatile int value = shared_data;  // Read
        (void)value;  // Suppress unused warning
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* writer_mutex(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        shared_data++;  // Write
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

/* Rwlock version */
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void* reader_rwlock(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_rwlock_rdlock(&rwlock);
        volatile int value = shared_data;  // Read
        (void)value;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

void* writer_rwlock(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_rwlock_wrlock(&rwlock);
        shared_data++;  // Write
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

double benchmark_mutex(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Create reader threads */
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_create(&readers[i], NULL, reader_mutex, NULL);
    }
    
    /* Create writer threads */
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&writers[i], NULL, writer_mutex, NULL);
    }
    
    /* Wait for all threads */
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    return (end.tv_sec - start.tv_sec) + 
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

double benchmark_rwlock(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Create reader threads */
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_create(&readers[i], NULL, reader_rwlock, NULL);
    }
    
    /* Create writer threads */
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&writers[i], NULL, writer_rwlock, NULL);
    }
    
    /* Wait for all threads */
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    return (end.tv_sec - start.tv_sec) + 
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void) {
    printf("=== Mutex vs Read-Write Lock Performance ===\n\n");
    printf("Workload: %d readers, %d writers\n", NUM_READERS, NUM_WRITERS);
    printf("Each thread: %d iterations\n", ITERATIONS);
    printf("Read/Write ratio: %d:1 (read-heavy)\n\n", NUM_READERS/NUM_WRITERS);
    
    /* Benchmark mutex */
    shared_data = 0;
    double time_mutex = benchmark_mutex();
    printf("Mutex:  %.3f seconds\n", time_mutex);
    
    /* Benchmark rwlock */
    shared_data = 0;
    double time_rwlock = benchmark_rwlock();
    printf("Rwlock: %.3f seconds\n", time_rwlock);
    
    printf("\n=== Results ===\n");
    if (time_rwlock < time_mutex) {
        printf("✅ Rwlock is %.2fx FASTER\n", time_mutex / time_rwlock);
        printf("   Multiple readers can read concurrently!\n");
    } else {
        printf("❌ Mutex is %.2fx faster\n", time_rwlock / time_mutex);
        printf("   Rwlock overhead not worth it for this workload\n");
        printf("\n=== Why Mutex Won ===\n");
        printf("1. Critical section too short (just read/write int)\n");
        printf("2. Rwlock has overhead (tracking readers, conditions)\n");
        printf("3. Only 80%% reads - need 90%%+ for rwlock to win\n");
    }
    
    printf("\n=== Why Rwlock Wins ===\n");
    printf("Mutex:  Only 1 thread at a time (even readers!)\n");
    printf("Rwlock: %d readers can read simultaneously\n", NUM_READERS);
    printf("        Writers still exclusive (1 at a time)\n");
    
    printf("\n=== When to Use Each ===\n");
    printf("Mutex:  Write-heavy, short critical sections, simple\n");
    printf("Rwlock: Read-heavy (90%%+ reads), longer critical sections\n");
    printf("\n=== Key Lesson ===\n");
    printf("Rwlock has overhead! Only use when:\n");
    printf("  • 90%%+ reads (not just 80%%)\n");
    printf("  • Longer critical sections\n");
    printf("  • Many concurrent readers\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT (approximate):
 * 
 * Mutex:  2.5 seconds
 * Rwlock: 0.8 seconds
 * 
 * Rwlock is 3x FASTER!
 * 
 * WHY?
 * - Mutex: 10 threads serialize (1 at a time)
 * - Rwlock: 8 readers concurrent, 2 writers serialize
 * - Result: Much better throughput!
 */
