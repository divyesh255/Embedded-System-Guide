# Semaphore Exercises

Practice resource management with proper error checking!

## Easy Exercises (20 minutes each)

### Exercise 1: Simple Resource Pool
**Task:** Create a pool of 2 resources accessed by 5 threads.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 5
#define POOL_SIZE 2

sem_t pool;

void *worker(void *arg) {
    int id = *(int *)arg;
    
    printf("[Thread %d] Waiting for resource\n", id);
    sem_wait(&pool);
    
    printf("[Thread %d] Got resource, working...\n", id);
    sleep(2);
    
    printf("[Thread %d] Done, releasing\n", id);
    sem_post(&pool);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    
    sem_init(&pool, 0, POOL_SIZE);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    sem_destroy(&pool);
    return 0;
}
```
</details>

### Exercise 2: Signal Between Threads
**Task:** Thread 1 waits for signal from Thread 2.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t signal_sem;

void *waiter(void *arg) {
    (void)arg;
    
    printf("[Waiter] Waiting for signal...\n");
    sem_wait(&signal_sem);
    
    printf("[Waiter] Received signal! Processing...\n");
    return NULL;
}

void *signaler(void *arg) {
    (void)arg;
    
    printf("[Signaler] Working for 2 seconds...\n");
    sleep(2);
    
    printf("[Signaler] Sending signal!\n");
    sem_post(&signal_sem);
    
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    sem_init(&signal_sem, 0, 0);  /* Start at 0 */
    
    pthread_create(&t1, NULL, waiter, NULL);
    pthread_create(&t2, NULL, signaler, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    sem_destroy(&signal_sem);
    return 0;
}
```
</details>

## Medium Exercises (30 minutes each)

### Exercise 3: Bounded Buffer (Semaphores Only)
**Task:** Implement bounded buffer with 1 producer, 1 consumer.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 3

int buffer[SIZE];
int in = 0, out = 0;

sem_t empty, full, mutex;

void *producer(void *arg) {
    (void)arg;
    
    for (int i = 0; i < 10; i++) {
        sem_wait(&empty);
        sem_wait(&mutex);
        
        buffer[in] = i;
        printf("Produced: %d\n", i);
        in = (in + 1) % SIZE;
        
        sem_post(&mutex);
        sem_post(&full);
    }
    
    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    
    for (int i = 0; i < 10; i++) {
        sem_wait(&full);
        sem_wait(&mutex);
        
        int item = buffer[out];
        printf("Consumed: %d\n", item);
        out = (out + 1) % SIZE;
        
        sem_post(&mutex);
        sem_post(&empty);
    }
    
    return NULL;
}

int main() {
    pthread_t prod, cons;
    
    sem_init(&empty, 0, SIZE);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    return 0;
}
```
</details>

### Exercise 4: Connection Pool
**Task:** Manage a pool of 3 database connections.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CONNECTIONS 3
#define NUM_CLIENTS 8

sem_t connection_pool;

void *client(void *arg) {
    int id = *(int *)arg;
    
    printf("[Client %d] Requesting connection...\n", id);
    sem_wait(&connection_pool);
    
    printf("[Client %d] Connected! Querying database...\n", id);
    sleep(1);
    
    printf("[Client %d] Disconnecting\n", id);
    sem_post(&connection_pool);
    
    return NULL;
}

int main() {
    pthread_t clients[NUM_CLIENTS];
    int ids[NUM_CLIENTS];
    
    printf("Database connection pool: %d connections\n\n", MAX_CONNECTIONS);
    
    sem_init(&connection_pool, 0, MAX_CONNECTIONS);
    
    for (int i = 0; i < NUM_CLIENTS; i++) {
        ids[i] = i + 1;
        pthread_create(&clients[i], NULL, client, &ids[i]);
        usleep(100000);
    }
    
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }
    
    sem_destroy(&connection_pool);
    return 0;
}
```
</details>

## Hard Exercise (45 minutes)

### Exercise 5: Reader-Writer with Semaphores
**Task:** Allow multiple readers OR one writer (no mutex allowed).

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int reader_count = 0;
sem_t resource;      /* Protects the resource */
sem_t reader_mutex;  /* Protects reader_count */

void *reader(void *arg) {
    int id = *(int *)arg;
    
    /* Entry section */
    sem_wait(&reader_mutex);
    reader_count++;
    if (reader_count == 1) {
        sem_wait(&resource);  /* First reader locks */
    }
    sem_post(&reader_mutex);
    
    /* Reading */
    printf("[Reader %d] Reading... (readers: %d)\n", id, reader_count);
    sleep(1);
    
    /* Exit section */
    sem_wait(&reader_mutex);
    reader_count--;
    if (reader_count == 0) {
        sem_post(&resource);  /* Last reader unlocks */
    }
    sem_post(&reader_mutex);
    
    return NULL;
}

void *writer(void *arg) {
    int id = *(int *)arg;
    
    printf("[Writer %d] Waiting to write...\n", id);
    sem_wait(&resource);
    
    printf("[Writer %d] Writing...\n", id);
    sleep(2);
    
    printf("[Writer %d] Done writing\n", id);
    sem_post(&resource);
    
    return NULL;
}

int main() {
    pthread_t readers[5], writers[2];
    int r_ids[5] = {1, 2, 3, 4, 5};
    int w_ids[2] = {1, 2};
    
    sem_init(&resource, 0, 1);
    sem_init(&reader_mutex, 0, 1);
    
    for (int i = 0; i < 5; i++) {
        pthread_create(&readers[i], NULL, reader, &r_ids[i]);
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_create(&writers[i], NULL, writer, &w_ids[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }
    
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }
    
    sem_destroy(&resource);
    sem_destroy(&reader_mutex);
    
    return 0;
}
```
</details>

---

**Key Takeaways:**
- ✅ Use correct initial value
- ✅ Always pair sem_wait with sem_post
- ✅ Check return values
- ✅ Destroy semaphores when done
- ✅ Document semaphore purpose

**Next Module:** 05_atomic_operations
