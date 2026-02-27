# Circular Buffer - Practice Exercises

Test your understanding of circular buffers with these hands-on exercises!

## 🎯 Exercise 1: Calculate Buffer Size (Easy - 10 min)

You're designing a UART receive buffer for a sensor that sends data at 115200 baud. Your main loop processes data every 100ms.

**Task:** Calculate the minimum buffer size needed.

**Given:**
- Baud rate: 115200 bps
- Processing interval: 100ms
- Safety margin: 2x

<details>
<summary>Solution</summary>

**Calculation:**
```
UART 8N1 framing: 1 start + 8 data + 1 stop = 10 bits per byte
Bytes per second = 115200 baud ÷ 10 bits/byte = 11520 bytes/sec
  (NOT 115200 ÷ 8 — start/stop bits are real overhead!)

Bytes per 100ms  = 11520 × 0.1 = 1152 bytes
With 2× safety   = 1152 × 2    = 2304 bytes
Next power of 2  = 4096 bytes
```

**Answer:** Use 4096-byte buffer (4KB)

**Why power of 2?**
- Fast masking: `index & 4095` instead of `index % 4096`
- Common in embedded systems
- Efficient memory alignment

**Code:**
```c
#define BUFFER_SIZE 4096
#define BUFFER_MASK 4095

// Fast wrap-around
head = (head + 1) & BUFFER_MASK;  // Instead of % BUFFER_SIZE
```
</details>

---

## 🎯 Exercise 2: Implement Peek Function (Medium - 20 min)

Implement a function to peek at multiple bytes without removing them from the buffer.

**Task:** Write `cb_peek_bulk()` function.

```c
/**
 * Peek at multiple bytes without removing
 * Returns: Number of bytes peeked
 */
uint32_t cb_peek_bulk(circular_buffer_t *cb, uint8_t *data, uint32_t len);
```

<details>
<summary>Solution</summary>

```c
uint32_t cb_peek_bulk(circular_buffer_t *cb, uint8_t *data, uint32_t len) {
    if (!cb || !data || len == 0) {
        return 0;
    }
    
    uint32_t peeked = 0;
    uint32_t temp_tail = cb->tail;
    
    // Peek without modifying tail
    while (peeked < len && peeked < cb->count) {
        data[peeked] = cb->buffer[temp_tail];
        temp_tail = (temp_tail + 1) % BUFFER_SIZE;
        peeked++;
    }
    
    return peeked;
}

// Usage example
uint8_t preview[10];
uint32_t count = cb_peek_bulk(&buffer, preview, 10);
// Data still in buffer!
```

**Key Points:**
- Don't modify tail pointer
- Use temporary variable
- Check bounds
- Return actual count
</details>

---

## 🎯 Exercise 3: Find Pattern in Buffer (Medium - 30 min)

Implement a function to search for a byte pattern in the circular buffer without removing data.

**Task:** Write `cb_find_pattern()` function.

```c
/**
 * Find pattern in buffer
 * Returns: true if found, position in *pos
 */
bool cb_find_pattern(circular_buffer_t *cb, 
                     const uint8_t *pattern, 
                     uint32_t pattern_len,
                     uint32_t *pos);
```

<details>
<summary>Solution</summary>

```c
bool cb_find_pattern(circular_buffer_t *cb, 
                     const uint8_t *pattern, 
                     uint32_t pattern_len,
                     uint32_t *pos) {
    if (!cb || !pattern || pattern_len == 0 || !pos) {
        return false;
    }
    
    if (cb->count < pattern_len) {
        return false;  // Not enough data
    }
    
    // Search through buffer
    for (uint32_t i = 0; i <= cb->count - pattern_len; i++) {
        bool match = true;
        uint32_t temp_tail = (cb->tail + i) % BUFFER_SIZE;
        
        // Check pattern
        for (uint32_t j = 0; j < pattern_len; j++) {
            uint32_t idx = (temp_tail + j) % BUFFER_SIZE;
            if (cb->buffer[idx] != pattern[j]) {
                match = false;
                break;
            }
        }
        
        if (match) {
            *pos = i;
            return true;
        }
    }
    
    return false;
}

// Usage: Find newline in UART buffer
uint8_t newline = '\n';
uint32_t pos;
if (cb_find_pattern(&uart_buffer, &newline, 1, &pos)) {
    // Found complete line at position pos
}
```

**Use Cases:**
- Finding message delimiters
- Detecting packet boundaries
- Parsing protocols
</details>

---

## 🎯 Exercise 4: Implement Watermark Alerts (Hard - 45 min)

Add high/low watermark functionality to trigger callbacks when buffer reaches certain levels.

**Task:** Extend circular buffer with watermark support.

<details>
<summary>Solution</summary>

