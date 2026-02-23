# Condition Variables - Efficient Thread Synchronization

**Study Time:** 45 minutes  
**Difficulty:** Intermediate  
**Prerequisites:** 01_threads, 02_mutex

## 📖 What is a Condition Variable?

A **condition variable** is a synchronization primitive that allows threads to wait efficiently for a condition to become true. Instead of busy-waiting (wasting CPU), threads sleep until signaled.

### Real-World Analogy

Imagine a restaurant:
- **Without Condition Variable** = Waiter constantly checks kitchen: "Is food ready? Is food ready?" (busy-waiting, wastes energy)
- **With Condition Variable** = Kitchen rings bell when food is ready, waiter sleeps until bell rings (efficient!)

The condition variable is the bell, and the waiter is your thread.

## 🤔 Why Do We Need Condition Variables?

### The Busy-Waiting Problem

```c
// BAD: Busy-waiting wastes CPU!
pthread_mutex_lock(&lock);
while (!data_ready) {
    pthread_mutex_unlock(&lock);
    // Spin! Wastes CPU cycles
    pthread_mutex_lock(&lock);
}
// Process data
pthread_mutex_unlock(&lock);
```

**Problems:**
- Wastes 100% CPU while waiting
- Slows down other threads
- Inefficient and battery-draining

### With Condition Variable - Efficient!

```c
// GOOD: Thread sleeps while waiting
pthread_mutex_lock(&lock);
while (!data_ready) {
    pthread_cond_wait(&cond, &lock);  // Sleep until signaled
}
// Process data
pthread_mutex_unlock(&lock);
```

**Benefits:**
- Thread sleeps (0% CPU)
- Other threads can run
- Wakes up instantly when signaled
- Efficient and battery-friendly

## 🛠️ Condition Variable API

### Core Functions

```c
#include <pthread.h>

/* Initialize condition variable */
int pthread_cond_init(pthread_cond_t *cond,
                      const pthread_condattr_t *attr);

/* Static initialization */
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* Wait for condition (releases lock, sleeps, reacquires lock) */
int pthread_cond_wait(pthread_cond_t *cond,
                      pthread_mutex_t *mutex);

/* Wait with timeout */
int pthread_cond_timedwait(pthread_cond_t *cond,
                           pthread_mutex_t *mutex,
                           const struct timespec *abstime);

/* Wake up ONE waiting thread */
int pthread_cond_signal(pthread_cond_t *cond);

/* Wake up ALL waiting threads */
int pthread_cond_broadcast(pthread_cond_t *cond);

/* Destroy condition variable */
int pthread_cond_destroy(pthread_cond_t *cond);
```

### Return Values
- **0** = Success
- **ETIMEDOUT** = Timeout expired (timedwait only)
- **EINVAL** = Invalid condition variable

## 💡 Key Concepts

### 1. Wait Pattern (ALWAYS use while loop!)

```c
pthread_mutex_lock(&mutex);

while (!condition) {  // MUST be while, not if!
    pthread_cond_wait(&cond, &mutex);
}

// Condition is now true, do work
process_data();

pthread_mutex_unlock(&mutex);
```

**Why while, not if?**
- Spurious wakeups can occur
- Multiple threads may wake up
- Condition might change before you run

### 2. Signal Pattern

```c
pthread_mutex_lock(&mutex);

// Make condition true
data_ready = 1;

pthread_cond_signal(&cond);  // Wake one waiter

pthread_mutex_unlock(&mutex);
```

### 3. Broadcast Pattern

```c
pthread_mutex_lock(&mutex);

// Make condition true for all
shutdown = 1;

pthread_cond_broadcast(&cond);  // Wake ALL waiters

pthread_mutex_unlock(&mutex);
```

### 4. How pthread_cond_wait() Works

```
1. Atomically: unlock mutex AND start waiting
2. Thread sleeps (0% CPU)
3. When signaled: wake up AND reacquire mutex
4. Return (mutex is locked again)
```

**Critical:** The unlock and wait are atomic!

## 🎯 Producer-Consumer Pattern

Classic use case for condition variables:

```c
// Shared data
int buffer[SIZE];
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

// Producer
void produce(int item) {
    pthread_mutex_lock(&mutex);
    
    while (count == SIZE) {  // Buffer full
        pthread_cond_wait(&not_full, &mutex);
    }
    
    buffer[count++] = item;
    pthread_cond_signal(&not_empty);  // Wake consumer
    
    pthread_mutex_unlock(&mutex);
}

// Consumer
int consume() {
    pthread_mutex_lock(&mutex);
    
    while (count == 0) {  // Buffer empty
        pthread_cond_wait(&not_empty, &mutex);
    }
    
    int item = buffer[--count];
    pthread_cond_signal(&not_full);  // Wake producer
    
    pthread_mutex_unlock(&mutex);
    return item;
}
```

## ⚠️ Common Pitfalls

