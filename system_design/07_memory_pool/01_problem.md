# Problem: Network Gateway — Memory Fragmentation

## 📋 The Scenario

You're building a **network gateway** that handles multiple protocols (TCP, UDP, MQTT, HTTP). The gateway must:

### Requirements
1. **Handle 50 concurrent TCP connections**
2. **Buffer 100 network packets** (variable size: 64-1500 bytes)
3. **Process MQTT messages** (up to 256 bytes)
4. **Log events** to SD card (128-byte log entries)
5. **Run 24/7** without restart

### Business Constraints
- **Reliability:** Must run for months without failure
- **Real-time:** Packet processing < 10ms
- **Memory:** Limited RAM (64KB total)
- **Safety:** No crashes from out-of-memory

## 🤔 Your First Attempt

You start with `malloc()` and `free()`:

```c
// network_gateway.c - First attempt with malloc/free

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    uint16_t size;
} packet_t;

typedef struct {
    int socket_fd;
    uint8_t *rx_buffer;
    uint8_t *tx_buffer;
} tcp_conn_t;

/* Allocate packet buffer */
packet_t* packet_alloc(uint16_t size) {
    packet_t *pkt = malloc(sizeof(packet_t));
    if (!pkt) return NULL;
    
    pkt->data = malloc(size);  // Variable size!
    if (!pkt->data) {
        free(pkt);
        return NULL;
    }
    
    pkt->size = size;
    return pkt;
}

/* Free packet */
void packet_free(packet_t *pkt) {
    if (pkt) {
        free(pkt->data);
        free(pkt);
    }
}

/* Allocate TCP connection */
tcp_conn_t* tcp_conn_alloc(void) {
    tcp_conn_t *conn = malloc(sizeof(tcp_conn_t));
    if (!conn) return NULL;
    
    conn->rx_buffer = malloc(1024);
    conn->tx_buffer = malloc(1024);
    
    if (!conn->rx_buffer || !conn->tx_buffer) {
        free(conn->rx_buffer);
        free(conn->tx_buffer);
        free(conn);
        return NULL;
    }
    
    return conn;
}
```

## 😱 The Problems Start

### Week 1: Works Fine

```
Day 1: All allocations succeed
Day 2: Still working
Day 3: No issues
```

### Week 2: First Failures

**Symptom:** Occasional `malloc()` returns NULL, even though total memory usage is only 40KB.

**Root cause:** Memory fragmentation!

```
Memory after 1 week:
[TCP][free][Pkt][free][TCP][free][Pkt][free][TCP]...

Fragmented! Many small free blocks, but no large contiguous block.
Need 1024 bytes → malloc() fails even though 5KB total is free!
```

### Week 3: Increasing Failures

**Symptom:** Gateway drops connections during peak traffic.

```
Peak traffic: 50 connections + 100 packets
malloc() fails → connection refused
Customer complaint: "Gateway unreliable during busy hours"
```

### Week 4: Timing Issues

**Symptom:** Packet processing takes 50ms instead of 5ms.

**Root cause:** `malloc()` searches for free block → non-deterministic!

```
malloc() timing:
  Best case:  5µs  (free block at head)
  Worst case: 2ms  (search entire heap)
  Average:    500µs (depends on fragmentation)

NOT real-time safe!
```

### Month 2: Memory Leaks

**Symptom:** Gateway crashes after 30 days uptime.

**Root cause:** Forgotten `free()` in error path:

```c
packet_t* process_packet(uint8_t *data, uint16_t size) {
    packet_t *pkt = packet_alloc(size);
    if (!pkt) return NULL;
    
    if (validate(data) < 0) {
        return NULL;  // BUG: forgot to free pkt!
    }
    
    memcpy(pkt->data, data, size);
    return pkt;
}

// After 30 days: all memory leaked → malloc() always fails
```

### Month 3: The Math Doesn't Work

**Memory usage:**
```
50 TCP connections × 2KB each = 100KB
100 packets × avg 512 bytes   = 50KB
MQTT buffers                  = 10KB
Log buffers                   = 5KB
Total needed                  = 165KB

Available RAM                 = 64KB

IMPOSSIBLE! Need 165KB but only have 64KB!
```

But with malloc(), you don't know this until runtime → random failures.

## 💭 Think About It (5 minutes)

1. **Why does fragmentation happen?**
   - Variable-size allocations
   - Allocate/free in random order
   - Gaps between blocks

2. **Why is malloc() non-deterministic?**
   - Searches heap for free block
   - Time depends on fragmentation
   - Not suitable for real-time

3. **How to detect leaks?**
   - Track allocations vs frees
   - But malloc() doesn't help

## 🎯 The Core Problems

### Problem 1: Fragmentation
```
After 1 week:
[Used][Free 10][Used][Free 5][Used][Free 20]...
Need 50 bytes → fails even though 35 bytes free!
```

### Problem 2: Non-Deterministic
```
malloc() time: 5µs to 2ms (depends on heap state)
Real-time requirement: < 10ms
Can't guarantee timing!
```

### Problem 3: No Leak Detection
```
Allocated: 1000 blocks
Freed:     998 blocks
Leaked:    2 blocks  ← Where? malloc() doesn't track!
```

### Problem 4: Unknown Limits
```
malloc() succeeds until it doesn't
No way to know: "Can I allocate 50 connections?"
Fails at runtime → too late!
```

## 📊 Impact Analysis

| Issue | Effect | Severity |
|-------|--------|----------|
| Fragmentation | Allocation failures after days | Critical |
| Non-deterministic | Missed real-time deadlines | High |
| Memory leaks | Crash after weeks | Critical |
| Unknown limits | Random failures | High |

## 💡 The Solution Preview

What if memory was pre-allocated at startup?

```c
// Memory pool — fixed-size blocks, no fragmentation

#define PACKET_POOL_SIZE  100
#define CONN_POOL_SIZE    50

static packet_t packet_pool[PACKET_POOL_SIZE];
static tcp_conn_t conn_pool[CONN_POOL_SIZE];

packet_t* packet_alloc(void) {
    // O(1) allocation from pool
    // Deterministic timing
    // No fragmentation
    // Leak detection: count used blocks
}

void packet_free(packet_t *pkt) {
    // O(1) return to pool
}

// At startup: know exact memory usage
// 100 packets × 512 bytes = 51.2KB
// 50 connections × 2KB = 100KB
// Total = 151.2KB → doesn't fit in 64KB!
// FAIL FAST at startup, not after 30 days!
```

**Benefits:**
- ✅ No fragmentation (fixed-size blocks)
- ✅ Deterministic O(1) allocation
- ✅ Leak detection (track used count)
- ✅ Known limits (fail at startup, not runtime)

## 🚀 Next Steps

1. **02_malloc_bad.c** — malloc/free simulation (shows fragmentation)
2. **03_pool_good.c** — Memory pool solution
3. **04_production.c** — Production-grade with statistics

---

**Key Takeaway:** malloc() causes fragmentation, is non-deterministic, and hides memory leaks. Memory pools provide deterministic, fragmentation-free allocation with leak detection.
