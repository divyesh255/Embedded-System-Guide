# Read-Write Locks - Practice Exercises

Test your understanding of rwlocks with these hands-on exercises!

---

## 🎯 Exercise 1: Implement Basic Rwlock Operations (Easy - 15 min)

Create a simple program using rwlock for shared counter.

**Task:** Multiple readers print counter, one writer increments it.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int counter = 0;

void* reader(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 5; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("[Reader %d] Counter = %d\n", id, counter);
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

void* writer(void* arg) {
    for (int i = 0; i < 5; i++) {
        sleep(2);
        pthread_rwlock_wrlock(&rwlock);
        counter++;
        printf("[Writer] Incremented to %d\n", counter);
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

int main(void) {
    pthread_t readers[3], writer_thread;
    int ids[3] = {0, 1, 2};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&readers[i], NULL, reader, &ids[i]);
    }
    pthread_create(&writer_thread, NULL, writer, NULL);
    
    for (int i = 0; i < 3; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer_thread, NULL);
    
    pthread_rwlock_destroy(&rwlock);
    return 0;
}
```

**Key points:**
- Multiple readers can read simultaneously
- Writer gets exclusive access
- Always destroy rwlock when done
</details>

---

## 🎯 Exercise 2: Benchmark Read-Heavy vs Write-Heavy (Medium - 25 min)

Compare rwlock vs mutex for different read/write ratios.

**Task:** Test 90% reads, 50% reads, 10% reads.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define ITERATIONS 100000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int data = 0;

double benchmark_mutex(int num_readers, int num_writers) {
    pthread_t threads[num_readers + num_writers];
    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Create reader threads
    for (int i = 0; i < num_readers; i++) {
        pthread_create(&threads[i], NULL, reader_mutex, NULL);
    }
    
    // Create writer threads
    for (int i = 0; i < num_writers; i++) {
        pthread_create(&threads[num_readers + i], NULL, writer_mutex, NULL);
    }
    
    // Wait for all
    for (int i = 0; i < num_readers + num_writers; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + 
           (end.tv_nsec - start.tv_nsec) / 1e9;
}

double benchmark_rwlock(int num_readers, int num_writers) {
    // Similar implementation with rwlock
    // ...
}

int main(void) {
    printf("=== Rwlock vs Mutex Benchmark ===\n\n");
    
    // 90% reads (9 readers, 1 writer)
    printf("90%% reads:\n");
    printf("  Mutex:  %.3fs\n", benchmark_mutex(9, 1));
    printf("  Rwlock: %.3fs\n", benchmark_rwlock(9, 1));
    
    // 50% reads (5 readers, 5 writers)
    printf("\n50%% reads:\n");
    printf("  Mutex:  %.3fs\n", benchmark_mutex(5, 5));
    printf("  Rwlock: %.3fs\n", benchmark_rwlock(5, 5));
    
    // 10% reads (1 reader, 9 writers)
    printf("\n10%% reads:\n");
    printf("  Mutex:  %.3fs\n", benchmark_mutex(1, 9));
    printf("  Rwlock: %.3fs\n", benchmark_rwlock(1, 9));
    
    return 0;
}
```

**Expected results:**
- 90% reads: Rwlock much faster
- 50% reads: Similar performance
- 10% reads: Mutex faster (rwlock overhead)

**Conclusion:** Use rwlock only for read-heavy workloads!
</details>

---

## 🎯 Exercise 3: Prevent Writer Starvation (Hard - 30 min)

Implement a fair rwlock that prevents writer starvation.

**Task:** Add writer priority to prevent starvation.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    int readers;
    int writers;
    int waiting_writers;  // Track waiting writers
} fair_rwlock_t;

void fair_rwlock_init(fair_rwlock_t *rw) {
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->read_cond, NULL);
    pthread_cond_init(&rw->write_cond, NULL);
    rw->readers = 0;
    rw->writers = 0;
    rw->waiting_writers = 0;
}

void fair_rdlock(fair_rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    
    // Wait if writer active OR writers waiting (priority!)
    while (rw->writers > 0 || rw->waiting_writers > 0) {
        pthread_cond_wait(&rw->read_cond, &rw->mutex);
    }
    
    rw->readers++;
    pthread_mutex_unlock(&rw->mutex);
}

void fair_wrlock(fair_rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    
    rw->waiting_writers++;  // Signal we're waiting
    
    while (rw->readers > 0 || rw->writers > 0) {
        pthread_cond_wait(&rw->write_cond, &rw->mutex);
    }
    
    rw->waiting_writers--;
    rw->writers = 1;
    pthread_mutex_unlock(&rw->mutex);
}

