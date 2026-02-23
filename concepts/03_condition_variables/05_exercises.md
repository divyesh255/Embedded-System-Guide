# Condition Variable Exercises

Practice with proper error checking and while loops!

## Easy Exercises (20 minutes each)

### Exercise 1: Simple Wait/Signal
**Task:** Create a thread that waits for a signal, then processes data.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int data_ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *worker(void *arg) {
    (void)arg;
    
    pthread_mutex_lock(&mutex);
    while (!data_ready) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Worker: Processing data\n");
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main() {
    pthread_t t;
    
    pthread_create(&t, NULL, worker, NULL);
    
    sleep(2);
    
    pthread_mutex_lock(&mutex);
    data_ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    pthread_join(t, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
```
</details>

### Exercise 2: Broadcast to Multiple Waiters
**Task:** Wake up 3 waiting threads using broadcast.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

int go = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *waiter(void *arg) {
    int id = *(int *)arg;
    
    pthread_mutex_lock(&mutex);
    while (!go) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread %d: GO!\n", id);
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main() {
    pthread_t threads[3];
    int ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, waiter, &ids[i]);
    }
    
    sleep(1);
    
    pthread_mutex_lock(&mutex);
    go = 1;
    pthread_cond_broadcast(&cond);  /* Wake all! */
    pthread_mutex_unlock(&mutex);
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
```
</details>

## Medium Exercises (30 minutes each)

### Exercise 3: Bounded Buffer
**Task:** Implement a bounded buffer with 1 producer and 1 consumer.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

#define SIZE 3

int buffer[SIZE];
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    (void)arg;
    
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        
        while (count == SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }
        
        buffer[count++] = i;
        printf("Produced: %d\n", i);
        
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        int item = buffer[--count];
        printf("Consumed: %d\n", item);
        
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }
    
    return NULL;
}

int main() {
    pthread_t prod, cons;
    
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
    
    return 0;
}
```
</details>

### Exercise 4: Barrier Synchronization
**Task:** Create a barrier where all threads wait until everyone arrives.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4

int arrived = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barrier = PTHREAD_COND_INITIALIZER;

void *worker(void *arg) {
    int id = *(int *)arg;
    
    printf("Thread %d: Working...\n", id);
    sleep(id);  /* Simulate work */
    
    pthread_mutex_lock(&mutex);
    arrived++;
    printf("Thread %d: Arrived at barrier (%d/%d)\n", id, arrived, NUM_THREADS);
    
    if (arrived < NUM_THREADS) {
        pthread_cond_wait(&barrier, &mutex);
    } else {
        pthread_cond_broadcast(&barrier);
    }
    
    pthread_mutex_unlock(&mutex);
    
    printf("Thread %d: Passed barrier!\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS] = {1, 2, 3, 4};
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&barrier);
    
    return 0;
}
```
</details>

## Hard Exercise (45 minutes)

### Exercise 5: Thread Pool with Work Queue
**Task:** Implement a simple thread pool that processes tasks from a queue.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define POOL_SIZE 3
#define QUEUE_SIZE 10

typedef struct {
    int tasks[QUEUE_SIZE];
    int count;
    int shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
} WorkQueue;

WorkQueue queue = {
    .count = 0,
    .shutdown = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER
};

void *worker(void *arg) {
    int id = *(int *)arg;
    
    while (1) {
        pthread_mutex_lock(&queue.mutex);
        
        while (queue.count == 0 && !queue.shutdown) {
            pthread_cond_wait(&queue.not_empty, &queue.mutex);
        }
        
        if (queue.shutdown && queue.count == 0) {
            pthread_mutex_unlock(&queue.mutex);
            break;
        }
        
        int task = queue.tasks[--queue.count];
        pthread_mutex_unlock(&queue.mutex);
        
        printf("[Worker %d] Processing task %d\n", id, task);
        sleep(1);
    }
    
    printf("[Worker %d] Shutting down\n", id);
    return NULL;
}

void add_task(int task) {
    pthread_mutex_lock(&queue.mutex);
    queue.tasks[queue.count++] = task;
    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
}

int main() {
    pthread_t workers[POOL_SIZE];
    int ids[POOL_SIZE] = {1, 2, 3};
    
    for (int i = 0; i < POOL_SIZE; i++) {
        pthread_create(&workers[i], NULL, worker, &ids[i]);
    }
    
    for (int i = 0; i < 10; i++) {
        add_task(i);
    }
    
    sleep(5);
    
    pthread_mutex_lock(&queue.mutex);
    queue.shutdown = 1;
    pthread_cond_broadcast(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
    
    for (int i = 0; i < POOL_SIZE; i++) {
        pthread_join(workers[i], NULL);
    }
    
    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.not_empty);
    
    return 0;
}
```
</details>

---

**Key Takeaways:**
- ✅ Always use while loop with pthread_cond_wait
- ✅ Hold mutex when signaling
- ✅ Use broadcast for multiple waiters
- ✅ Check condition after wakeup

**Next Module:** 04_atomic_operations
