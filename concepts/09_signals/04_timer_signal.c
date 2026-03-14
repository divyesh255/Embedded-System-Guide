/**
 * 04_timer_signal.c - Timer with SIGALRM
 * 
 * Demonstrates using SIGALRM for periodic timers.
 * 
 * Compile: gcc -Wall -Wextra 04_timer_signal.c -o 04_timer_signal
 * Run: ./04_timer_signal
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

volatile sig_atomic_t timer_expired = 0;
volatile sig_atomic_t tick_count = 0;

void timer_handler(int signum) {
    (void)signum;
    
    timer_expired = 1;
    tick_count++;
    
    /* Reset alarm for next tick */
    alarm(2);  /* 2 seconds */
}

void print_time(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", buffer);
}

int main(void) {
    printf("=== Timer with SIGALRM ===\n\n");
    
    /* Setup SIGALRM handler */
    struct sigaction sa;
    sa.sa_handler = timer_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    
    printf("Timer started: 2-second intervals\n");
    printf("Will run for 10 ticks (20 seconds)\n\n");
    
    /* Start timer */
    alarm(2);  /* First alarm in 2 seconds */
    
    /* Main loop */
    while (tick_count < 10) {
        if (timer_expired) {
            timer_expired = 0;
            
            print_time();
            printf("Timer tick #%d\n", tick_count);
            
            /* Do periodic work here */
            if (tick_count % 3 == 0) {
                printf("  → Performing periodic maintenance\n");
            }
        }
        
        /* Do other work */
        usleep(100000);  /* 100ms */
    }
    
    printf("\n=== Timer Complete ===\n");
    printf("Total ticks: %d\n", tick_count);
    
    return 0;
}

/*
 * SIGALRM TIMER:
 * 
 * alarm(seconds):
 * - Schedules SIGALRM after N seconds
 * - Only one alarm at a time
 * - Calling alarm() again resets it
 * - alarm(0) cancels pending alarm
 * 
 * LIMITATIONS:
 * - Second resolution only
 * - Only one alarm per process
 * - Not very precise
 * 
 * BETTER ALTERNATIVES:
 * - timer_create() - POSIX timers (nanosecond precision)
 * - timerfd_create() - Timer as file descriptor
 * - setitimer() - Microsecond precision
 * 
 * USE CASES:
 * - Timeouts
 * - Periodic tasks
 * - Watchdog timers
 * - Rate limiting
 * 
 * EXAMPLE PATTERN:
 * 
 * void timeout_handler(int sig) {
 *     timeout_occurred = 1;
 * }
 * 
 * // Set 5-second timeout
 * alarm(5);
 * 
 * // Do work
 * while (!timeout_occurred && !done) {
 *     do_work();
 * }
 * 
 * // Cancel alarm if done early
 * alarm(0);
 */
