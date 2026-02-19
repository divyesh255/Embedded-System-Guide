/**
 * 02_thread_args.c - Passing Arguments to Threads
 * 
 * This example shows how to pass data to threads and get return values.
 * Demonstrates proper argument passing and return value handling.
 * 
 * Compile: gcc -pthread -o 02_thread_args 02_thread_args.c
 * Run: ./02_thread_args
 * 
 * Study time: 15 minutes
 * Difficulty: Beginner
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/**
 * thread_data - Structure to pass multiple values to thread
 */
struct thread_data {
    int thread_id;
    char *message;
    int sleep_time;
};

/**
 * simple_thread - Thread that receives a single integer
 * @arg: Pointer to integer (cast from void*)
 * 
 * Demonstrates passing a single value to a thread.
 * 
 * Return: Pointer to result (allocated on heap)
 */
void *simple_thread(void *arg)
{
    /* Cast void* back to int* */
    int *num = (int *)arg;
    
    printf("[Thread %d] Received number: %d\n", *num, *num);
    printf("[Thread %d] Computing square...\n", *num);
    
    sleep(1);  /* Simulate work */
    
    /* Allocate result on heap (NOT stack!) */
    int *result = malloc(sizeof(int));
    *result = (*num) * (*num);
    
    printf("[Thread %d] Result: %d\n", *num, *result);
    
    /* Return result as void* */
    return (void *)result;
}

/**
 * complex_thread - Thread that receives a structure
 * @arg: Pointer to thread_data structure
 * 
 * Demonstrates passing multiple values via a structure.
 * 
 * Return: NULL
 */
void *complex_thread(void *arg)
{
    /* Cast void* to struct pointer */
    struct thread_data *data = (struct thread_data *)arg;
    
    printf("[Thread %d] Message: %s\n", data->thread_id, data->message);
    printf("[Thread %d] Sleeping for %d seconds...\n", 
           data->thread_id, data->sleep_time);
    
    sleep(data->sleep_time);
    
    printf("[Thread %d] Done!\n", data->thread_id);
    
    return NULL;
}

/**
 * main - Demonstrates different ways to pass arguments
 */
int main(void)
{
    pthread_t thread1, thread2, thread3;
    int result;
    void *thread_result;
    
    printf("=== Example 1: Passing Single Integer ===\n\n");
    
    /* 
     * CORRECT: Allocate on heap
     * This ensures the data persists after main() continues
     */
    int *num1 = malloc(sizeof(int));
    *num1 = 5;
    
    result = pthread_create(&thread1, NULL, simple_thread, num1);
    if (result != 0) {
        fprintf(stderr, "Error creating thread1\n");
        return 1;
    }
    
    /* Wait for thread and get return value */
    pthread_join(thread1, &thread_result);
    
    /* Cast return value back to int* */
    int *square = (int *)thread_result;
    printf("\nMain: Thread returned %d\n", *square);
    
    /* Clean up heap memory */
    free(num1);
    free(square);
    
    printf("\n=== Example 2: Passing Structure ===\n\n");
    
    /* Allocate structure on heap */
    struct thread_data *data1 = malloc(sizeof(struct thread_data));
    data1->thread_id = 1;
    data1->message = "Hello from thread 1";
    data1->sleep_time = 1;
    
    struct thread_data *data2 = malloc(sizeof(struct thread_data));
    data2->thread_id = 2;
    data2->message = "Hello from thread 2";
    data2->sleep_time = 2;
    
    /* Create two threads with different data */
    pthread_create(&thread2, NULL, complex_thread, data1);
    pthread_create(&thread3, NULL, complex_thread, data2);
    
    /* Wait for both threads */
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    
    /* Clean up */
    free(data1);
    free(data2);
    
    printf("\nAll threads completed!\n");
    
    return 0;
}

/*
 * EXPECTED OUTPUT:
 * ----------------
 * === Example 1: Passing Single Integer ===
 * 
 * [Thread 5] Received number: 5
 * [Thread 5] Computing square...
 * [Thread 5] Result: 25
 * 
 * Main: Thread returned 25
 * 
 * === Example 2: Passing Structure ===
 * 
 * [Thread 1] Message: Hello from thread 1
 * [Thread 1] Sleeping for 1 seconds...
 * [Thread 2] Message: Hello from thread 2
 * [Thread 2] Sleeping for 2 seconds...
 * [Thread 1] Done!
 * [Thread 2] Done!
 * 
 * All threads completed!
 * 
 * 
 * KEY CONCEPTS:
 * -------------
 * 1. Arguments must be passed as void* pointers
 * 2. Cast void* to appropriate type in thread function
 * 3. Use heap (malloc) for data that outlives function scope
 * 4. Return values also passed as void* pointers
 * 5. Always free() heap-allocated memory
 * 
 * 
 * COMMON MISTAKES:
 * ----------------
 * ❌ Passing address of stack variable:
 *    int num = 5;
 *    pthread_create(&t, NULL, func, &num);  // WRONG!
 *    // num may be destroyed before thread uses it
 * 
 * ❌ Forgetting to cast:
 *    void *thread_func(void *arg) {
 *        int num = arg;  // WRONG! Need to cast
 *    }
 * 
 * ❌ Not freeing heap memory:
 *    int *num = malloc(sizeof(int));
 *    // ... use num ...
 *    // Forgot to free(num) - memory leak!
 * 
 * ❌ Returning stack variable:
 *    void *thread_func(void *arg) {
 *        int result = 42;
 *        return &result;  // WRONG! Stack variable!
 *    }
 * 
 * 
 * BEST PRACTICES:
 * ---------------
 * ✅ Always allocate thread arguments on heap
 * ✅ Use structures for multiple arguments
 * ✅ Free memory after thread completes
 * ✅ Check malloc() return value
 * ✅ Document ownership of allocated memory
 * 
 * 
 * TRY THIS:
 * ---------
 * 1. Pass different numbers to simple_thread
 *    Try negative numbers, zero, large numbers
 * 
 * 2. Add more fields to thread_data structure
 *    Pass arrays, function pointers, etc.
 * 
 * 3. Create multiple threads in a loop
 *    Pass different data to each
 * 
 * 4. Try passing stack variable (see it fail!)
 *    int num = 5;
 *    pthread_create(&t, NULL, func, &num);
 * 
 * 5. Return different types from threads
 *    Strings, structures, arrays
 * 
 * 
 * MEMORY MANAGEMENT:
 * ------------------
 * Heap allocation is necessary because:
 * 1. Stack variables may be destroyed before thread uses them
 * 2. Thread may outlive the function that created it
 * 3. Multiple threads may need access to same data
 * 
 * Always remember:
 * - malloc() in main or before pthread_create()
 * - free() after pthread_join()
 * - One malloc() = One free()
 * 
 * 
 * NEXT STEP:
 * ----------
 * → 03_multiple_threads.c - Create and manage multiple threads
 */
