# Atomic Operations - Lock-Free Programming

**Study Time:** 35 minutes  
**Difficulty:** Intermediate to Advanced  
**Prerequisites:** 01_threads, 02_mutex

## 📖 What are Atomic Operations?

**Atomic operations** are indivisible operations that complete without interruption. They're the foundation of lock-free programming - achieving thread safety without locks!

### Real-World Analogy

Imagine a vending machine:
- **Non-Atomic** = Insert coin → Press button → Get item (3 steps, can be interrupted)
- **Atomic** = Insert coin AND press button AND get item (1 indivisible action)

Atomic operations happen "all at once" - no thread can see them half-done.

## 🤔 Why Use Atomic Operations?

### The Lock Overhead Problem

```c
// With mutex - overhead!
pthread_mutex_lock(&lock);
counter++;
pthread_mutex_unlock(&lock);

// With atomic - no lock!
atomic_fetch_add(&counter, 1);  // Faster!
```

**Benefits:**
- ⚡ Much faster (no lock overhead)
- 🚫 No deadlock possible
- 📈 Better scalability
- 🔄 Lock-free algorithms

## 🎯 Atomic vs Mutex

| Feature | Mutex | Atomic |
|---------|-------|--------|
| Speed | Slower (~20-100ns) | Faster (~1-10ns) |
| Complexity | Simple to use | Harder to use correctly |
| Use case | Complex operations | Simple operations |
| Deadlock | Possible | Impossible |
| Scalability | Limited | Excellent |

### When to Use Each

**Use Mutex when:**
- Multiple operations need to be atomic together
- Complex data structures
- Need condition variables
- Easier to understand/maintain

**Use Atomic when:**
- Single variable operations
- Performance critical
- Lock-free algorithms
- Simple counters/flags

## 🛠️ C11 Atomic API

### Basic Types

```c
#include <stdatomic.h>

atomic_int counter;           // Atomic integer
atomic_bool flag;             // Atomic boolean
atomic_long value;            // Atomic long
atomic_ulong uvalue;          // Atomic unsigned long
_Atomic int custom;           // Generic atomic
```

### Core Operations

```c
/* Initialize */
atomic_int counter = ATOMIC_VAR_INIT(0);
// or
atomic_init(&counter, 0);

/* Load (read) */
int value = atomic_load(&counter);

/* Store (write) */
atomic_store(&counter, 42);

/* Add and return old value */
int old = atomic_fetch_add(&counter, 1);

/* Subtract and return old value */
int old = atomic_fetch_sub(&counter, 1);

/* Compare and swap */
int expected = 5;
bool success = atomic_compare_exchange_strong(&counter, &expected, 10);

/* Exchange (swap) */
int old = atomic_exchange(&counter, 100);
```

### Memory Orders

```c
// Sequentially consistent (default, safest)
atomic_load_explicit(&counter, memory_order_seq_cst);

// Acquire (for loads)
atomic_load_explicit(&counter, memory_order_acquire);

// Release (for stores)
atomic_store_explicit(&counter, 42, memory_order_release);

// Relaxed (no ordering, fastest)
atomic_load_explicit(&counter, memory_order_relaxed);
```

## 💡 Key Concepts

### 1. Atomicity

```c
// NOT atomic - 3 operations
counter++;  // Read, add, write

// Atomic - 1 operation
atomic_fetch_add(&counter, 1);
```

### 2. Compare-And-Swap (CAS)

```c
atomic_int value = ATOMIC_VAR_INIT(5);
int expected = 5;
int desired = 10;

// If value == expected, set to desired
if (atomic_compare_exchange_strong(&value, &expected, desired)) {
    printf("Swapped! Now %d\n", desired);
} else {
    printf("Failed! Value was %d, not %d\n", expected, 5);
}
```

### 3. Lock-Free Counter

```c
atomic_int counter = ATOMIC_VAR_INIT(0);

void increment() {
    atomic_fetch_add(&counter, 1);  // Thread-safe, no lock!
}

int get_value() {
    return atomic_load(&counter);
}
```

### 4. Lock-Free Flag

```c
atomic_bool ready = ATOMIC_VAR_INIT(false);

// Thread 1
atomic_store(&ready, true);

// Thread 2
while (!atomic_load(&ready)) {
    // Wait
}
```

## ⚠️ Common Pitfalls

