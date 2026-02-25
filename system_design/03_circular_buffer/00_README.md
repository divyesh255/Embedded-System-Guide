# Circular Buffer - Essential Data Structure for Embedded Systems

**Study Time:** 35 minutes  
**Difficulty:** Beginner  
**Industry Use:** Universal - Used in 95%+ of embedded products

## 🎯 What You'll Learn

- What is a circular buffer (ring buffer)?
- Why it's essential for embedded systems
- How to implement it correctly
- Common use cases (UART, sensors, audio)
- Thread-safe implementations
- Avoiding common pitfalls

## 📖 What is a Circular Buffer?

A **Circular Buffer** (also called Ring Buffer) is a fixed-size buffer that wraps around when it reaches the end, forming a logical circle. It's perfect for streaming data where you want to continuously write and read without moving data in memory.

### Real-World Analogy: Conveyor Belt

```
    Write →  [■][■][■][ ][ ][ ]  ← Read
             ↑               ↑
           Head            Tail
           
    When full, head wraps to start:
    
    Write →  [■][■][■][■][■][■]  ← Read
             ↑
          Head/Tail
```

Think of a circular conveyor belt at a sushi restaurant - dishes keep coming around, you take what you want, and new dishes are added continuously.

## 🤔 Why Use Circular Buffers?

### The Problem: Linear Buffer

```c
// Linear buffer - BAD for streaming data
char buffer[100];
int count = 0;

void add_data(char data) {
    if (count < 100) {
        buffer[count++] = data;
    } else {
        // Buffer full! Must shift all data or lose it
        memmove(buffer, buffer+1, 99);  // SLOW!
        buffer[99] = data;
    }
}
```

**Problems:**
- ❌ Expensive data movement (memmove)
- ❌ Loses data when full
- ❌ Not suitable for continuous streams
- ❌ Poor performance

### The Solution: Circular Buffer

```c
// Circular buffer - GOOD for streaming data
char buffer[100];
int head = 0;  // Write position
int tail = 0;  // Read position

void add_data(char data) {
    buffer[head] = data;
    head = (head + 1) % 100;  // Wrap around!
    // No data movement needed!
}

char get_data(void) {
    char data = buffer[tail];
    tail = (tail + 1) % 100;  // Wrap around!
    return data;
}
```

**Benefits:**
- ✅ No data movement
- ✅ O(1) operations
- ✅ Perfect for streaming
- ✅ Fixed memory usage

## 🏗️ Circular Buffer Components

### 1. Buffer Array
```c
uint8_t buffer[BUFFER_SIZE];  // Fixed-size array
```

### 2. Head Pointer (Write)
```c
uint32_t head;  // Where to write next
```

### 3. Tail Pointer (Read)
```c
uint32_t tail;  // Where to read next
```

### 4. Size/Count
```c
uint32_t count;  // How many items in buffer
// OR
bool full;      // Is buffer full?
```

## 📊 Buffer States

### Empty Buffer
```
[ ][ ][ ][ ][ ][ ][ ][ ]
 ↑
head/tail
count = 0
```

### Partially Full
```
[■][■][■][ ][ ][ ][ ][ ]
       ↑           ↑
      head        tail
count = 3
```

### Full Buffer
```
[■][■][■][■][■][■][■][■]
 ↑
head/tail
count = 8
```

### Wrapped Around
```
[■][■][ ][ ][ ][■][■][■]
     ↑           ↑
    head        tail
count = 5
```

## 🎯 Common Use Cases

### 1. UART Receive Buffer
```c
// Interrupt handler writes to buffer
void UART_IRQHandler(void) {
    char data = UART_DATA_REG;
    circular_buffer_write(&uart_rx_buffer, data);
}

// Main loop reads from buffer
void main_loop(void) {
    if (!circular_buffer_empty(&uart_rx_buffer)) {
        char data = circular_buffer_read(&uart_rx_buffer);
        process_data(data);
    }
}
```

**Why it works:**
- Interrupt writes at any time
- Main loop reads when ready
- No data loss (if buffer sized correctly)
- Decouples producer and consumer

### 2. Sensor Data Buffering
```c
// ADC interrupt stores samples
void ADC_IRQHandler(void) {
    uint16_t sample = ADC_READ();
    circular_buffer_write(&adc_buffer, sample);
}

// Processing task reads samples
void process_samples(void) {
    while (!circular_buffer_empty(&adc_buffer)) {
        uint16_t sample = circular_buffer_read(&adc_buffer);
        apply_filter(sample);
    }
}
```

### 3. Command Queue
```c
// User input adds commands
void add_command(command_t cmd) {
    circular_buffer_write(&cmd_queue, cmd);
}

// Command processor executes
void command_processor(void) {
    if (!circular_buffer_empty(&cmd_queue)) {
        command_t cmd = circular_buffer_read(&cmd_queue);
        execute_command(cmd);
    }
}
```

### 4. Audio Streaming
```c
// DMA fills buffer with audio samples
void DMA_IRQHandler(void) {
    for (int i = 0; i < CHUNK_SIZE; i++) {
        circular_buffer_write(&audio_buffer, audio_data[i]);
    }
}

// Audio codec reads samples
void audio_output(void) {
    if (circular_buffer_count(&audio_buffer) >= MIN_SAMPLES) {
        uint16_t sample = circular_buffer_read(&audio_buffer);
        DAC_OUTPUT(sample);
    }
}
```

