/**
 * 01_basic_thread.c - Your First Thread
 * 
 * This is the simplest possible thread example. It creates one thread
 * that prints a message, then waits for it to finish.
 * 
 * Compile: gcc -pthread -o 01_basic_thread 01_basic_thread.c
 * Run: ./01_basic_thread
 * 
 * Study time: 15 minutes
 * Difficulty: Beginner
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // for sleep()

/**
 * thread_function - The function that runs in the new thread
 * @arg: Argument passed to thread (unused here)
 * 
 * This function will execute in a separate thread of execution.
 * It runs concurrently with the main thread.
 * 
 * Return: NULL (required by pthread API)
 */
void *thread_function(void *arg)
{
    /* Suppress unused parameter warning */
    (void)arg;
    
    /* This code runs in the NEW thread */
    printf("Hello from the thread!\n");
    printf("Thread ID: %lu\n", pthread_self());
    
    /* Simulate some work */
    sleep(1);
    
    printf("Thread finishing...\n");
    
    /* Return NULL to indicate success */
    return NULL;
}

/**
 * main - Entry point
 * 
 * Creates a thread, waits for it to finish, then exits.
 */
int main(void)
{
    pthread_t thread;  /* Thread identifier */
    int result;
    
    printf("Main thread starting...\n");
    printf("Main thread ID: %lu\n", pthread_self());
    
    /* 
     * Create a new thread
     * 
     * pthread_create() parameters:
     * 1. &thread     - Where to store thread ID
     * 2. NULL        - Use default attributes
     * 3. thread_function - Function to run in thread
     * 4. NULL        - No argument to pass
     */
    result = pthread_create(&thread, NULL, thread_function, NULL);
    
    /* Always check for errors! */
    if (result != 0) {
        fprintf(stderr, "Error: pthread_create failed with code %d\n", result);
        return 1;
    }
    
    printf("Thread created successfully!\n");
    
    /*
     * Wait for the thread to finish
     * 
     * pthread_join() blocks until the thread terminates.
     * This is like wait() for processes.
     * 
     * Parameters:
     * 1. thread - Thread to wait for
     * 2. NULL   - Don't care about return value
     */
    result = pthread_join(thread, NULL);
    
    if (result != 0) {
        fprintf(stderr, "Error: pthread_join failed with code %d\n", result);
        return 1;
    }
    
    printf("Thread joined successfully!\n");
    printf("Main thread exiting...\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT:
 * ----------------
 * Main thread starting...
 * Main thread ID: 140123456789
 * Thread created successfully!
 * Hello from the thread!
 * Thread ID: 140123456790
 * Thread finishing...
 * Thread joined successfully!
 * Main thread exiting...
 * 
 * Note: Thread IDs will be different on your system
 * 
 * 
 * KEY CONCEPTS:
 * -------------
 * 1. pthread_t - Thread identifier (like PID for processes)
 * 2. pthread_create() - Creates and starts a new thread
 * 3. pthread_join() - Waits for thread to finish
 * 4. Thread function must return void* and take void* argument
 * 5. Always check return values!
 * 
 * 
 * TRY THIS:
 * ---------
 * 1. Comment out pthread_join() - What happens?
 *    (Thread may not finish before main exits!)
 * 
 * 2. Add more printf statements in both threads
 *    Notice they can interleave!
 * 
 * 3. Change sleep(1) to sleep(3)
 *    See how main waits longer
 * 
 * 4. Print pthread_self() in both threads
 *    Compare the thread IDs
 * 
 * 5. Create the thread in a loop (but still join it)
 *    See how thread IDs change
 * 
 * 
 * COMMON MISTAKES:
 * ----------------
 * ❌ Forgetting to link with -pthread
 * ❌ Not checking pthread_create() return value
 * ❌ Not calling pthread_join() (thread may not finish)
 * ❌ Using wrong function signature for thread function
 * 
 * 
 * NEXT STEP:
 * ----------
 * → 02_thread_args.c - Learn how to pass data to threads
 */