void fair_unlock(fair_rwlock_t *rw) {
    pthread_mutex_lock(&rw->mutex);
    
    if (rw->writers > 0) {
        rw->writers = 0;
        // Wake writers first (priority), then readers
        if (rw->waiting_writers > 0) {
            pthread_cond_signal(&rw->write_cond);
        } else {
            pthread_cond_broadcast(&rw->read_cond);
        }
    } else {
        rw->readers--;
        if (rw->readers == 0 && rw->waiting_writers > 0) {
            pthread_cond_signal(&rw->write_cond);
        }
    }
    
    pthread_mutex_unlock(&rw->mutex);
}
```

**Key improvements:**
- Track `waiting_writers`
- Readers wait if writers are waiting (priority)
- Writers get woken up first on unlock
- Prevents writer starvation!
</details>

---

## 🎯 Exercise 4: Upgrade Read Lock to Write Lock (Hard - 25 min)

Safely upgrade a read lock to write lock.

**Task:** Read data, decide if update needed, upgrade to write lock.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int data = 0;

// WRONG: Deadlock!
void upgrade_wrong(void) {
    pthread_rwlock_rdlock(&rwlock);
    
    if (data < 100) {
        // Try to upgrade - DEADLOCK!
        pthread_rwlock_wrlock(&rwlock);  // ❌ Deadlock!
        data++;
        pthread_rwlock_unlock(&rwlock);
    }
    
    pthread_rwlock_unlock(&rwlock);
}

// CORRECT: Release and reacquire
void upgrade_correct(void) {
    pthread_rwlock_rdlock(&rwlock);
    bool need_update = (data < 100);
    pthread_rwlock_unlock(&rwlock);  // Release read lock
    
    if (need_update) {
        pthread_rwlock_wrlock(&rwlock);  // Acquire write lock
        
        // Re-check condition (may have changed!)
        if (data < 100) {
            data++;
        }
        
        pthread_rwlock_unlock(&rwlock);
    }
}

// ALTERNATIVE: Use trylock
void upgrade_trylock(void) {
    pthread_rwlock_rdlock(&rwlock);
    
    if (data < 100) {
        pthread_rwlock_unlock(&rwlock);  // Release read
        
        // Try to get write lock
        if (pthread_rwlock_trywrlock(&rwlock) == 0) {
            if (data < 100) {  // Re-check
                data++;
            }
            pthread_rwlock_unlock(&rwlock);
        }
    } else {
        pthread_rwlock_unlock(&rwlock);
    }
}
```

**Key lessons:**
- ❌ Cannot upgrade read → write (deadlock!)
- ✅ Must release read, then acquire write
- ✅ Re-check condition after acquiring write lock
- ✅ Use trylock to avoid blocking
</details>

---

## 🎯 Exercise 5: Thread-Safe Cache with Rwlock (Hard - 35 min)

Implement a thread-safe LRU cache using rwlock.

**Task:** Cache with get (read) and put (write) operations.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define CACHE_SIZE 10

typedef struct cache_entry {
    char *key;
    int value;
    struct cache_entry *next;
    struct cache_entry *prev;
} cache_entry_t;

typedef struct {
    pthread_rwlock_t lock;
    cache_entry_t *head;  // MRU
    cache_entry_t *tail;  // LRU
    int size;
} lru_cache_t;

void cache_init(lru_cache_t *cache) {
    pthread_rwlock_init(&cache->lock, NULL);
    cache->head = NULL;
    cache->tail = NULL;
    cache->size = 0;
}

// Get (read operation)
int cache_get(lru_cache_t *cache, const char *key) {
    pthread_rwlock_rdlock(&cache->lock);
    
    cache_entry_t *entry = cache->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            int value = entry->value;
            pthread_rwlock_unlock(&cache->lock);
            return value;  // Found
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&cache->lock);
    return -1;  // Not found
}

// Put (write operation)
void cache_put(lru_cache_t *cache, const char *key, int value) {
    pthread_rwlock_wrlock(&cache->lock);
    
    // Check if key exists
    cache_entry_t *entry = cache->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;  // Update
            pthread_rwlock_unlock(&cache->lock);
            return;
        }
        entry = entry->next;
    }
    
    // Add new entry
    cache_entry_t *new_entry = malloc(sizeof(cache_entry_t));
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = cache->head;
    new_entry->prev = NULL;
    
    if (cache->head) {
        cache->head->prev = new_entry;
    }
    cache->head = new_entry;
    
    if (!cache->tail) {
        cache->tail = new_entry;
    }
    
    cache->size++;
    
    // Evict LRU if cache full
    if (cache->size > CACHE_SIZE) {
        cache_entry_t *lru = cache->tail;
        cache->tail = lru->prev;
        if (cache->tail) {
            cache->tail->next = NULL;
        }
        free(lru->key);
        free(lru);
        cache->size--;
    }
    
    pthread_rwlock_unlock(&cache->lock);
}
```

**Features:**
- Thread-safe get/put operations
- LRU eviction policy
- Rwlock for concurrent reads
- Exclusive writes for updates
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Rwlock API**
   - `pthread_rwlock_rdlock()` - shared read access
   - `pthread_rwlock_wrlock()` - exclusive write access
   - `pthread_rwlock_unlock()` - release lock

2. **When to Use**
   - Read-heavy workloads (90%+ reads)
   - Long read operations
   - Shared data structures

3. **Performance**
   - Faster than mutex for many readers
   - Slower than mutex for many writers
   - Overhead not worth it for write-heavy

4. **Pitfalls**
   - Writer starvation
   - Cannot upgrade read → write
   - Must re-check after upgrade
   - Always unlock in all paths

5. **Common Patterns**
   - Configuration cache
   - Lookup tables
   - Statistics counters
   - Session stores

---

**Congratulations!** You've mastered read-write locks! 🎉

