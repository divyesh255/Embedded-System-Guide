# Semaphores - Resource Management

**Study Time:** 40 minutes  
**Difficulty:** Intermediate  
**Prerequisites:** 01_threads, 02_mutex

## 📖 What is a Semaphore?

A **semaphore** is a synchronization primitive that controls access to a shared resource through a counter. Think of it as a parking lot with a limited number of spaces.

### Real-World Analogy

Imagine a parking lot with 5 spaces:
- **Semaphore counter = 5** (5 spaces available)
- Car arrives → counter decrements (4 spaces left)
- Another car → counter decrements (3 spaces left)
- Car leaves → counter increments (4 spaces available)
- When counter = 0 → cars must wait

The semaphore is the parking attendant counting available spaces.

## 🤔 Why Do We Need Semaphores?

### The Resource Limit Problem

```c
// Problem: Only 3 database connections available
// But 10 threads want to connect!

// Without semaphore: Chaos!
for (int i = 0; i < 10; i++) {
    db_connect();  // Only 3 can succeed!
}

// With semaphore: Controlled access
sem_wait(&db_semaphore);  // Wait for available connection
db_connect();              // Guaranteed to succeed
// ... use connection ...
db_disconnect();
sem_post(&db_semaphore);  // Release connection
```

## 🎯 Semaphore vs Mutex

| Feature | Mutex | Semaphore |
|---------|-------|-----------|
| Purpose | Mutual exclusion | Resource counting |
| Initial value | 1 (unlocked) | N (resource count) |
| Ownership | Yes (thread that locks must unlock) | No (any thread can signal) |
| Use case | Protect critical section | Limit concurrent access |
| Example | One thread in section | N threads accessing pool |

### When to Use Each

**Use Mutex when:**
- Protecting shared data
- Need mutual exclusion
- Same thread locks and unlocks

**Use Semaphore when:**
- Limiting concurrent access
- Managing resource pool
- Producer-consumer with count
- Different threads signal/wait

## 🛠️ Semaphore API

### POSIX Semaphores

```c
#include <semaphore.h>

/* Initialize semaphore */
int sem_init(sem_t *sem, int pshared, unsigned int value);
// pshared: 0 = threads, 1 = processes
// value: initial count

/* Wait (decrement, P operation) */
int sem_wait(sem_t *sem);      // Block if count = 0

/* Try wait (non-blocking) */
int sem_trywait(sem_t *sem);   // Return error if count = 0

/* Timed wait */
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);

/* Post (increment, V operation) */
int sem_post(sem_t *sem);      // Increment count

/* Get current value */
int sem_getvalue(sem_t *sem, int *sval);

/* Destroy semaphore */
int sem_destroy(sem_t *sem);
```

### Return Values
- **0** = Success
- **-1** = Error (check errno)
- **EAGAIN** = Would block (trywait)
- **ETIMEDOUT** = Timeout (timedwait)

## 💡 Key Concepts

### 1. Binary Semaphore (Like Mutex)

```c
sem_t sem;
sem_init(&sem, 0, 1);  // Initial value = 1

// Thread 1
sem_wait(&sem);  // Decrement to 0
// Critical section
sem_post(&sem);  // Increment to 1

// Thread 2
sem_wait(&sem);  // Blocks if Thread 1 is in critical section
```

### 2. Counting Semaphore (Resource Pool)

```c
sem_t pool;
sem_init(&pool, 0, 3);  // 3 resources available

// Thread 1
sem_wait(&pool);  // Count: 3 → 2
use_resource();

// Thread 2
sem_wait(&pool);  // Count: 2 → 1
use_resource();

// Thread 3
sem_wait(&pool);  // Count: 1 → 0
use_resource();

// Thread 4
sem_wait(&pool);  // BLOCKS! Count = 0

// Thread 1 finishes
sem_post(&pool);  // Count: 0 → 1, Thread 4 wakes up!
```

### 3. Producer-Consumer with Semaphores

```c
#define SIZE 10

sem_t empty;  // Count of empty slots
sem_t full;   // Count of full slots
sem_t mutex;  // Protect buffer

sem_init(&empty, 0, SIZE);  // All slots empty
sem_init(&full, 0, 0);      // No slots full
sem_init(&mutex, 0, 1);     // Binary semaphore

// Producer
void produce(int item) {
    sem_wait(&empty);  // Wait for empty slot
    sem_wait(&mutex);  // Lock buffer
    
    buffer[in] = item;
    in = (in + 1) % SIZE;
    
    sem_post(&mutex);  // Unlock buffer
    sem_post(&full);   // Signal full slot
}

// Consumer
int consume() {
    sem_wait(&full);   // Wait for full slot
    sem_wait(&mutex);  // Lock buffer
    
    int item = buffer[out];
    out = (out + 1) % SIZE;
    
    sem_post(&mutex);  // Unlock buffer
    sem_post(&empty);  // Signal empty slot
    return item;
}
```

## ⚠️ Common Pitfalls

### 1. **Forgetting to Post**

