# Spinlocks - Practice Exercises

Test your understanding of spinlocks with these hands-on exercises!

---

## 🎯 Exercise 1: Implement Basic Spinlock (Easy - 15 min)

Implement a spinlock using `atomic_exchange`.

**Task:** Complete the implementation.

<details>
<summary>Solution</summary>

```c
#include <stdatomic.h>

typedef atomic_int spinlock_t;

void spin_lock(spinlock_t *lock) {
    /* Keep trying to exchange 0 → 1 */
    while (atomic_exchange(lock, 1) == 1) {
        /* Spin - lock is held by someone else */
    }
    /* Got the lock! */
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* Usage */
spinlock_t lock = 0;

spin_lock(&lock);
/* Critical section */
spin_unlock(&lock);
```

**Key points:**
- `atomic_exchange(lock, 1)` atomically sets lock to 1 and returns old value
- If old value was 1, someone else has lock, keep spinning
- If old value was 0, we got the lock!
</details>

---

## 🎯 Exercise 2: Spinlock vs Mutex Benchmark (Medium - 20 min)

Compare spinlock and mutex performance for different critical section lengths.

**Task:** Measure time for short (10 instructions) vs long (1000 instructions) critical sections.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define NUM_THREADS 4
#define ITERATIONS 100000

typedef atomic_int spinlock_t;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
spinlock_t spinlock = 0;

void spin_lock(spinlock_t *lock) {
    while (atomic_exchange(lock, 1) == 1);
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* Short critical section */
void short_work(void) {
    volatile int x = 0;
    for (int i = 0; i < 10; i++) x++;
}

/* Long critical section */
void long_work(void) {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) x++;
}

void* spinlock_thread(void* arg) {
    void (*work)(void) = arg;
    for (int i = 0; i < ITERATIONS; i++) {
        spin_lock(&spinlock);
        work();
        spin_unlock(&spinlock);
    }
    return NULL;
}

