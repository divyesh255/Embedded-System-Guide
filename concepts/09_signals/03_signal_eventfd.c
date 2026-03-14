/**
 * 03_signal_eventfd.c - Thread-Safe Signal Handling with eventfd
 * 
 * Demonstrates the BEST PRACTICE for signal handling in multi-threaded programs.
 * Signal handler writes to eventfd, main loop reads from it.
 * 
 * Compile: gcc -Wall -Wextra -pthread 03_signal_eventfd.c -o 03_signal_eventfd
 * Run: ./03_signal_eventfd
 * Test: Press Ctrl+C
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <pthread.h>

int signal_efd;  /* eventfd for signal notification */
volatile sig_atomic_t running = 1;

void signal_handler(int signum) {
    /* ASYNC-SIGNAL-SAFE! */
    uint64_t val = signum;
    write(signal_efd, &val, sizeof(val));  /* write() is async-signal-safe */
}

void* worker_thread(void* arg) {
    int id = *(int*)arg;
    
    printf("[Worker %d] Started\n", id);
    
    while (running) {
        printf("[Worker %d] Working...\n", id);
        sleep(2);
    }
    
    printf("[Worker %d] Shutting down\n", id);
    return NULL;
}

int main(void) {
    printf("=== Thread-Safe Signal Handling with eventfd ===\n\n");
    
    /* Create eventfd for signal notification */
    signal_efd = eventfd(0, EFD_NONBLOCK);
    if (signal_efd == -1) {
        perror("eventfd");
        exit(EXIT_FAILURE);
    }
    
    /* Setup signal handlers */
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    printf("Signal handlers installed (SIGINT, SIGTERM)\n");
    printf("Press Ctrl+C to trigger graceful shutdown\n\n");
    
    /* Start worker threads */
    pthread_t workers[3];
    int ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&workers[i], NULL, worker_thread, &ids[i]);
    }
    
    /* Main event loop with poll() */
    struct pollfd pfd = {
        .fd = signal_efd,
        .events = POLLIN
    };
    
    printf("[Main] Event loop started\n\n");
    
    while (running) {
        /* Wait for signal with timeout */
        int ret = poll(&pfd, 1, 1000);  /* 1 second timeout */
        
        if (ret > 0) {
            /* Signal received! */
            uint64_t signum;
            read(signal_efd, &signum, sizeof(signum));
            
            printf("\n[Main] Signal %lu received via eventfd\n", signum);
            printf("[Main] Initiating graceful shutdown...\n\n");
            
            running = 0;  /* Stop all workers */
        } else if (ret == 0) {
            /* Timeout - do periodic work */
            printf("[Main] Heartbeat...\n");
        }
    }
    
    /* Wait for workers to finish */
    printf("[Main] Waiting for workers to finish...\n");
    for (int i = 0; i < 3; i++) {
        pthread_join(workers[i], NULL);
    }
    
    close(signal_efd);
    
    printf("\n=== Shutdown Complete ===\n");
    printf("All threads terminated gracefully\n");
    
    return 0;
}

/*
 * WHY THIS IS THE BEST PRACTICE:
 * 
 * 1. ASYNC-SIGNAL-SAFE:
 *    - Signal handler only calls write()
 *    - write() is async-signal-safe
 *    - No race conditions!
 * 
 * 2. THREAD-SAFE:
 *    - eventfd works across threads
 *    - No need for mutexes in signal handler
 *    - Clean separation of concerns
 * 
 * 3. INTEGRATES WITH EVENT LOOPS:
 *    - Use poll/epoll/select
 *    - Can monitor multiple fds
 *    - Perfect for async I/O frameworks
 * 
 * 4. CLEAN SHUTDOWN:
 *    - Signal processed in main loop
 *    - Can do complex cleanup safely
 *    - All threads notified properly
 * 
 * PATTERN:
 * Signal → Handler → eventfd → poll() → Main Loop → Process
 * 
 * This is how production systems handle signals!
 */
