# Spinlocks

**Busy-wait synchronization for short critical sections**

---

## 🎯 What is a Spinlock?

A **spinlock** is a lock that causes a thread to **busy-wait** (spin) in a loop while repeatedly checking if the lock is available.

```c
// Thread keeps checking until lock is free
while (lock == LOCKED) {
    // Spin! (busy wait)
}
lock = LOCKED;
// Critical section
lock = UNLOCKED;
```

---

## 🤔 Spinlock vs Mutex

| Feature | Spinlock | Mutex |
|---------|----------|-------|
| **Waiting** | Busy-wait (spin) | Sleep (block) |
| **Context Switch** | No | Yes |
| **CPU Usage** | High (wastes CPU) | Low (sleeps) |
| **Overhead** | Very low | Higher |
| **Best For** | Very short locks | Long locks |
| **Use Case** | Kernel, ISR | User space |

---

## ⚡ When to Use Spinlocks

**Use spinlocks when:**
- ✅ Critical section is **very short** (< 100 instructions)
- ✅ Lock will be held for **less time than context switch**
- ✅ Running on **multicore** system
- ✅ In **kernel space** or **interrupt handlers**
- ✅ **Cannot sleep** (e.g., interrupt context)

**Use mutex when:**
- ❌ Critical section is long
- ❌ Single-core system
- ❌ Can afford to sleep
- ❌ User-space application

---

## 🔧 How Spinlocks Work

### Basic Spinlock (Naive)

```c
typedef int spinlock_t;

void spin_lock(spinlock_t *lock) {
    while (*lock == 1) {
        // Spin! Waste CPU cycles
    }
    *lock = 1;  // Acquire lock
}

void spin_unlock(spinlock_t *lock) {
    *lock = 0;  // Release lock
}
```

**Problem:** Race condition! Two threads can both see `*lock == 0` and both acquire the lock.

### Correct Spinlock (Atomic)

```c
#include <stdatomic.h>

typedef atomic_int spinlock_t;

void spin_lock(spinlock_t *lock) {
    int expected = 0;
    while (!atomic_compare_exchange_weak(lock, &expected, 1)) {
        expected = 0;  // Reset for next iteration
        // Spin!
    }
}

void spin_unlock(spinlock_t *lock) {
    atomic_store(lock, 0);
}
```

**How it works:**
1. Try to atomically swap 0 → 1
2. If successful, lock acquired
3. If failed, someone else has lock, keep spinning
4. Unlock by atomically setting to 0

---

## 🎨 Spinlock Variants

### 1. Test-and-Set Spinlock

```c
void spin_lock(spinlock_t *lock) {
    while (atomic_exchange(lock, 1) == 1) {
        // Spin until we successfully set it to 1
    }
}
```

### 2. Test-and-Test-and-Set (Better!)

```c
void spin_lock(spinlock_t *lock) {
    while (1) {
        // First test (read-only, no cache invalidation)
        while (atomic_load(lock) == 1) {
            // Spin without writing
        }
        
        // Then test-and-set
        if (atomic_exchange(lock, 1) == 0) {
            break;  // Got the lock!
        }
    }
}
```

**Why better?** Reduces cache coherence traffic on multicore systems.

### 3. Ticket Spinlock (Fair)

```c
typedef struct {
    atomic_int next_ticket;
    atomic_int now_serving;
} ticket_spinlock_t;

void ticket_lock(ticket_spinlock_t *lock) {
    int my_ticket = atomic_fetch_add(&lock->next_ticket, 1);
    while (atomic_load(&lock->now_serving) != my_ticket) {
        // Spin until it's my turn
    }
}

void ticket_unlock(ticket_spinlock_t *lock) {
    atomic_fetch_add(&lock->now_serving, 1);
}
```

**Advantage:** FIFO ordering, prevents starvation.

---

## ⚠️ Spinlock Pitfalls

### 1. Priority Inversion

```c
// Low priority thread holds spinlock
spin_lock(&lock);
// ... critical section ...

// High priority thread spins forever!
spin_lock(&lock);  // Can't preempt low priority thread
```

**Solution:** Disable preemption while holding spinlock (kernel only).

### 2. Deadlock

```c
// Thread 1
spin_lock(&lock_a);
spin_lock(&lock_b);  // Deadlock!

// Thread 2
spin_lock(&lock_b);
spin_lock(&lock_a);  // Deadlock!
```

**Solution:** Always acquire locks in same order.

### 3. Holding Too Long

```c
spin_lock(&lock);
expensive_computation();  // BAD! Wastes CPU
spin_unlock(&lock);
```

**Solution:** Keep critical section very short.

### 4. Single-Core Waste

```c
// On single-core system
spin_lock(&lock);  // Spins forever if lock holder can't run!
```

**Solution:** Use mutex on single-core systems.

---

## 🔬 Performance Comparison

**Scenario:** 1000 threads, 10,000 increments each

| Lock Type | Time | CPU Usage |
|-----------|------|-----------|
| No lock | 0.1s | 100% (race conditions!) |
| Spinlock | 0.5s | 100% (all cores busy) |
| Mutex | 1.2s | 20% (threads sleep) |

**Conclusion:** Spinlocks are faster but waste CPU. Use only for very short critical sections.

---

## 💡 Real-World Usage

### Linux Kernel

```c
spinlock_t my_lock = SPIN_LOCK_UNLOCKED;

spin_lock(&my_lock);
// Critical section (very short!)
spin_unlock(&my_lock);
```

### Interrupt Context

```c
spinlock_t irq_lock;

void interrupt_handler(void) {
    spin_lock(&irq_lock);
    // Handle interrupt (can't sleep here!)
    spin_unlock(&irq_lock);
}
```

### Multicore Data Structure

```c
typedef struct {
    spinlock_t lock;
    int data;
} shared_data_t;

void update_data(shared_data_t *sd, int value) {
    spin_lock(&sd->lock);
    sd->data = value;  // Very fast operation
    spin_unlock(&sd->lock);
}
```

---

## 🎓 Key Takeaways

1. **Spinlocks busy-wait** — waste CPU but avoid context switch
2. **Use for very short critical sections** — < 100 instructions
3. **Require atomic operations** — to avoid race conditions
4. **Best on multicore** — single-core wastes CPU
5. **Common in kernel** — where sleeping is not allowed
6. **Test-and-test-and-set** — reduces cache traffic
7. **Ticket spinlocks** — provide fairness

---

## 🚀 Next Steps

1. **01_naive_spinlock.c** — See the race condition
2. **02_atomic_spinlock.c** — Correct implementation
3. **03_test_and_test_and_set.c** — Optimized version
4. **04_ticket_spinlock.c** — Fair spinlock
5. **05_exercises.md** — Practice problems

---

**Ready to see spinlocks in action?** → `01_naive_spinlock.c`