### 1. **Using if Instead of while**

```c
// BAD - Spurious wakeup not handled!
pthread_mutex_lock(&mutex);
if (!ready) {  // WRONG!
    pthread_cond_wait(&cond, &mutex);
}
pthread_mutex_unlock(&mutex);

// GOOD - Handles spurious wakeups
pthread_mutex_lock(&mutex);
while (!ready) {  // CORRECT!
    pthread_cond_wait(&cond, &mutex);
}
pthread_mutex_unlock(&mutex);
```

### 2. **Signaling Without Lock**

```c
// BAD - Race condition!
data_ready = 1;
pthread_cond_signal(&cond);  // Not protected!

// GOOD - Signal under lock
pthread_mutex_lock(&mutex);
data_ready = 1;
pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);
```

### 3. **Wrong Mutex**

```c
// BAD - Different mutex!
pthread_mutex_lock(&mutex1);
pthread_cond_wait(&cond, &mutex2);  // WRONG!

// GOOD - Same mutex
pthread_mutex_lock(&mutex);
pthread_cond_wait(&cond, &mutex);
```

### 4. **Forgetting to Check Condition**

```c
// BAD - Assumes condition is true
pthread_cond_wait(&cond, &mutex);
process_data();  // Might not be ready!

// GOOD - Always check condition
while (!data_ready) {
    pthread_cond_wait(&cond, &mutex);
}
process_data();  // Definitely ready
```

### 5. **Deadlock with Broadcast**

```c
// BAD - All threads wake, one gets lock, others deadlock
pthread_cond_broadcast(&cond);

// GOOD - Use while loop to recheck
while (!condition) {
    pthread_cond_wait(&cond, &mutex);
}
```

## ✅ Best Practices

### 1. **Always Use while Loop**
```c
while (!condition) {
    pthread_cond_wait(&cond, &mutex);
}
```

### 2. **Hold Mutex When Signaling**
```c
pthread_mutex_lock(&mutex);
condition = true;
pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);
```

### 3. **Use Broadcast for Multiple Waiters**
```c
// Wake all threads when shutting down
pthread_cond_broadcast(&cond);
```

### 4. **Pair Each Condition with a Mutex**
```c
// One condition variable per condition
pthread_cond_t not_empty;  // with mutex
pthread_cond_t not_full;   // with same mutex
```

### 5. **Initialize Before Use**
```c
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// or
pthread_cond_init(&cond, NULL);
```

## 🎯 When to Use

### ✅ Use Condition Variables When:
- Thread needs to wait for a condition
- Producer-consumer pattern
- Thread pool waiting for work
- Barrier synchronization
- Event notification

### ❌ Don't Use When:
- Simple flag check (use atomic)
- No waiting needed (just mutex)
- Real-time constraints (unpredictable wakeup time)

## 📊 Signal vs Broadcast

### pthread_cond_signal()
- Wakes **one** waiting thread
- More efficient
- Use when any one thread can handle it

```c
// One item available, wake one consumer
pthread_cond_signal(&not_empty);
```

### pthread_cond_broadcast()
- Wakes **all** waiting threads
- Less efficient but necessary sometimes
- Use when all threads need to wake

```c
// Shutdown signal, wake everyone
pthread_cond_broadcast(&shutdown_cond);
```

## 🔍 Spurious Wakeups

**What are they?**
- Thread wakes up from pthread_cond_wait() even though no signal was sent
- Can happen due to OS implementation details
- POSIX allows this behavior

**How to handle:**
```c
// ALWAYS use while loop!
while (!condition) {
    pthread_cond_wait(&cond, &mutex);
}
// Now condition is definitely true
```

## 📚 Common Patterns

### 1. **Simple Flag**
```c
int ready = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Waiter
pthread_mutex_lock(&mutex);
while (!ready) {
    pthread_cond_wait(&cond, &mutex);
}
pthread_mutex_unlock(&mutex);

// Signaler
pthread_mutex_lock(&mutex);
ready = 1;
pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);
```

### 2. **Queue Not Empty**
```c
while (queue_empty()) {
    pthread_cond_wait(&not_empty, &mutex);
}
item = dequeue();
```

### 3. **Barrier (All Threads Wait)**
```c
pthread_mutex_lock(&mutex);
arrived++;
if (arrived < NUM_THREADS) {
    pthread_cond_wait(&barrier, &mutex);
} else {
    pthread_cond_broadcast(&barrier);
}
pthread_mutex_unlock(&mutex);
```

## 🚀 Ready to Code?

Now let's see condition variables in action!

**Next Steps:**
1. Run `01_busy_wait_bad.c` - See the problem
2. Run `02_condvar_good.c` - See the solution
3. Run `03_producer_consumer.c` - Classic pattern
4. Run `04_spurious_wakeup.c` - Handle edge cases
5. Complete `05_exercises.md` - Practice!

---

**Remember:** Condition variables = Efficient waiting!
