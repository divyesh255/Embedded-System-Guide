# Mutex - Mutual Exclusion Lock

**Study Time:** 30 minutes  
**Difficulty:** Beginner to Intermediate  
**Prerequisites:** 01_threads module

## 📖 What is a Mutex?

A **mutex** (mutual exclusion lock) is a synchronization primitive that ensures only one thread can access a shared resource at a time. Think of it as a lock on a door - only one person can hold the key and enter at a time.

### Real-World Analogy

Imagine a single bathroom in an office:
- **Without Mutex** = No lock on door → People walk in on each other (chaos!)
- **With Mutex** = Lock on door → One person at a time, others wait outside

The mutex is the lock, and the bathroom is your shared data.

## 🤔 Why Do We Need Mutexes?

### The Race Condition Problem

When multiple threads access shared data simultaneously without synchronization, you get **race conditions**:

```c
// DANGER: Race condition!
int counter = 0;

void *increment(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        counter++;  // NOT atomic! Three operations:
                    // 1. Read counter
                    // 2. Add 1
                    // 3. Write back
    }
    return NULL;
}

// Two threads running this = Lost updates!
// Expected: 2000000
// Actual: ~1200000 (varies each run)
```

### What Happens Without Mutex

```
Thread 1: Read counter (0)
Thread 2: Read counter (0)    ← Both read same value!
Thread 1: Add 1 (0 + 1 = 1)
Thread 2: Add 1 (0 + 1 = 1)   ← Both compute 1!
Thread 1: Write 1
Thread 2: Write 1              ← Lost one increment!
```

### With Mutex - Problem Solved

```c
pthread_mutex_t lock;

void *increment(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&lock);    // Acquire lock
        counter++;                     // Safe!
        pthread_mutex_unlock(&lock);   // Release lock
    }
    return NULL;
}

// Result: Always 2000000 ✓
```

## 🛠️ Mutex API

### Core Functions

```c
#include <pthread.h>

/* Initialize mutex */
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *attr);

/* Static initialization (simpler) */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Lock mutex (block if already locked) */
int pthread_mutex_lock(pthread_mutex_t *mutex);

/* Try to lock (non-blocking) */
int pthread_mutex_trylock(pthread_mutex_t *mutex);

/* Unlock mutex */
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* Destroy mutex */
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

### Return Values
- **0** = Success
- **EBUSY** = Already locked (trylock only)
- **EINVAL** = Invalid mutex
- **EDEADLK** = Would cause deadlock

## 💡 Key Concepts

### 1. Critical Section

Code that accesses shared data - must be protected:

```c
pthread_mutex_lock(&lock);
/* ← Critical section starts */
shared_data++;
shared_array[i] = value;
/* ← Critical section ends */
pthread_mutex_unlock(&lock);
```

### 2. Lock Granularity

**Fine-grained** (small critical sections):
```c
pthread_mutex_lock(&lock);
counter++;  // Just this
pthread_mutex_unlock(&lock);
```
✅ Better concurrency  
❌ More lock/unlock overhead

**Coarse-grained** (large critical sections):
```c
pthread_mutex_lock(&lock);
// Many operations
counter++;
process_data();
update_state();
pthread_mutex_unlock(&lock);
```
✅ Less overhead  
❌ Worse concurrency

### 3. Lock Ownership

- Only the thread that locked can unlock
- Mutex remembers who owns it
- Can't unlock from different thread

### 4. Blocking Behavior

```c
pthread_mutex_lock(&lock);  // Blocks until available
// vs
if (pthread_mutex_trylock(&lock) == 0) {
    // Got lock!
    pthread_mutex_unlock(&lock);
} else {
    // Lock busy, do something else
}
```

## ⚠️ Common Pitfalls

### 1. **Forgetting to Unlock**

```c
// BAD - Lock never released!
pthread_mutex_lock(&lock);
if (error) {
    return -1;  // Forgot to unlock!
}
pthread_mutex_unlock(&lock);

// GOOD - Always unlock
pthread_mutex_lock(&lock);
if (error) {
    pthread_mutex_unlock(&lock);
    return -1;
}
pthread_mutex_unlock(&lock);
```

### 2. **Deadlock**

```c
// BAD - Deadlock!
Thread 1:                    Thread 2:
pthread_mutex_lock(&lock1);  pthread_mutex_lock(&lock2);
pthread_mutex_lock(&lock2);  pthread_mutex_lock(&lock1);
// ← Both stuck waiting!

// GOOD - Same lock order
Thread 1:                    Thread 2:
pthread_mutex_lock(&lock1);  pthread_mutex_lock(&lock1);
pthread_mutex_lock(&lock2);  pthread_mutex_lock(&lock2);
```

### 3. **Locking Twice (Same Thread)**

```c
// BAD - Deadlock with self!
pthread_mutex_lock(&lock);
function_that_also_locks();  // Tries to lock again!
pthread_mutex_unlock(&lock);

