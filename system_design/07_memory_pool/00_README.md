# Memory Pool

**The pattern for safe, deterministic dynamic memory allocation**

---

## 🎯 What Problem Does This Solve?

Embedded systems need dynamic memory, but `malloc()` is dangerous:

```c
// WRONG: malloc() in embedded systems
void* ptr = malloc(1024);  // ❌ Can fail unpredictably
                           // ❌ Causes fragmentation
                           // ❌ Non-deterministic timing
                           // ❌ No memory left for critical tasks
```

**The naive solution — using malloc/free — breaks in production:**
- Memory fragmentation after hours of operation
- Out-of-memory failures at random times
- Unpredictable allocation times (not real-time safe)
- Memory leaks from forgotten `free()`

**The solution: Memory Pool**

Pre-allocate fixed-size blocks at startup → allocate/free from pool at runtime.

---

## 🔧 How It Works

### Fixed-Size Blocks

Memory pool divides a large buffer into equal-sized blocks:

```
Memory Pool (1024 bytes, 16 blocks of 64 bytes each):
┌────┬────┬────┬────┬────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │ 4  │ 5  │ 6  │ 7  │  Free blocks
├────┼────┼────┼────┼────┼────┼────┼────┤
│ 8  │ 9  │ 10 │ 11 │ 12 │ 13 │ 14 │ 15 │
└────┴────┴────┴────┴────┴────┴────┴────┘
```

### Free List

Available blocks linked together:

```
Free list: 0 → 1 → 2 → 3 → ... → 15 → NULL

Allocate:  Remove from head
Free:      Add back to head
```

### Allocation

```c
void* pool_alloc(pool_t *pool) {
    if (pool->free_list == NULL) {
        return NULL;  // Pool exhausted
    }
    
    block_t *block = pool->free_list;
    pool->free_list = block->next;  // Remove from free list
    pool->used_count++;
    
    return (void*)block;
}
```

### Deallocation

```c
void pool_free(pool_t *pool, void *ptr) {
    block_t *block = (block_t*)ptr;
    block->next = pool->free_list;  // Add to head of free list
    pool->free_list = block;
    pool->used_count--;
}
```

---

## 📐 Pool Types

### Single-Size Pool
All blocks same size. Used for: packets, messages, buffers.

```
Block size: 64 bytes
Count: 16 blocks
Total: 1024 bytes
```

### Multi-Size Pools
Multiple pools for different sizes. Used for: general allocation.

```
Small pool:  32 bytes × 32 blocks = 1024 bytes
Medium pool: 128 bytes × 16 blocks = 2048 bytes
Large pool:  512 bytes × 8 blocks = 4096 bytes
```

### Object Pool
Pool of specific objects (e.g., TCP connections). Used for: resource management.

```c
typedef struct {
    int socket_fd;
    uint32_t ip_addr;
    uint16_t port;
    // ... connection state
} tcp_conn_t;

tcp_conn_t conn_pool[MAX_CONNECTIONS];
```

---

## ⚙️ Key Design Decisions

### 1. Block Size
```c
#define BLOCK_SIZE  64   // Must fit largest allocation
#define BLOCK_COUNT 16   // Based on worst-case usage
```
- Too small → allocations fail
- Too large → wasted memory
- Size = max(all object sizes) + alignment

### 2. Pool Size Calculation
```
Total memory = BLOCK_SIZE × BLOCK_COUNT + overhead
Overhead = free list pointers, metadata
```

### 3. Alignment
```c
// Align to 4-byte boundary (ARM Cortex-M)
#define ALIGN_SIZE  4
#define ALIGNED_SIZE(size) (((size) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))
```

### 4. Initialization
```c
void pool_init(pool_t *pool, void *buffer, size_t block_size, size_t count) {
    pool->buffer = buffer;
    pool->block_size = block_size;
    pool->block_count = count;
    pool->free_list = NULL;
    pool->used_count = 0;
    
    // Link all blocks into free list
    for (size_t i = 0; i < count; i++) {
        void *block = (uint8_t*)buffer + (i * block_size);
        pool_free(pool, block);
    }
}
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│           Application Layer                 │
│  tcp_conn_alloc()  packet_alloc()  ...      │
└──────────────────┬──────────────────────────┘
                   │
┌──────────────────▼──────────────────────────┐
│           Memory Pool Manager               │
│  pool_alloc()  pool_free()  pool_stats()    │
└──────────────────┬──────────────────────────┘
                   │
┌──────────────────▼──────────────────────────┐
│           Static Memory Buffer              │
│  uint8_t pool_buffer[POOL_SIZE];            │
└─────────────────────────────────────────────┘
```

---

## 📊 Comparison

| Approach | Deterministic | Fragmentation | Real-time | Complexity |
|----------|---------------|---------------|-----------|------------|
| `malloc()` | ❌ No | ❌ Yes | ❌ No | Simple |
| Static only | ✅ Yes | ✅ None | ✅ Yes | Simple |
| **Memory Pool** | ✅ Yes | ✅ None | ✅ Yes | Medium |
| RTOS heap | ⚠️ Depends | ⚠️ Depends | ⚠️ Depends | High |

---

## 🔑 Key Takeaways

1. **Deterministic** — allocation time is O(1), always succeeds or fails immediately
2. **No fragmentation** — fixed-size blocks, no gaps
3. **Real-time safe** — bounded execution time
4. **Leak detection** — track used_count, detect leaks at shutdown
5. **Pre-allocated** — all memory allocated at startup, no runtime surprises

---

## 🎯 Use Cases

**Network Stack:**
- Packet buffers (fixed size)
- TCP connections (object pool)
- Socket descriptors

**Protocol Handler:**
- Message buffers
- Command structures
- Response packets

**Data Logger:**
- Log entry buffers
- File handles
- Compression buffers

**Motor Controller:**
- Command queue entries
- Sensor sample buffers
- PID state structures

---

**Ready to see the problem?** → `01_problem.md`
