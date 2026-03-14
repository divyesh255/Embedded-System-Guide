/**
 * 01_basic_signal.c - Basic Signal Handling
 * 
 * Demonstrates basic signal() function for handling SIGINT (Ctrl+C).
 * 
 * Compile: gcc -Wall -Wextra 01_basic_signal.c -o 01_basic_signal
 * Run: ./01_basic_signal
 * Test: Press Ctrl+C to send SIGINT
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;
volatile sig_atomic_t signal_count = 0;

void signal_handler(int signum) {
    (void)signum;  /* Unused */
    
    /* Only async-signal-safe operations! */
    signal_count++;
    
    if (signal_count >= 3) {
        running = 0;  /* Exit after 3 signals */
    }
    
    /* write() is async-signal-safe, printf() is NOT! */
    const char msg[] = "Caught SIGINT! Press Ctrl+C 2 more times to exit.\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

int main(void) {
    printf("=== Basic Signal Handling ===\n\n");
    printf("Press Ctrl+C to send SIGINT signal\n");
    printf("Program will exit after 3 SIGINT signals\n\n");
    
    /* Register signal handler */
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }
    
    printf("Running... (PID: %d)\n", getpid());
    
    /* Main loop */
    int counter = 0;
    while (running) {
        printf("Working... %d\n", counter++);
        sleep(1);
    }
    
    printf("\n=== Graceful Shutdown ===\n");
    printf("Received %d signals\n", signal_count);
    printf("Cleaning up...\n");
    printf("Goodbye!\n");
    
    return 0;
}

/*
 * KEY CONCEPTS:
 * 
 * 1. signal() registers a handler for a signal
 * 2. sig_atomic_t ensures atomic access
 * 3. volatile prevents compiler optimization
 * 4. Only async-signal-safe functions in handler
 * 5. write() is safe, printf() is NOT
 * 
 * SIGNAL SAFETY:
 * - Handler can be called at ANY time
 * - Must use async-signal-safe functions only
 * - Minimal work in handler (set flags)
 * - Process flags in main loop
 * 
 * TRY IT:
 * 1. Run the program
 * 2. Press Ctrl+C once - see message
 * 3. Press Ctrl+C twice more - program exits
 * 4. Notice graceful shutdown
 */