void* mutex_thread(void* arg) {
    void (*work)(void) = arg;
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        work();
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

double benchmark(void* (*func)(void*), void* arg) {
    pthread_t threads[NUM_THREADS];
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, func, arg);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    return (end.tv_sec - start.tv_sec) + 
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(void) {
    printf("=== Spinlock vs Mutex Benchmark ===\n\n");
    
    printf("Short critical section (10 instructions):\n");
    double spin_short = benchmark(spinlock_thread, short_work);
    double mutex_short = benchmark(mutex_thread, short_work);
    printf("  Spinlock: %.3f seconds\n", spin_short);
    printf("  Mutex:    %.3f seconds\n", mutex_short);
    printf("  Winner:   %s (%.2fx faster)\n\n", 
           spin_short < mutex_short ? "Spinlock" : "Mutex",
           spin_short < mutex_short ? mutex_short/spin_short : spin_short/mutex_short);
    
    printf("Long critical section (1000 instructions):\n");
    double spin_long = benchmark(spinlock_thread, long_work);
    double mutex_long = benchmark(mutex_thread, long_work);
    printf("  Spinlock: %.3f seconds\n", spin_long);
    printf("  Mutex:    %.3f seconds\n", mutex_long);
    printf("  Winner:   %s (%.2fx faster)\n", 
           spin_long < mutex_long ? "Spinlock" : "Mutex",
           spin_long < mutex_long ? mutex_long/spin_long : spin_long/mutex_long);
    
    return 0;
}
```

**Expected results:**
- Short CS: Spinlock wins (avoids context switch overhead)
- Long CS: Mutex wins (spinlock wastes CPU)

**Conclusion:** Use spinlocks only for very short critical sections!
</details>

---

## 🎯 Exercise 3: Implement Try-Lock (Medium - 20 min)

Implement `spin_trylock` that returns immediately if lock is held.

**Task:** Add non-blocking try-lock function.

<details>
<summary>Solution</summary>

```c
#include <stdatomic.h>
#include <stdbool.h>

typedef atomic_int spinlock_t;

bool spin_trylock(spinlock_t *lock) {
    /* Try once to acquire lock */
    int expected = 0;
    return atomic_compare_exchange_strong(lock, &expected, 1);
}

void spin_lock(spinlock_t *lock) {
    while (!spin_trylock(lock)) {
        /* Spin */
    }
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* Usage */
spinlock_t lock = 0;

if (spin_trylock(&lock)) {
    /* Got the lock! */
    /* Critical section */
    spin_unlock(&lock);
} else {
    /* Lock is held, do something else */
    printf("Lock busy, skipping\n");
}
```

**Use cases:**
- Avoid blocking when lock is contended
- Implement lock-free algorithms
- Timeout mechanisms
</details>

---

## 🎯 Exercise 4: Detect Deadlock (Hard - 25 min)

Create a scenario where two spinlocks cause deadlock.

**Task:** Demonstrate and fix the deadlock.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

typedef atomic_int spinlock_t;

spinlock_t lock_a = 0;
spinlock_t lock_b = 0;

void spin_lock(spinlock_t *lock) {
    while (atomic_exchange(lock, 1) == 1);
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* DEADLOCK: Thread 1 */
void* thread1_bad(void* arg) {
    spin_lock(&lock_a);
    printf("Thread 1: Got lock A\n");
    sleep(1);  /* Give thread 2 time to get lock B */
    
    printf("Thread 1: Waiting for lock B...\n");
    spin_lock(&lock_b);  /* DEADLOCK! */
    printf("Thread 1: Got lock B\n");
    
    spin_unlock(&lock_b);
    spin_unlock(&lock_a);
    return NULL;
}

/* DEADLOCK: Thread 2 */
void* thread2_bad(void* arg) {
    spin_lock(&lock_b);
    printf("Thread 2: Got lock B\n");
    sleep(1);  /* Give thread 1 time to get lock A */
    
    printf("Thread 2: Waiting for lock A...\n");
    spin_lock(&lock_a);  /* DEADLOCK! */
    printf("Thread 2: Got lock A\n");
    
    spin_unlock(&lock_a);
    spin_unlock(&lock_b);
    return NULL;
}

/* FIX: Always acquire locks in same order */
void* thread1_good(void* arg) {
    spin_lock(&lock_a);  /* A first */
    spin_lock(&lock_b);  /* B second */
    printf("Thread 1: Got both locks\n");
    spin_unlock(&lock_b);
    spin_unlock(&lock_a);
    return NULL;
}

void* thread2_good(void* arg) {
    spin_lock(&lock_a);  /* A first */
    spin_lock(&lock_b);  /* B second */
    printf("Thread 2: Got both locks\n");
    spin_unlock(&lock_b);
    spin_unlock(&lock_a);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    
    printf("=== Demonstrating Deadlock ===\n");
    printf("(Press Ctrl+C to stop)\n\n");
    
    /* Uncomment to see deadlock */
    // pthread_create(&t1, NULL, thread1_bad, NULL);
    // pthread_create(&t2, NULL, thread2_bad, NULL);
    
    /* Fixed version */
    pthread_create(&t1, NULL, thread1_good, NULL);
    pthread_create(&t2, NULL, thread2_good, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\n=== Deadlock Prevention ===\n");
    printf("✅ Always acquire locks in same order\n");
    printf("✅ Use trylock with timeout\n");
    printf("✅ Avoid holding multiple locks\n");
    
    return 0;
}
```

**Deadlock conditions:**
1. Thread 1 holds A, waits for B
2. Thread 2 holds B, waits for A
3. Neither can proceed!

**Fix:** Always acquire locks in same order (A then B).
</details>

---

## 🎯 Exercise 5: Adaptive Spinlock (Hard - 30 min)

Implement an adaptive spinlock that spins for a while, then yields.

**Task:** Combine spinning with yielding for better performance.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>

#define SPIN_LIMIT 1000

typedef atomic_int spinlock_t;

void adaptive_spin_lock(spinlock_t *lock) {
    int spin_count = 0;
    
    while (1) {
        /* Try to acquire lock */
        if (atomic_exchange(lock, 1) == 0) {
            return;  /* Got the lock! */
        }
        
        /* Spin for a while */
        if (spin_count < SPIN_LIMIT) {
            spin_count++;
            /* Busy wait */
        } else {
            /* Spun too long, yield CPU */
            sched_yield();
            spin_count = 0;  /* Reset counter */
        }
    }
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}

/* Test */
spinlock_t lock = 0;
int counter = 0;

void* worker(void* arg) {
    for (int i = 0; i < 100000; i++) {
        adaptive_spin_lock(&lock);
        counter++;
        spin_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    
    printf("=== Adaptive Spinlock ===\n\n");
    
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Counter: %d (expected: 400000)\n", counter);
    
    printf("\n=== How It Works ===\n");
    printf("1. Spin for %d iterations\n", SPIN_LIMIT);
    printf("2. If still locked, yield CPU\n");
    printf("3. Repeat until lock acquired\n");
    
    printf("\n=== Benefits ===\n");
    printf("✅ Fast for short waits (spinning)\n");
    printf("✅ Efficient for long waits (yielding)\n");
    printf("✅ Best of both worlds!\n");
    
    return 0;
}
```

**Strategy:**
- Spin for short time (fast if lock released quickly)
- Yield if spinning too long (don't waste CPU)
- Adaptive to lock contention level

**Used in:** Linux kernel, many production systems
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Spinlock Implementation**
   - Use atomic operations (compare-exchange, exchange)
   - Busy-wait in loop
   - Simple unlock (atomic store)

2. **When to Use**
   - Very short critical sections (< 100 instructions)
   - Multicore systems
   - Kernel/interrupt context
   - Cannot sleep

3. **Performance**
   - Faster than mutex for short CS
   - Wastes CPU for long CS
   - Test-and-test-and-set reduces cache traffic

4. **Fairness**
   - Basic spinlock: no fairness
   - Ticket spinlock: FIFO ordering
   - Prevents starvation

5. **Pitfalls**
   - Deadlock (multiple locks)
   - Priority inversion
   - Single-core waste
   - Holding too long

---

**Congratulations!** You've mastered spinlocks! 🎉

**Next:** Module 07 - Event Notification