```c
typedef void (*watermark_callback_t)(uint32_t level);

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    
    // Watermark support
    uint32_t high_watermark;
    uint32_t low_watermark;
    watermark_callback_t high_callback;
    watermark_callback_t low_callback;
    bool high_triggered;
    bool low_triggered;
} circular_buffer_wm_t;

void cb_set_watermarks(circular_buffer_wm_t *cb,
                       uint32_t high,
                       uint32_t low,
                       watermark_callback_t high_cb,
                       watermark_callback_t low_cb) {
    if (!cb) return;
    
    cb->high_watermark = high;
    cb->low_watermark = low;
    cb->high_callback = high_cb;
    cb->low_callback = low_cb;
    cb->high_triggered = false;
    cb->low_triggered = false;
}

bool cb_write_wm(circular_buffer_wm_t *cb, uint8_t data) {
    if (!cb || cb->count >= BUFFER_SIZE) {
        return false;
    }
    
    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    cb->count++;
    
    // Check high watermark
    if (cb->count >= cb->high_watermark && !cb->high_triggered) {
        cb->high_triggered = true;
        if (cb->high_callback) {
            cb->high_callback(cb->count);
        }
    }
    
    return true;
}

bool cb_read_wm(circular_buffer_wm_t *cb, uint8_t *data) {
    if (!cb || !data || cb->count == 0) {
        return false;
    }
    
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count--;
    
    // Check low watermark
    if (cb->count <= cb->low_watermark && !cb->low_triggered) {
        cb->low_triggered = true;
        if (cb->low_callback) {
            cb->low_callback(cb->count);
        }
    }
    
    // Reset high trigger when below high watermark
    if (cb->count < cb->high_watermark) {
        cb->high_triggered = false;
    }
    
    return true;
}

// Usage example
void high_water_alert(uint32_t level) {
    printf("WARNING: Buffer %u%% full!\n", (level * 100) / BUFFER_SIZE);
    // Increase processing priority
}

void low_water_alert(uint32_t level) {
    printf("INFO: Buffer %u%% full\n", (level * 100) / BUFFER_SIZE);
    // Reduce processing priority
}

circular_buffer_wm_t buffer;
cb_set_watermarks(&buffer, 
                  BUFFER_SIZE * 80 / 100,  // 80% high
                  BUFFER_SIZE * 20 / 100,  // 20% low
                  high_water_alert,
                  low_water_alert);
```

**Benefits:**
- Early warning of buffer overflow
- Dynamic priority adjustment
- Flow control
- Performance monitoring
</details>

---

## 🎯 Exercise 5: Multi-Producer Circular Buffer (Hard - 60 min)

Design a circular buffer that supports multiple producers (e.g., multiple UART ports) writing to the same buffer.

**Task:** Make the circular buffer safe for multiple producers.

<details>
<summary>Solution</summary>

```c
#include <stdatomic.h>

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    atomic_uint head;  // Atomic for multiple producers
    atomic_uint tail;  // Atomic for consumer
    atomic_uint count;
    
    // Per-producer statistics
    uint32_t producer_count[4];
    uint32_t producer_drops[4];
} mp_circular_buffer_t;

/**
 * Multi-producer write
 * Uses atomic operations for thread safety
 */
bool mp_cb_write(mp_circular_buffer_t *cb, uint8_t data, uint8_t producer_id) {
    if (!cb || producer_id >= 4) {
        return false;
    }
    
    // Atomic check and increment
    uint32_t current_count = atomic_load(&cb->count);
    if (current_count >= BUFFER_SIZE) {
        cb->producer_drops[producer_id]++;
        return false;
    }
    
    // Atomic get-and-increment head
    uint32_t write_pos = atomic_fetch_add(&cb->head, 1) % BUFFER_SIZE;
    
    // Write data
    cb->buffer[write_pos] = data;
    
    // Atomic increment count
    atomic_fetch_add(&cb->count, 1);
    
    // Update statistics
    cb->producer_count[producer_id]++;
    
    return true;
}

/**
 * Single consumer read
 */
bool mp_cb_read(mp_circular_buffer_t *cb, uint8_t *data) {
    if (!cb || !data) {
        return false;
    }
    
    // Atomic check and decrement
    uint32_t current_count = atomic_load(&cb->count);
    if (current_count == 0) {
        return false;
    }
    
    // Atomic get-and-increment tail
    uint32_t read_pos = atomic_fetch_add(&cb->tail, 1) % BUFFER_SIZE;
    
    // Read data
    *data = cb->buffer[read_pos];
    
    // Atomic decrement count
    atomic_fetch_sub(&cb->count, 1);
    
    return true;
}

// Usage: Multiple UART ports
void UART1_IRQHandler(void) {
    uint8_t data = UART1_DATA;
    mp_cb_write(&shared_buffer, data, 0);  // Producer 0
}

void UART2_IRQHandler(void) {
    uint8_t data = UART2_DATA;
    mp_cb_write(&shared_buffer, data, 1);  // Producer 1
}

void process_data(void) {
    uint8_t data;
    while (mp_cb_read(&shared_buffer, &data)) {
        // Process data from any UART
    }
}
```

**Key Concepts:**
- Atomic operations for thread safety
- Lock-free design
- Per-producer statistics
- Single consumer (simplifies design)

**Trade-offs:**
- More complex than single-producer
- Requires atomic support
- Slightly slower
- But no locks needed!
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Buffer Sizing**
   - Calculate based on data rate and latency
   - Use power-of-2 sizes
   - Add safety margin

2. **Advanced Operations**
   - Peek without removing
   - Pattern searching
   - Bulk operations

3. **Monitoring**
   - Watermark alerts
   - Statistics tracking
   - Performance analysis

4. **Concurrency**
   - Multi-producer support
   - Atomic operations
   - Lock-free design

5. **Real-World Usage**
   - UART buffering
   - Sensor data queuing
   - Protocol parsing

## 🚀 Next Steps

1. **Practice:** Implement circular buffer in your projects
2. **Optimize:** Profile and optimize for your platform
3. **Test:** Write unit tests for edge cases
4. **Apply:** Use in UART, SPI, I2C drivers

---

**Congratulations!** You've mastered Circular Buffers - one of the most essential patterns in embedded systems!
