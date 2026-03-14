# Read-Write Locks (rwlock)

**Multiple readers OR single writer synchronization**

---

## 🎯 What is a Read-Write Lock?

A **read-write lock** (rwlock) allows:
- **Multiple readers** to access data simultaneously (shared access)
- **Single writer** with exclusive access (no readers or writers)

```c
// Multiple readers can read at the same time
reader1: read_lock()  ✓
reader2: read_lock()  ✓  (allowed!)
reader3: read_lock()  ✓  (allowed!)

// Writer needs exclusive access
writer: write_lock()  ✗  (blocked until all readers done)
```

---

## 🤔 Why Use Read-Write Locks?

**Problem:** Mutex allows only ONE thread at a time, even for reading!

```c
pthread_mutex_lock(&mutex);
int value = shared_data;  // Just reading!
pthread_mutex_unlock(&mutex);
```

**Inefficient:** Multiple readers could safely read simultaneously.

**Solution:** Read-write lock allows concurrent reads!

```c
pthread_rwlock_rdlock(&rwlock);
int value = shared_data;  // Multiple readers OK!
pthread_rwlock_unlock(&rwlock);
```

---

## 📊 Rwlock vs Mutex

| Feature | Mutex | Read-Write Lock |
|---------|-------|-----------------|
| **Readers** | One at a time | Multiple concurrent |
| **Writers** | One at a time | One at a time |
| **Read-heavy** | Slow | Fast |
| **Write-heavy** | Fast | Slower (overhead) |
| **Complexity** | Simple | More complex |
| **Use Case** | General | Read-heavy workloads |

---

## 🔧 POSIX Read-Write Lock API

### Initialization

```c
#include <pthread.h>

pthread_rwlock_t rwlock;

// Static initialization
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Dynamic initialization
pthread_rwlock_init(&rwlock, NULL);

// Cleanup
pthread_rwlock_destroy(&rwlock);
```

### Read Lock (Shared)

```c
// Acquire read lock (blocks if writer holds lock)
pthread_rwlock_rdlock(&rwlock);

// Read data (multiple readers allowed)
int value = shared_data;

// Release lock
pthread_rwlock_unlock(&rwlock);
```

### Write Lock (Exclusive)

```c
// Acquire write lock (blocks if any readers or writer)
pthread_rwlock_wrlock(&rwlock);

// Modify data (exclusive access)
shared_data = new_value;

// Release lock
pthread_rwlock_unlock(&rwlock);
```

### Try Lock (Non-blocking)

```c
// Try to acquire read lock
if (pthread_rwlock_tryrdlock(&rwlock) == 0) {
    // Got read lock
    pthread_rwlock_unlock(&rwlock);
}

// Try to acquire write lock
if (pthread_rwlock_trywrlock(&rwlock) == 0) {
    // Got write lock
    pthread_rwlock_unlock(&rwlock);
}
```

---

## 💡 When to Use Read-Write Locks

**Use rwlock when:**
- ✅ **Read-heavy workload** (90%+ reads)
- ✅ **Long read operations** (worth the overhead)
- ✅ **Shared data structure** (cache, config, lookup table)
- ✅ **Multiple reader threads**

**Use mutex when:**
- ❌ Write-heavy workload
- ❌ Short critical sections
- ❌ Simple locking needs
- ❌ Single reader/writer

---

## 📈 Performance Characteristics

**Read-heavy workload (90% reads):**
```
Mutex:   10 threads → 1x throughput
Rwlock:  10 threads → 9x throughput  (9 readers concurrent!)
```

**Write-heavy workload (90% writes):**
```
Mutex:   10 threads → 1x throughput
Rwlock:  10 threads → 0.8x throughput  (overhead!)
```

**Conclusion:** Rwlock shines with many readers, few writers.

---

## ⚠️ Common Pitfalls

### 1. Writer Starvation

```c
// Readers keep coming, writer never gets lock!
while (1) {
    pthread_rwlock_rdlock(&rwlock);  // Reader
    // ...
    pthread_rwlock_unlock(&rwlock);
}

// Writer starves!
pthread_rwlock_wrlock(&rwlock);  // Blocked forever!
```

**Solution:** Use writer-preferred rwlock or limit reader time.

