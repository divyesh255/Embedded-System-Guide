/**
 * 03_nonblocking.c - Non-blocking eventfd
 * 
 * Demonstrates EFD_NONBLOCK flag for non-blocking operations.
 * 
 * Compile: gcc -pthread 03_nonblocking.c -o 03_nonblocking
 * Run: ./03_nonblocking
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <pthread.h>

int efd;

void* producer(void* arg) {
    for (int i = 0; i < 5; i++) {
        sleep(1);
        
        uint64_t value = 1;
        write(efd, &value, sizeof(value));
        printf("[Producer] Sent event %d\n", i + 1);
    }
    return NULL;
}

int main(void) {
    printf("=== Non-blocking eventfd Example ===\n\n");
    
    /* Create non-blocking eventfd */
    efd = eventfd(0, EFD_NONBLOCK);
    if (efd == -1) {
        perror("eventfd");
        exit(EXIT_FAILURE);
    }
    
    printf("Created non-blocking eventfd\n\n");
    
    /* Start producer thread */
    pthread_t prod;
    pthread_create(&prod, NULL, producer, NULL);
    
    /* Consumer polls for events */
    printf("[Consumer] Polling for events...\n\n");
    
    int events_received = 0;
    while (events_received < 5) {
        uint64_t value;
        ssize_t s = read(efd, &value, sizeof(value));
        
        if (s == sizeof(value)) {
            printf("[Consumer] Received event! Value: %lu\n", value);
            events_received++;
        } else if (errno == EAGAIN) {
            printf("[Consumer] No events yet, doing other work...\n");
            usleep(500000);  /* 500ms */
        } else {
            perror("read");
            break;
        }
    }
    
    pthread_join(prod, NULL);
    close(efd);
    
    printf("\n=== Non-blocking Benefits ===\n");
    printf("✅ read() returns immediately (no blocking)\n");
    printf("✅ Returns EAGAIN if no events\n");
    printf("✅ Can do other work while waiting\n");
    printf("✅ Perfect for event loops\n");
    
    printf("\n=== Use Cases ===\n");
    printf("• Event-driven programs\n");
    printf("• Integration with poll/epoll\n");
    printf("• Async I/O frameworks\n");
    printf("• Real-time systems\n");
    
    return 0;
}

/*
 * BLOCKING vs NON-BLOCKING:
 * 
 * Blocking (default):
 *   read() → blocks until counter > 0
 *   Good for: Simple waiting
 * 
 * Non-blocking (EFD_NONBLOCK):
 *   read() → returns immediately
 *   Returns EAGAIN if counter == 0
 *   Good for: Event loops, polling
 * 
 * Choose based on your application needs!
 */