// GOOD - Use recursive mutex or redesign
```

### 4. **Not Checking Return Values**

```c
// BAD
pthread_mutex_lock(&lock);

// GOOD
if (pthread_mutex_lock(&lock) != 0) {
    perror("mutex lock failed");
    return -1;
}
```

### 5. **Holding Lock Too Long**

```c
// BAD - Blocks other threads unnecessarily
pthread_mutex_lock(&lock);
expensive_computation();  // Doesn't need lock!
shared_data++;
pthread_mutex_unlock(&lock);

// GOOD - Minimize critical section
expensive_computation();  // Do this first
pthread_mutex_lock(&lock);
shared_data++;
pthread_mutex_unlock(&lock);
```

## ✅ Best Practices

### 1. **Keep Critical Sections Small**
```c
// Minimize time holding lock
pthread_mutex_lock(&lock);
// Only essential operations
pthread_mutex_unlock(&lock);
```

### 2. **Consistent Lock Ordering**
```c
// Always lock in same order to avoid deadlock
void transfer(Account *from, Account *to) {
    // Lock lower address first
    if (from < to) {
        pthread_mutex_lock(&from->lock);
        pthread_mutex_lock(&to->lock);
    } else {
        pthread_mutex_lock(&to->lock);
        pthread_mutex_lock(&from->lock);
    }
    // ... transfer ...
    pthread_mutex_unlock(&to->lock);
    pthread_mutex_unlock(&from->lock);
}
```

### 3. **Use RAII Pattern (in C++)**
```cpp
// C++ only - automatic unlock
std::lock_guard<std::mutex> guard(mutex);
// Automatically unlocks when scope exits
```

### 4. **Document Lock Requirements**
```c
/* Requires: lock must be held */
static void update_shared_data(void) {
    shared_counter++;
}
```

### 5. **Initialize Before Use**
```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// or
pthread_mutex_init(&lock, NULL);
```

## 🎯 When to Use Mutex

### ✅ Use Mutex When:
- Multiple threads access shared data
- Need to protect critical sections
- Operations are not atomic
- Need mutual exclusion guarantee

### ❌ Don't Use Mutex When:
- Single-threaded program
- Data is read-only
- Can use atomic operations instead
- Performance is critical and lock-free is possible

## 📊 Performance Considerations

### Mutex Overhead
- **Lock/Unlock**: ~20-100 nanoseconds (uncontended)
- **Contention**: Can be milliseconds if waiting
- **Context Switch**: ~1-10 microseconds

### Optimization Tips

1. **Reduce Lock Contention**
   ```c
   // Use multiple locks for different data
   pthread_mutex_t lock1;  // For data1
   pthread_mutex_t lock2;  // For data2
   ```

2. **Read-Write Locks**
   ```c
   // Multiple readers, single writer
   pthread_rwlock_t rwlock;
   pthread_rwlock_rdlock(&rwlock);  // Shared
   pthread_rwlock_wrlock(&rwlock);  // Exclusive
   ```

3. **Lock-Free Alternatives**
   ```c
   // For simple counters
   atomic_int counter;
   atomic_fetch_add(&counter, 1);
   ```

## 🔍 Debugging Mutex Issues

### Tools

1. **Helgrind** (Valgrind)
   ```bash
   valgrind --tool=helgrind ./program
   ```

2. **ThreadSanitizer** (GCC/Clang)
   ```bash
   gcc -fsanitize=thread -g program.c
   ```

3. **GDB**
   ```bash
   gdb ./program
   (gdb) info threads
   (gdb) thread 2
   (gdb) bt
   ```

### Common Symptoms

- **Deadlock**: Program hangs, threads waiting forever
- **Race Condition**: Inconsistent results, crashes
- **Priority Inversion**: High-priority thread blocked

## 📚 Mutex Types

### 1. Normal Mutex (Default)
```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```
- Fast
- No error checking
- Undefined behavior if misused

### 2. Error-Checking Mutex
```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
pthread_mutex_init(&lock, &attr);
```
- Detects errors (double lock, wrong thread unlock)
- Slower
- Good for debugging

### 3. Recursive Mutex
```c
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
```
- Same thread can lock multiple times
- Must unlock same number of times
- Use sparingly

## 🚀 Ready to Code?

Now that you understand mutex theory, let's see it in action!

**Next Steps:**
1. Run `01_race_condition.c` - See the problem
2. Run `02_mutex_solution.c` - See the fix
3. Run `03_deadlock.c` - Learn what NOT to do
4. Run `04_trylock.c` - Non-blocking locks
5. Complete `05_exercises.md` - Practice!

---

**Remember:** Mutex = Mutual Exclusion = One at a time!