### 1. **Using Non-Atomic Operations**

```c
// BAD - Not atomic!
atomic_int counter;
counter++;  // WRONG! Use atomic_fetch_add

// GOOD
atomic_fetch_add(&counter, 1);
```

### 2. **ABA Problem**

```c
// Thread 1 reads A
int old = atomic_load(&value);  // Reads A

// Thread 2 changes A → B → A
atomic_store(&value, B);
atomic_store(&value, A);

// Thread 1 thinks nothing changed!
atomic_compare_exchange_strong(&value, &old, C);  // Succeeds!
```

**Solution:** Use version numbers or tagged pointers.

### 3. **Memory Ordering Issues**

```c
// BAD - May reorder!
data = 42;
atomic_store(&ready, true, memory_order_relaxed);

// GOOD - Guarantees order
data = 42;
atomic_store(&ready, true, memory_order_release);
```

### 4. **Complex Operations**

```c
// BAD - Not atomic together!
atomic_fetch_add(&x, 1);
atomic_fetch_add(&y, 1);  // Another thread might see x updated but not y

// GOOD - Use mutex for multiple operations
pthread_mutex_lock(&lock);
x++;
y++;
pthread_mutex_unlock(&lock);
```

## ✅ Best Practices

### 1. **Start with Seq_Cst**
```c
// Use default (seq_cst) until you need performance
atomic_load(&counter);  // Safest
```

### 2. **Use Appropriate Type**
```c
atomic_int counter;      // For counters
atomic_bool flag;        // For flags
atomic_ulong pointer;    // For pointers (cast)
```

### 3. **Document Memory Order**
```c
// Document why you use relaxed
atomic_store(&counter, 0, memory_order_relaxed);  // OK: just statistics
```

### 4. **Test Thoroughly**
```c
// Atomic bugs are hard to reproduce
// Use ThreadSanitizer
gcc -fsanitize=thread program.c
```

### 5. **Keep It Simple**
```c
// If logic is complex, use mutex instead
// Atomic operations should be simple
```

## 🎯 Common Patterns

### 1. **Atomic Counter**
```c
atomic_int counter = ATOMIC_VAR_INIT(0);

void increment() {
    atomic_fetch_add(&counter, 1);
}

int get_count() {
    return atomic_load(&counter);
}
```

### 2. **Spin Lock**
```c
atomic_flag lock = ATOMIC_FLAG_INIT;

void spin_lock() {
    while (atomic_flag_test_and_set(&lock)) {
        // Spin
    }
}

void spin_unlock() {
    atomic_flag_clear(&lock);
}
```

### 3. **Once Flag**
```c
atomic_bool initialized = ATOMIC_VAR_INIT(false);

void init_once() {
    bool expected = false;
    if (atomic_compare_exchange_strong(&initialized, &expected, true)) {
        // Do initialization
    }
}
```

### 4. **Reference Counting**
```c
atomic_int refcount = ATOMIC_VAR_INIT(1);

void acquire() {
    atomic_fetch_add(&refcount, 1);
}

void release() {
    if (atomic_fetch_sub(&refcount, 1) == 1) {
        // Last reference, free resource
    }
}
```

## 📊 Memory Orders Explained

### Sequential Consistency (seq_cst)
- **Strongest** guarantee
- All threads see same order
- **Slowest** but safest
- **Use:** When in doubt

### Acquire-Release
- **Medium** guarantee
- Synchronizes specific operations
- **Faster** than seq_cst
- **Use:** Producer-consumer

### Relaxed
- **No** ordering guarantee
- **Fastest**
- **Use:** Simple counters, statistics

## 🔍 Debugging Tips

### Use ThreadSanitizer
```bash
gcc -fsanitize=thread -g program.c
./a.out
```

### Common Issues
- **Data races**: Use atomic operations
- **ABA problem**: Add version numbers
- **Memory ordering**: Start with seq_cst

## 🚀 Ready to Code?

Now let's see atomic operations in action!

**Next Steps:**
1. Run `01_atomic_counter.c` - Lock-free counter
2. Run `02_compare_and_swap.c` - CAS operation
3. Run `03_spinlock.c` - Lock-free spinlock
4. Run `04_reference_counting.c` - Practical example
5. Complete `05_exercises.md` - Practice!

---

**Remember:** Atomic = Indivisible = No interruption!