```c
// BAD - Resource never released!
sem_wait(&sem);
if (error) {
    return -1;  // Forgot sem_post!
}
sem_post(&sem);

// GOOD - Always release
sem_wait(&sem);
if (error) {
    sem_post(&sem);
    return -1;
}
sem_post(&sem);
```

### 2. **Wrong Initial Value**

```c
// BAD - Wrong count!
sem_t pool;
sem_init(&pool, 0, 0);  // No resources available!

// GOOD - Correct count
sem_init(&pool, 0, 5);  // 5 resources available
```

### 3. **Deadlock with Multiple Semaphores**

```c
// BAD - Deadlock possible!
Thread 1:              Thread 2:
sem_wait(&sem1);       sem_wait(&sem2);
sem_wait(&sem2);       sem_wait(&sem1);

// GOOD - Same order
Thread 1:              Thread 2:
sem_wait(&sem1);       sem_wait(&sem1);
sem_wait(&sem2);       sem_wait(&sem2);
```

### 4. **Not Checking Return Values**

```c
// BAD
sem_wait(&sem);

// GOOD
if (sem_wait(&sem) != 0) {
    perror("sem_wait failed");
    return -1;
}
```

### 5. **Using After Destroy**

```c
// BAD
sem_destroy(&sem);
sem_wait(&sem);  // Undefined behavior!

// GOOD
sem_wait(&sem);
// ... done with semaphore ...
sem_destroy(&sem);
```

## ✅ Best Practices

### 1. **Initialize Before Use**
```c
sem_t sem;
if (sem_init(&sem, 0, 1) != 0) {
    perror("sem_init failed");
    return -1;
}
```

### 2. **Always Destroy**
```c
// Clean up when done
sem_destroy(&sem);
```

### 3. **Check Return Values**
```c
if (sem_wait(&sem) != 0) {
    perror("sem_wait");
    // Handle error
}
```

### 4. **Use Correct Initial Value**
```c
// Binary semaphore (like mutex)
sem_init(&sem, 0, 1);

// Counting semaphore (resource pool)
sem_init(&pool, 0, MAX_RESOURCES);

// Signaling (initially 0)
sem_init(&signal, 0, 0);
```

### 5. **Document Semaphore Purpose**
```c
sem_t db_connections;  // Limits to 5 concurrent DB connections
sem_init(&db_connections, 0, 5);
```

## 🎯 Common Patterns

### 1. **Resource Pool**
```c
sem_t pool;
sem_init(&pool, 0, MAX_CONNECTIONS);

// Acquire resource
sem_wait(&pool);
Resource *r = get_resource();
// Use resource
release_resource(r);
sem_post(&pool);
```

### 2. **Signaling Between Threads**
```c
sem_t signal;
sem_init(&signal, 0, 0);  // Start at 0

// Thread 1 (waiter)
sem_wait(&signal);  // Blocks until signaled
process_data();

// Thread 2 (signaler)
prepare_data();
sem_post(&signal);  // Wake up Thread 1
```

### 3. **Barrier (N threads wait)**
```c
sem_t barrier;
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void barrier_wait(int n) {
    pthread_mutex_lock(&mutex);
    count++;
    if (count == n) {
        for (int i = 0; i < n; i++) {
            sem_post(&barrier);
        }
    }
    pthread_mutex_unlock(&mutex);
    
    sem_wait(&barrier);
}
```

### 4. **Rate Limiting**
```c
sem_t rate_limiter;
sem_init(&rate_limiter, 0, REQUESTS_PER_SECOND);

void make_request() {
    sem_wait(&rate_limiter);
    // Make API request
    
    // Refill after 1 second
    sleep(1);
    sem_post(&rate_limiter);
}
```

## 📊 Semaphore Types

### 1. **Binary Semaphore**
- Initial value: 0 or 1
- Acts like a mutex
- Used for mutual exclusion

### 2. **Counting Semaphore**
- Initial value: N (resource count)
- Tracks available resources
- Used for resource pools

### 3. **Signaling Semaphore**
- Initial value: 0
- Used for thread signaling
- One thread waits, another signals

## 🔍 Debugging Tips

### Check Current Value
```c
int value;
sem_getvalue(&sem, &value);
printf("Semaphore value: %d\n", value);
```

### Common Issues
- **Hangs**: Forgot to post, or wrong initial value
- **Too many accesses**: Initial value too high
- **Deadlock**: Circular wait on multiple semaphores

### Tools
```bash
# Detect deadlocks
valgrind --tool=helgrind ./program

# Check for leaks
valgrind --leak-check=full ./program
```

## 🚀 Ready to Code?

Now let's see semaphores in action!

**Next Steps:**
1. Run `01_binary_semaphore.c` - Like a mutex
2. Run `02_counting_semaphore.c` - Resource pool
3. Run `03_producer_consumer.c` - Classic pattern
4. Run `04_rate_limiter.c` - Practical example
5. Complete `05_exercises.md` - Practice!

---

**Remember:** Semaphores = Resource counting!
