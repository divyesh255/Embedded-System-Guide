/**
 * 01_basic_eventfd.c - Basic eventfd Usage
 * 
 * Demonstrates basic eventfd creation, write, and read operations.
 * 
 * Compile: gcc -pthread 01_basic_eventfd.c -o 01_basic_eventfd
 * Run: ./01_basic_eventfd
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>

int efd;  /* Global eventfd */

void* writer_thread(void* arg) {
    printf("[Writer] Starting work...\n");
    sleep(2);  /* Simulate work */
    
    printf("[Writer] Work done! Signaling event...\n");
    uint64_t value = 1;
    ssize_t s = write(efd, &value, sizeof(value));
    if (s != sizeof(value)) {
        perror("write");
        return NULL;
    }
    
    printf("[Writer] Event signaled\n");
    return NULL;
}

void* reader_thread(void* arg) {
    printf("[Reader] Waiting for event...\n");
    
    uint64_t result;
    ssize_t s = read(efd, &result, sizeof(result));
    if (s != sizeof(result)) {
        perror("read");
        return NULL;
    }
    
    printf("[Reader] Event received! Value: %lu\n", result);
    return NULL;
}

int main(void) {
    printf("=== Basic eventfd Example ===\n\n");
    
    /* Create eventfd */
    efd = eventfd(0, 0);  /* Initial value 0, blocking mode */
    if (efd == -1) {
        perror("eventfd");
        exit(EXIT_FAILURE);
    }
    
    printf("Created eventfd: %d\n\n", efd);
    
    /* Create threads */
    pthread_t reader, writer;
    
    pthread_create(&reader, NULL, reader_thread, NULL);
    pthread_create(&writer, NULL, writer_thread, NULL);
    
    /* Wait for threads */
    pthread_join(reader, NULL);
    pthread_join(writer, NULL);
    
    /* Cleanup */
    close(efd);
    
    printf("\n=== How It Works ===\n");
    printf("1. eventfd created with counter = 0\n");
    printf("2. Reader calls read() - blocks (counter is 0)\n");
    printf("3. Writer does work, then write(1)\n");
    printf("4. Counter becomes 1, reader unblocks\n");
    printf("5. Reader gets value 1, counter resets to 0\n");
    
    printf("\n=== Key Points ===\n");
    printf("✅ Lightweight notification (single fd)\n");
    printf("✅ read() blocks until counter > 0\n");
    printf("✅ write() increments counter\n");
    printf("✅ Perfect for thread signaling\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT:
 * 
 * Created eventfd: 3
 * 
 * [Reader] Waiting for event...
 * [Writer] Starting work...
 * [Writer] Work done! Signaling event...
 * [Writer] Event signaled
 * [Reader] Event received! Value: 1
 * 
 * KEY CONCEPTS:
 * - eventfd is a file descriptor
 * - read() blocks until counter > 0
 * - write() adds to counter
 * - Very lightweight (just a counter)
 */
