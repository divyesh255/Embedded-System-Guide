# Memory Pool - Practice Exercises

Test your understanding of memory pools with these hands-on exercises!

---

## 🎯 Exercise 1: Calculate Pool Size (Easy - 10 min)

A system needs to allocate:
- 20 TCP connections (2KB each)
- 50 packet buffers (512 bytes each)
- 10 log entries (128 bytes each)

**Task:** Calculate total memory needed for pools.

<details>
<summary>Solution</summary>

```
TCP connections: 20 × 2048 bytes  = 40,960 bytes = 40 KB
Packet buffers:  50 × 512 bytes   = 25,600 bytes = 25 KB
Log entries:     10 × 128 bytes   = 1,280 bytes  = 1.25 KB
Total:                              67,840 bytes = 66.25 KB

With alignment (4-byte):
TCP:    2048 bytes (already aligned)
Packet: 512 bytes (already aligned)
Log:    128 bytes (already aligned)

Final: 66.25 KB
```

**Answer:** Need 67KB of RAM for memory pools.
</details>

---

## 🎯 Exercise 2: Detect Double-Free (Medium - 15 min)

Implement double-free detection using a magic number.

**Task:** Add magic number to detect if block is already free.

<details>
<summary>Solution</summary>

```c
#define MAGIC_FREE 0xDEADBEEF
#define MAGIC_USED 0xCAFEBABE

typedef struct block {
    uint32_t magic;
    struct block *next;
} block_t;

void* pool_alloc(pool_t *pool) {
    if (pool->free_list == NULL) return NULL;
    
    block_t *block = pool->free_list;
    
    /* Verify block is free */
    if (block->magic != MAGIC_FREE) {
        printf("ERROR: Corrupted free list!\n");
        return NULL;
    }
    
    pool->free_list = block->next;
    block->magic = MAGIC_USED;  /* Mark as used */
    pool->used_count++;
    
    return (void*)block;
}

void pool_free(pool_t *pool, void *ptr) {
    if (!ptr) return;
    
    block_t *block = (block_t*)ptr;
    
    /* Detect double-free */
    if (block->magic == MAGIC_FREE) {
        printf("ERROR: Double-free detected at %p!\n", ptr);
        return;
    }
    
    if (block->magic != MAGIC_USED) {
        printf("ERROR: Invalid block at %p (magic=0x%X)!\n", 
               ptr, block->magic);
        return;
    }
    
    block->magic = MAGIC_FREE;  /* Mark as free */
    block->next = pool->free_list;
    pool->free_list = block;
    pool->used_count--;
}
```

**Benefits:**
- Detects double-free immediately
- Detects memory corruption
- Minimal overhead (4 bytes per block)
</details>

---

## 🎯 Exercise 3: Implement Best-Fit Allocator (Medium - 20 min)

Create a multi-size pool allocator that chooses the smallest pool that fits.

**Task:** Implement `mem_alloc()` that picks best-fit pool.

<details>
<summary>Solution</summary>

```c
typedef struct {
    pool_t *pool;
    uint32_t size;
} pool_entry_t;

#define NUM_POOLS 4

static pool_entry_t pools[NUM_POOLS] = {
    { &tiny_pool,   32 },
    { &small_pool,  128 },
    { &medium_pool, 512 },
    { &large_pool,  2048 }
};

void* mem_alloc(uint32_t size) {
    /* Find smallest pool that fits */
    for (int i = 0; i < NUM_POOLS; i++) {
        if (size <= pools[i].size) {
            void *ptr = pool_alloc(pools[i].pool);
            if (ptr) {
                return ptr;
            }
            /* Pool exhausted, try next larger pool */
        }
    }
    
    return NULL;  /* No pool large enough */
}

void mem_free(void *ptr, uint32_t size) {
    if (!ptr) return;
    
    /* Find pool that owns this pointer */
    for (int i = 0; i < NUM_POOLS; i++) {
        if (size <= pools[i].size) {
            pool_free(pools[i].pool, ptr);
            return;
        }
    }
}
```

**Example:**
```
Request 50 bytes:
  - Try tiny (32) → too small
  - Try small (128) → fits! Use small pool
  
Request 100 bytes:
  - Try tiny (32) → too small
  - Try small (128) → fits! Use small pool
  
Request 300 bytes:
  - Try tiny (32) → too small
  - Try small (128) → too small
  - Try medium (512) → fits! Use medium pool
```

**Benefits:**
- Minimizes wasted space
- Automatic pool selection
- Fallback to larger pools if needed
</details>

---

## 🎯 Exercise 4: Add Pool Defragmentation (Hard - 25 min)

Implement pool compaction to move used blocks together.

**Task:** Compact pool to reduce fragmentation (for relocatable objects).

<details>
<summary>Solution</summary>

