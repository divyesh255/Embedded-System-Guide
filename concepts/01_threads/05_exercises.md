# Thread Exercises

Practice problems to reinforce your understanding of POSIX threads.
**Important:** All solutions follow best practices with proper error checking!

## Easy Exercises (15 minutes each)

### Exercise 1: Hello Threads
**Task:** Create 3 threads, each printing "Hello from thread X" where X is the thread number (1, 2, 3).

**Expected Output:**
```
Hello from thread 1
Hello from thread 2
Hello from thread 3
```

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_hello(void *arg) {
    int id = *(int *)arg;
    printf("Hello from thread %d\n", id);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int ids[3] = {1, 2, 3};
    int i;
    
    /* Create threads */
    for (i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, print_hello, &ids[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            return 1;
        }
    }
    
    /* Join threads */
    for (i = 0; i < 3; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
            return 1;
        }
    }
    
    return 0;
}
```
</details>

### Exercise 2: Sum Calculator
**Task:** Create a thread that calculates the sum of numbers from 1 to 1000 and returns the result.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *calculate_sum(void *arg) {
    int *result = malloc(sizeof(int));
    if (!result) {
        fprintf(stderr, "Error: malloc failed\n");
        return NULL;
    }
    
    *result = 0;
    for (int i = 1; i <= 1000; i++) {
        *result += i;
    }
    return (void *)result;
}

int main() {
    pthread_t thread;
    void *result;
    
    if (pthread_create(&thread, NULL, calculate_sum, NULL) != 0) {
        fprintf(stderr, "Error: Failed to create thread\n");
        return 1;
    }
    
    if (pthread_join(thread, &result) != 0) {
        fprintf(stderr, "Error: Failed to join thread\n");
        return 1;
    }
    
    if (result) {
        printf("Sum: %d\n", *(int *)result);
        free(result);
    }
    
    return 0;
}
```
</details>

### Exercise 3: Thread Array
**Task:** Create 10 threads using an array. Each thread should print its index.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *print_index(void *arg) {
    int index = *(int *)arg;
    printf("Thread %d\n", index);
    return NULL;
}

int main() {
    pthread_t threads[10];
    int *indices[10];
    int i;
    
    /* Create threads */
    for (i = 0; i < 10; i++) {
        indices[i] = malloc(sizeof(int));
        if (!indices[i]) {
            fprintf(stderr, "Error: malloc failed for index %d\n", i);
            return 1;
        }
        *indices[i] = i;
        
        if (pthread_create(&threads[i], NULL, print_index, indices[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            free(indices[i]);
            return 1;
        }
    }
    
    /* Join threads and cleanup */
    for (i = 0; i < 10; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
        }
        free(indices[i]);
    }
    
    return 0;
}
```
</details>

## Medium Exercises (30 minutes each)

### Exercise 4: Parallel Array Sum
**Task:** Create 4 threads to sum different quarters of an array of 1000 integers. Combine results in main.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ARRAY_SIZE 1000
#define NUM_THREADS 4

int array[ARRAY_SIZE];

struct thread_data {
    int start;
    int end;
};

void *partial_sum(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int *sum = malloc(sizeof(int));
    if (!sum) return NULL;
    
    *sum = 0;
    for (int i = data->start; i < data->end; i++) {
        *sum += array[i];
    }
    return sum;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct thread_data data[NUM_THREADS];
    int chunk = ARRAY_SIZE / NUM_THREADS;
    int total = 0;
    int i;
    
    /* Initialize array */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i + 1;
    }
    
    /* Create threads */
    for (i = 0; i < NUM_THREADS; i++) {
        data[i].start = i * chunk;
        data[i].end = (i + 1) * chunk;
        
        if (pthread_create(&threads[i], NULL, partial_sum, &data[i]) != 0) {
            fprintf(stderr, "Error: Failed to create thread %d\n", i);
            return 1;
        }
    }
    
    /* Collect results */
    for (i = 0; i < NUM_THREADS; i++) {
        void *result;
        
        if (pthread_join(threads[i], &result) != 0) {
            fprintf(stderr, "Error: Failed to join thread %d\n", i);
            continue;
        }
        
        if (result) {
            total += *(int *)result;
            free(result);
        }
    }
    
    printf("Total sum: %d\n", total);
    printf("Expected: %d\n", (ARRAY_SIZE * (ARRAY_SIZE + 1)) / 2);
    
    return 0;
}
```
</details>

### Exercise 5: Producer Thread
**Task:** Create a producer thread that generates 10 random numbers and stores them in a global array.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 10
int numbers[SIZE];

void *producer(void *arg) {
    int i;
    srand(time(NULL));
    
    for (i = 0; i < SIZE; i++) {
        numbers[i] = rand() % 100;
        printf("Produced: %d\n", numbers[i]);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int i;
    
    if (pthread_create(&thread, NULL, producer, NULL) != 0) {
        fprintf(stderr, "Error: Failed to create producer thread\n");
        return 1;
    }
    
    if (pthread_join(thread, NULL) != 0) {
        fprintf(stderr, "Error: Failed to join producer thread\n");
        return 1;
    }
    
    printf("\nFinal array: ");
    for (i = 0; i < SIZE; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
    
    return 0;
}
```
</details>

## Hard Exercise (45 minutes)

### Exercise 6: Simple Task Queue
**Task:** Create 3 worker threads that process 10 tasks. Use proper synchronization.

<details>
<summary>Solution (requires mutex - preview of next module)</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_WORKERS 3
#define NUM_TASKS 10

int next_task = 0;
pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;

void *worker(void *arg) {
    int id = *(int *)arg;
    
    while (1) {
        int task;
        
        pthread_mutex_lock(&task_mutex);
        if (next_task >= NUM_TASKS) {
            pthread_mutex_unlock(&task_mutex);
            break;
        }
        task = next_task++;
        pthread_mutex_unlock(&task_mutex);
        
        printf("[Worker %d] Processing task %d\n", id, task);
        sleep(1);
    }
    
    printf("[Worker %d] Finished\n", id);
    return NULL;
}

int main() {
    pthread_t workers[NUM_WORKERS];
    int ids[NUM_WORKERS];
    int i;
    
    for (i = 0; i < NUM_WORKERS; i++) {
        ids[i] = i + 1;
        if (pthread_create(&workers[i], NULL, worker, &ids[i]) != 0) {
            fprintf(stderr, "Error: Failed to create worker %d\n", i);
            return 1;
        }
    }
    
    for (i = 0; i < NUM_WORKERS; i++) {
        if (pthread_join(workers[i], NULL) != 0) {
            fprintf(stderr, "Error: Failed to join worker %d\n", i);
        }
    }
    
    printf("All tasks completed!\n");
    pthread_mutex_destroy(&task_mutex);
    
    return 0;
}
```
</details>

---

**Key Takeaways:**
- ✅ Always check pthread_create() return value
- ✅ Always check pthread_join() return value
- ✅ Always check malloc() return value
- ✅ Always free() allocated memory
- ✅ Handle errors gracefully

**Next Module:** 02_mutex - Learn proper synchronization!