## 🔧 Implementation Approaches

### Approach 1: Using Count
```c
typedef struct {
    uint8_t *buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t count;  // Number of items
} circular_buffer_t;

bool is_full(circular_buffer_t *cb) {
    return cb->count == cb->size;
}

bool is_empty(circular_buffer_t *cb) {
    return cb->count == 0;
}
```

**Pros:** Simple, clear state  
**Cons:** Extra variable

### Approach 2: Using Full Flag
```c
typedef struct {
    uint8_t *buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    bool full;  // Full flag
} circular_buffer_t;

bool is_full(circular_buffer_t *cb) {
    return cb->full;
}

bool is_empty(circular_buffer_t *cb) {
    return !cb->full && (cb->head == cb->tail);
}
```

**Pros:** Can use full buffer capacity  
**Cons:** Slightly more complex logic

### Approach 3: Sacrifice One Slot
```c
// Keep one slot empty to distinguish full from empty
bool is_full(circular_buffer_t *cb) {
    return ((cb->head + 1) % cb->size) == cb->tail;
}

bool is_empty(circular_buffer_t *cb) {
    return cb->head == cb->tail;
}
```

**Pros:** Simple logic, no extra variables  
**Cons:** Wastes one slot

## 🐛 Common Pitfalls

### Pitfall 1: Modulo is Slow
```c
// BAD: Modulo on every operation
head = (head + 1) % size;

// BETTER: Use power-of-2 size and mask
head = (head + 1) & (size - 1);  // If size is power of 2

// BEST: Check and wrap
head++;
if (head >= size) head = 0;
```

### Pitfall 2: Not Thread-Safe
```c
// BAD: Race condition!
void write(uint8_t data) {
    buffer[head] = data;
    head = (head + 1) % size;  // Interrupt here = corruption!
}

// GOOD: Atomic operations or disable interrupts
void write(uint8_t data) {
    DISABLE_INTERRUPTS();
    buffer[head] = data;
    head = (head + 1) % size;
    ENABLE_INTERRUPTS();
}
```

### Pitfall 3: Overflow Handling
```c
// BAD: Silent data loss
void write(uint8_t data) {
    buffer[head] = data;
    head = (head + 1) % size;
    // Overwrites old data if full!
}

// GOOD: Check before write
bool write(uint8_t data) {
    if (is_full()) {
        return false;  // Or handle overflow
    }
    buffer[head] = data;
    head = (head + 1) % size;
    return true;
}
```

### Pitfall 4: Wrong Size Calculation
```c
// BAD: Off-by-one error
uint32_t count = (head - tail) % size;

// GOOD: Handle wrap-around correctly
uint32_t count = (head >= tail) ? 
                 (head - tail) : 
                 (size - tail + head);
```

## 📏 Design Guidelines

### DO's ✅

1. **Use Power-of-2 Sizes**
   ```c
   #define BUFFER_SIZE 256  // Fast masking
   ```

2. **Check Full Before Write**
   ```c
   if (!is_full(&buffer)) {
       write(&buffer, data);
   }
   ```

3. **Check Empty Before Read**
   ```c
   if (!is_empty(&buffer)) {
       data = read(&buffer);
   }
   ```

4. **Make Thread-Safe**
   ```c
   // Disable interrupts or use atomics
   ```

5. **Size Appropriately**
   ```c
   // Size = (max_rate * max_latency) + margin
   ```

### DON'Ts ❌

1. **Don't Use Modulo in ISR**
   ```c
   // Slow in interrupt!
   head = (head + 1) % size;
   ```

2. **Don't Ignore Overflow**
   ```c
   // Always check full condition
   ```

3. **Don't Use Dynamic Allocation**
   ```c
   // Use static buffers in embedded
   ```

4. **Don't Forget Volatile**
   ```c
   // If accessed from ISR
   volatile uint32_t head;
   ```

## 🎓 When to Use Circular Buffers

### Perfect For ✅
- UART/SPI/I2C receive buffers
- Sensor data streaming
- Audio/video buffers
- Command queues
- Event queues
- Logging systems
- DMA transfers

### Not Ideal For ❌
- Random access needed
- Variable-size messages
- Priority-based access
- Sorted data

## 📊 Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Write | O(1) | O(1) |
| Read | O(1) | O(1) |
| Check Full | O(1) | O(1) |
| Check Empty | O(1) | O(1) |
| Get Count | O(1) | O(1) |

**Memory:** Fixed - O(n) where n is buffer size

## 🚀 Next Steps

Now that you understand the concept, let's see it in action:

1. **01_problem.md** - UART data loss problem
2. **02_array_bad.c** - Linear buffer approach (fails)
3. **03_circular_good.c** - Circular buffer solution
4. **04_production.c** - Thread-safe implementation
5. **05_exercises.md** - Practice problems

---

**Remember:** Circular buffers are the workhorse of embedded systems. Master this pattern, and you'll use it in every project!