```c
typedef struct {
    void     *buffer;
    block_t  *free_list;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t used_count;
    uint8_t  *used_bitmap;  /* Track which blocks are used */
} pool_t;

void pool_init(pool_t *pool, void *buffer, uint32_t block_size, 
               uint32_t count, uint8_t *bitmap) {
    pool->buffer = buffer;
    pool->block_size = block_size;
    pool->block_count = count;
    pool->used_bitmap = bitmap;
    pool->used_count = 0;
    pool->free_list = NULL;
    
    /* Clear bitmap */
    memset(bitmap, 0, (count + 7) / 8);
    
    /* Build free list */
    for (uint32_t i = 0; i < count; i++) {
        block_t *block = (block_t*)((uint8_t*)buffer + (i * block_size));
        block->next = pool->free_list;
        pool->free_list = block;
    }
}

void* pool_alloc(pool_t *pool) {
    if (pool->free_list == NULL) return NULL;
    
    block_t *block = pool->free_list;
    pool->free_list = block->next;
    
    /* Mark block as used in bitmap */
    uint32_t idx = ((uint8_t*)block - (uint8_t*)pool->buffer) / pool->block_size;
    pool->used_bitmap[idx / 8] |= (1 << (idx % 8));
    
    pool->used_count++;
    return (void*)block;
}

void pool_compact(pool_t *pool, void **ptrs, uint32_t ptr_count) {
    uint32_t write_idx = 0;
    
    /* Move all used blocks to front */
    for (uint32_t read_idx = 0; read_idx < pool->block_count; read_idx++) {
        if (pool->used_bitmap[read_idx / 8] & (1 << (read_idx % 8))) {
            /* Block is used */
            if (read_idx != write_idx) {
                /* Move block */
                void *src = (uint8_t*)pool->buffer + (read_idx * pool->block_size);
                void *dst = (uint8_t*)pool->buffer + (write_idx * pool->block_size);
                memmove(dst, src, pool->block_size);
                
                /* Update pointer references */
                for (uint32_t i = 0; i < ptr_count; i++) {
                    if (ptrs[i] == src) {
                        ptrs[i] = dst;
                    }
                }
                
                /* Update bitmap */
                pool->used_bitmap[read_idx / 8] &= ~(1 << (read_idx % 8));
                pool->used_bitmap[write_idx / 8] |= (1 << (write_idx % 8));
            }
            write_idx++;
        }
    }
    
    /* Rebuild free list from compacted blocks */
    pool->free_list = NULL;
    for (uint32_t i = write_idx; i < pool->block_count; i++) {
        block_t *block = (block_t*)((uint8_t*)pool->buffer + (i * pool->block_size));
        block->next = pool->free_list;
        pool->free_list = block;
    }
}
```

**Note:** Compaction only works if you can update all pointers to moved blocks!
</details>

---

## 🎯 Exercise 5: Implement Object Pool (Hard - 30 min)

Create a type-safe object pool for TCP connections.

**Task:** Implement object pool with constructor/destructor.

<details>
<summary>Solution</summary>

```c
typedef struct {
    int socket_fd;
    uint32_t ip_addr;
    uint16_t port;
    uint8_t state;
    /* ... connection data ... */
} tcp_conn_t;

typedef struct {
    tcp_conn_t *objects;
    uint32_t   *free_indices;
    uint32_t   capacity;
    uint32_t   free_count;
} tcp_pool_t;

void tcp_pool_init(tcp_pool_t *pool, tcp_conn_t *buffer, 
                   uint32_t *indices, uint32_t capacity) {
    pool->objects = buffer;
    pool->free_indices = indices;
    pool->capacity = capacity;
    pool->free_count = capacity;
    
    /* Initialize free list */
    for (uint32_t i = 0; i < capacity; i++) {
        pool->free_indices[i] = i;
    }
}

tcp_conn_t* tcp_conn_alloc(tcp_pool_t *pool) {
    if (pool->free_count == 0) return NULL;
    
    /* Get free index */
    uint32_t idx = pool->free_indices[--pool->free_count];
    tcp_conn_t *conn = &pool->objects[idx];
    
    /* Constructor */
    memset(conn, 0, sizeof(tcp_conn_t));
    conn->socket_fd = -1;
    conn->state = 0;
    
    return conn;
}

void tcp_conn_free(tcp_pool_t *pool, tcp_conn_t *conn) {
    if (!conn) return;
    
    /* Destructor */
    if (conn->socket_fd >= 0) {
        close(conn->socket_fd);
    }
    
    /* Return to pool */
    uint32_t idx = conn - pool->objects;
    pool->free_indices[pool->free_count++] = idx;
}

/* Usage */
#define MAX_CONNECTIONS 50
static tcp_conn_t conn_buffer[MAX_CONNECTIONS];
static uint32_t conn_indices[MAX_CONNECTIONS];
static tcp_pool_t conn_pool;

int main(void) {
    tcp_pool_init(&conn_pool, conn_buffer, conn_indices, MAX_CONNECTIONS);
    
    tcp_conn_t *conn = tcp_conn_alloc(&conn_pool);
    if (conn) {
        conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        conn->ip_addr = 0x7F000001;  /* 127.0.0.1 */
        conn->port = 8080;
        
        /* Use connection... */
        
        tcp_conn_free(&conn_pool, conn);  /* Closes socket automatically */
    }
    
    return 0;
}
```

**Benefits:**
- Type-safe (no void* casts)
- Constructor/destructor support
- Resource cleanup automatic
- Fast allocation (array index)
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Pool Sizing**
   - Calculate based on worst-case usage
   - Account for alignment
   - Add safety margin

2. **Safety Features**
   - Double-free detection (magic numbers)
   - Bounds checking
   - Corruption detection

3. **Multi-Size Pools**
   - Best-fit allocation
   - Minimize wasted space
   - Fallback to larger pools

4. **Advanced Features**
   - Pool compaction (if relocatable)
   - Object pools (type-safe)
   - Constructor/destructor support

5. **Production Considerations**
   - Thread safety (add mutex)
   - Statistics tracking
   - Leak detection

---

**Congratulations!** You've mastered Memory Pools — the foundation of safe embedded memory management!
