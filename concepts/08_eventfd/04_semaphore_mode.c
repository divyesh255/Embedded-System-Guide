/**
 * 04_semaphore_mode.c - EFD_SEMAPHORE Flag
 * 
 * Demonstrates semaphore-like behavior with EFD_SEMAPHORE.
 * 
 * Compile: gcc -pthread 04_semaphore_mode.c -o 04_semaphore_mode
 * Run: ./04_semaphore_mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>

int efd_normal, efd_semaphore;

void demonstrate_normal_mode(void) {
    printf("=== Normal Mode (default) ===\n\n");
    
    efd_normal = eventfd(0, 0);
    
    /* Write 5 events */
    printf("Writing 5 events (1+1+1+1+1)...\n");
    for (int i = 0; i < 5; i++) {
        uint64_t val = 1;
        write(efd_normal, &val, sizeof(val));
    }
    printf("Counter = 5\n\n");
    
    /* Single read gets all */
    uint64_t result;
    read(efd_normal, &result, sizeof(result));
    printf("Read once: got %lu\n", result);
    printf("Counter reset to 0\n\n");
    
    close(efd_normal);
}

void demonstrate_semaphore_mode(void) {
    printf("=== Semaphore Mode (EFD_SEMAPHORE) ===\n\n");
    
    efd_semaphore = eventfd(0, EFD_SEMAPHORE);
    
    /* Write 5 events */
    printf("Writing 5 events (1+1+1+1+1)...\n");
    for (int i = 0; i < 5; i++) {
        uint64_t val = 1;
        write(efd_semaphore, &val, sizeof(val));
    }
    printf("Counter = 5\n\n");
    
    /* Each read gets 1 and decrements */
    printf("Reading 5 times:\n");
    for (int i = 0; i < 5; i++) {
        uint64_t result;
        read(efd_semaphore, &result, sizeof(result));
        printf("  Read %d: got %lu (counter now %d)\n", i+1, result, 4-i);
    }
    printf("Counter = 0\n\n");
    
    close(efd_semaphore);
}

int main(void) {
    printf("=== eventfd: Normal vs Semaphore Mode ===\n\n");
    
    demonstrate_normal_mode();
    demonstrate_semaphore_mode();
    
    printf("=== Comparison ===\n\n");
    
    printf("Normal Mode:\n");
    printf("  write(5) → counter = 5\n");
    printf("  read()   → returns 5, counter = 0\n");
    printf("  Use: Event counting, batch processing\n\n");
    
    printf("Semaphore Mode (EFD_SEMAPHORE):\n");
    printf("  write(5) → counter = 5\n");
    printf("  read()   → returns 1, counter = 4\n");
    printf("  read()   → returns 1, counter = 3\n");
    printf("  ... (5 reads total)\n");
    printf("  Use: Resource counting, work queue\n\n");
    
    printf("=== When to Use Each ===\n");
    printf("Normal:    Accumulate events, single consumer\n");
    printf("Semaphore: Resource pool, multiple consumers\n");
    
    return 0;
}

/*
 * KEY DIFFERENCE:
 * 
 * Normal Mode:
 *   - read() returns counter value
 *   - Counter resets to 0
 *   - Good for: Counting total events
 * 
 * Semaphore Mode (EFD_SEMAPHORE):
 *   - read() always returns 1
 *   - Counter decrements by 1
 *   - Good for: Resource counting
 * 
 * Example Use Case for Semaphore Mode:
 *   - Thread pool with N available workers
 *   - write(N) to initialize
 *   - Each task does read() to claim a worker
 *   - write(1) when worker becomes available
 */
