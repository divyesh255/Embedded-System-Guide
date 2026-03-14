/**
 * 02_sigaction.c - Advanced Signal Handling with sigaction()
 * 
 * Demonstrates sigaction() for more control over signal handling.
 * 
 * Compile: gcc -Wall -Wextra 02_sigaction.c -o 02_sigaction
 * Run: ./02_sigaction
 * Test: Press Ctrl+C or send signals from another terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int signum, siginfo_t *info, void *context) {
    (void)context;  /* Unused */
    
    /* Get signal information */
    const char msg1[] = "\n=== SIGINT Received ===\n";
    const char msg2[] = "Signal number: ";
    const char msg3[] = "\nSent by PID: ";
    const char msg4[] = "\nShutting down...\n";
    
    write(STDOUT_FILENO, msg1, sizeof(msg1) - 1);
    write(STDOUT_FILENO, msg2, sizeof(msg2) - 1);
    
    /* Convert signal number to string (simple way) */
    char num[4];
    snprintf(num, sizeof(num), "%d", signum);
    write(STDOUT_FILENO, num, strlen(num));
    
    write(STDOUT_FILENO, msg3, sizeof(msg3) - 1);
    
    /* Convert PID to string */
    char pid[12];
    snprintf(pid, sizeof(pid), "%d", info->si_pid);
    write(STDOUT_FILENO, pid, strlen(pid));
    
    write(STDOUT_FILENO, msg4, sizeof(msg4) - 1);
    
    running = 0;
}

void sigusr1_handler(int signum, siginfo_t *info, void *context) {
    (void)signum;
    (void)context;
    
    const char msg[] = "\n[SIGUSR1] Custom signal received!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    
    /* Show sender info */
    const char msg2[] = "Sender PID: ";
    write(STDOUT_FILENO, msg2, sizeof(msg2) - 1);
    
    char pid[12];
    snprintf(pid, sizeof(pid), "%d", info->si_pid);
    write(STDOUT_FILENO, pid, strlen(pid));
    write(STDOUT_FILENO, "\n", 1);
}

int main(void) {
    printf("=== Advanced Signal Handling (sigaction) ===\n\n");
    printf("PID: %d\n\n", getpid());
    
    /* Setup SIGINT handler with sigaction */
    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_sigaction = sigint_handler;
    sa_int.sa_flags = SA_SIGINFO | SA_RESTART;  /* Get info, restart syscalls */
    sigemptyset(&sa_int.sa_mask);
    
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }
    
    /* Setup SIGUSR1 handler */
    struct sigaction sa_usr1;
    memset(&sa_usr1, 0, sizeof(sa_usr1));
    sa_usr1.sa_sigaction = sigusr1_handler;
    sa_usr1.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa_usr1.sa_mask);
    
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(EXIT_FAILURE);
    }
    
    printf("Signal handlers installed:\n");
    printf("  SIGINT  (Ctrl+C) - Graceful shutdown\n");
    printf("  SIGUSR1 - Custom signal\n\n");
    printf("Try: kill -USR1 %d\n", getpid());
    printf("Or:  Press Ctrl+C to exit\n\n");
    
    /* Main loop */
    int counter = 0;
    while (running) {
        printf("Working... %d\n", counter++);
        sleep(2);
    }
    
    printf("\n=== Cleanup Complete ===\n");
    printf("Exiting gracefully\n");
    
    return 0;
}

/*
 * SIGACTION vs SIGNAL:
 * 
 * sigaction() advantages:
 * 1. More portable across Unix systems
 * 2. SA_SIGINFO provides signal details
 * 3. SA_RESTART automatically restarts syscalls
 * 4. Can block other signals during handler
 * 5. More explicit and safer
 * 
 * SA_SIGINFO:
 * - Provides siginfo_t structure
 * - Contains sender PID, signal value, etc.
 * - Use sa_sigaction instead of sa_handler
 * 
 * SA_RESTART:
 * - Automatically restarts interrupted syscalls
 * - No need to check for EINTR
 * - Makes code cleaner
 * 
 * TRY IT:
 * 1. Run program, note the PID
 * 2. From another terminal: kill -USR1 <PID>
 * 3. See SIGUSR1 handler output
 * 4. Press Ctrl+C to see SIGINT handler
 */