### 2. Deadlock with Nested Locks

```c
pthread_rwlock_rdlock(&rwlock);
pthread_rwlock_wrlock(&rwlock);  // DEADLOCK!
```

**Solution:** Don't upgrade read lock to write lock. Release and reacquire.

### 3. Forgetting to Unlock

```c
pthread_rwlock_rdlock(&rwlock);
if (error) return;  // BUG: Forgot to unlock!
pthread_rwlock_unlock(&rwlock);
```

**Solution:** Always unlock in all code paths.

### 4. Using for Write-Heavy Workload

```c
// 90% writes - rwlock overhead not worth it!
pthread_rwlock_wrlock(&rwlock);  // Slower than mutex
```

**Solution:** Use mutex for write-heavy workloads.

---

## 🎨 Common Patterns

### 1. Configuration Cache

```c
typedef struct {
    pthread_rwlock_t lock;
    config_t data;
} config_cache_t;

// Many readers
void read_config(config_cache_t *cache) {
    pthread_rwlock_rdlock(&cache->lock);
    use_config(&cache->data);
    pthread_rwlock_unlock(&cache->lock);
}

// Rare writer
void update_config(config_cache_t *cache, config_t *new_config) {
    pthread_rwlock_wrlock(&cache->lock);
    cache->data = *new_config;
    pthread_rwlock_unlock(&cache->lock);
}
```

### 2. Lookup Table

```c
typedef struct {
    pthread_rwlock_t lock;
    hash_table_t table;
} lookup_cache_t;

// Frequent lookups
void* lookup(lookup_cache_t *cache, const char *key) {
    pthread_rwlock_rdlock(&cache->lock);
    void *value = hash_table_get(&cache->table, key);
    pthread_rwlock_unlock(&cache->lock);
    return value;
}

// Infrequent updates
void insert(lookup_cache_t *cache, const char *key, void *value) {
    pthread_rwlock_wrlock(&cache->lock);
    hash_table_put(&cache->table, key, value);
    pthread_rwlock_unlock(&cache->lock);
}
```

### 3. Statistics Counter

```c
typedef struct {
    pthread_rwlock_t lock;
    uint64_t requests;
    uint64_t errors;
} stats_t;

// Many readers (monitoring)
void get_stats(stats_t *stats, uint64_t *req, uint64_t *err) {
    pthread_rwlock_rdlock(&stats->lock);
    *req = stats->requests;
    *err = stats->errors;
    pthread_rwlock_unlock(&stats->lock);
}

// Occasional writer (update)
void increment_requests(stats_t *stats) {
    pthread_rwlock_wrlock(&stats->lock);
    stats->requests++;
    pthread_rwlock_unlock(&stats->lock);
}
```

---

## 🔬 Implementation Details

### How Rwlock Works Internally

```c
typedef struct {
    int readers;        // Number of active readers
    int writer;         // 1 if writer active, 0 otherwise
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
} rwlock_t;

void rdlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    while (rw->writer) {
        pthread_cond_wait(&rw->read_cond, &rw->mutex);
    }
    rw->readers++;
    pthread_mutex_unlock(&rw->mutex);
}

void wrlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    while (rw->readers > 0 || rw->writer) {
        pthread_cond_wait(&rw->write_cond, &rw->mutex);
    }
    rw->writer = 1;
    pthread_mutex_unlock(&rw->mutex);
}
```

---

## 🎓 Key Takeaways

1. **Multiple readers** can hold lock simultaneously
2. **Single writer** gets exclusive access
3. **Best for read-heavy** workloads (90%+ reads)
4. **Overhead** makes it slower than mutex for writes
5. **Writer starvation** is a real concern
6. **Don't upgrade** read lock to write lock (deadlock!)
7. **Always unlock** in all code paths

---

## 🚀 Next Steps

1. **01_mutex_vs_rwlock.c** - See the performance difference
2. **02_config_cache.c** - Configuration cache pattern
3. **03_writer_starvation.c** - Understand starvation
4. **04_lookup_table.c** - Hash table with rwlock
5. **05_exercises.md** - Practice problems

---

**Ready to see rwlocks in action?** → `01_mutex_vs_rwlock.c`
