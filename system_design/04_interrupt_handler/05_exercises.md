# Interrupt Handler - Practice Exercises

Test your understanding with these hands-on exercises!

---

## 🎯 Exercise 1: Buffer Sizing Calculation (Easy - 10 min)

A new system has these requirements:
- Sample rate: 1 sensor every **5ms** (200 Hz)
- SD write: **40ms** per write (any number of samples)
- Display update: **20ms**
- Network send: **80ms** (only on alert)

**Task:** Calculate the minimum circular buffer size needed.

<details>
<summary>Solution</summary>

**Step 1: Find worst-case blocking time**

Always include ALL operations — even rare ones like network alert:
```
SD write:       40ms  (always)
Display update: 20ms  (always)
Network send:   80ms  (rare, but MUST include for worst case!)
─────────────────────
Worst case:     140ms per batch
```

**Step 2: Samples arriving during worst-case blocking**
```
Samples during 140ms = 140ms / 5ms = 28 samples
```

**Step 3: Apply safety factor**
```
Buffer size = 28 × 4 = 112 minimum
→ Use 128 (next power of 2)
```

**Step 4: Verify buffer oscillation**
```
Buffer drains to 0 each iteration
Worst case fills to 28 during processing
28 < 128 → Never overflows ✅
```

**Answer: 128 slots**

**Why include the alert even though it's rare?**
Buffer sizing is worst-case. If the alert fires once and the buffer is too small → data loss.
</details>

---

## 🎯 Exercise 2: Fix the Volatile Bug (Easy - 15 min)

This code has a silent bug. Find and fix it.

```c
uint32_t sample_count = 0;
bool     data_ready   = false;

void TIMER_IRQHandler(void) {
    sample_count++;
    data_ready = true;
}

void main_loop(void) {
    while (!data_ready) {
        /* wait */
    }
    printf("Got %u samples\n", sample_count);
    data_ready = false;
}
```

<details>
<summary>Solution</summary>

**Bug:** Missing `volatile` on both shared variables.

**Problem:**
- Compiler sees `data_ready` never changes in `main_loop`
- Optimizes `while (!data_ready)` to `while (true)` → infinite loop!
- Same for `sample_count` — may be cached in register

**Fixed:**
```c
volatile uint32_t sample_count = 0;  // MUST be volatile
volatile bool     data_ready   = false;  // MUST be volatile

void TIMER_IRQHandler(void) {
    sample_count++;
    data_ready = true;
}

void main_loop(void) {
    while (!data_ready) {  // Now checks memory every iteration
        /* wait */
    }
    printf("Got %u samples\n", sample_count);
    data_ready = false;
}
```

**Rule:** Every variable shared between ISR and main loop MUST be `volatile`.
</details>

---

## 🎯 Exercise 3: Identify the ISR Mistake (Medium - 20 min)

What is wrong with this ISR? List all problems.

```c
char uart_buffer[256];
int  uart_idx = 0;

void UART_IRQHandler(void) {
    char byte = UART_DATA_REG;
    uart_buffer[uart_idx] = byte;
    uart_idx++;

    if (byte == '\n') {
        printf("Received: %s\n", uart_buffer);  // Print complete line
        uart_idx = 0;
    }
}
```

<details>
<summary>Solution</summary>

**Problem 1: `printf` in ISR**
```c
printf("Received: %s\n", uart_buffer);  // WRONG!
// printf uses UART which can block
// Can take milliseconds
// Will miss incoming bytes!
```

**Problem 2: Missing `volatile`**
```c
char uart_buffer[256];  // Should be volatile
int  uart_idx = 0;      // Should be volatile
```

**Problem 3: No bounds check**
```c
uart_buffer[uart_idx] = byte;
uart_idx++;
// What if uart_idx reaches 256? Buffer overflow!
```

**Problem 4: Missing interrupt flag clear**
```c
// UART_CLEAR_FLAG(); missing → ISR fires again immediately!
```

**Fixed:**
```c
volatile char uart_buffer[256];
volatile int  uart_idx = 0;
volatile bool line_ready = false;

void UART_IRQHandler(void) {
    char byte = UART_DATA_REG;
    UART_CLEAR_FLAG();  // Clear first!

    if (uart_idx < 255) {  // Bounds check
        uart_buffer[uart_idx++] = byte;
    }

    if (byte == '\n') {
        uart_buffer[uart_idx] = '\0';
        line_ready = true;  // Signal main loop
        uart_idx = 0;
        // NO printf here!
    }
}

void main_loop(void) {
    if (line_ready) {
        printf("Received: %s\n", (char*)uart_buffer);  // printf in main ✅
        line_ready = false;
    }
}
```
</details>

---

## 🎯 Exercise 4: Design a New System (Medium - 30 min)

Design the interrupt system for a weather station:

**Requirements:**
- Sample temperature + humidity every **20ms** (50 Hz)
- Log to flash memory: **60ms** per write (log every sample)
- Update e-ink display: **200ms** (update every 10 seconds — show latest reading)
- Send to cloud: **150ms** (send every 60 seconds — send **latest sample only**, not all 3000)

**Important clarification on cloud send:**
The cloud send transmits only the **latest sample** (or a computed average) — NOT all samples accumulated over 60 seconds. Sending all 3000 samples would require a 3000-slot buffer, which is impractical. Instead, the main loop keeps a running average and sends that once per minute.

**Tasks:**
1. Calculate buffer size
2. Calculate max buffer usage
3. Verify the system works

<details>
<summary>Solution</summary>

**Step 1: Clarify what "send to cloud" means**

```
❌ WRONG: send all 3000 samples accumulated over 60 seconds
   → Needs 3000-slot buffer — impractical!

✅ CORRECT: send only the latest sample (or running average)
   → Buffer only needs to hold samples for current batch window
```

The circular buffer is NOT a 60-second accumulator. It only holds samples between ISR fires and the next main loop drain. The main loop keeps a separate running average for the cloud.

**Step 2: Determine operation frequencies**
```
Sample interval: 20ms
Flash write:     60ms — every batch
Display update:  200ms — every 500 samples (10 seconds)
Cloud send:      150ms — every 3000 samples (60 seconds), latest sample only
```

**Step 3: Find worst-case blocking time**
```
Flash write:    60ms  (always)
Display update: 200ms (every 500 samples)
Cloud send:     150ms (every 3000 samples)
─────────────────────────────────────────
Worst case:     60ms + 200ms + 150ms = 410ms (all three in same batch)
```

**Step 4: Calculate max buffer usage**
```
Worst case: 410ms / 20ms = 20.5 → 21 samples
```

**Step 5: Buffer size**
```
Buffer = 21 × 4 = 84 minimum
→ Use 128 (next power of 2) ✅
```

**Step 6: Verify**
```
Buffer drains to 0 each iteration
Worst case fills to 21 during processing
21 < 128 → Never overflows ✅
```

**Main loop design:**
```c
#define DISPLAY_EVERY  500   // Every 10 seconds (500 × 20ms)
#define CLOUD_EVERY   3000   // Every 60 seconds (3000 × 20ms)

static uint32_t total_samples = 0;
static float    running_avg   = 0.0f;  // For cloud summary

void main_loop_iteration(void) {
    // Drain entire buffer
    sample_t batch[128];
    uint32_t batch_size = 0;
    while (circ_read(&batch[batch_size])) batch_size++;

    total_samples += batch_size;

    // Update running average (cheap, no blocking)
    for (uint32_t i = 0; i < batch_size; i++) {
        running_avg = 0.99f * running_avg + 0.01f * batch[i].temp;
    }

    // Always: write batch to flash (60ms)
    flash_write(batch, batch_size);

    // Every 10 seconds: update display with latest sample (200ms)
    if (total_samples % DISPLAY_EVERY < batch_size) {
        update_display(batch[batch_size - 1]);
    }

    // Every 60 seconds: send running average to cloud (150ms)
    // NOT all 3000 samples — just the summary!
    if (total_samples % CLOUD_EVERY < batch_size) {
        cloud_send_summary(running_avg);  // 150ms, single value
    }
}
```
</details>

---

## 🎯 Exercise 5: Race Condition Analysis (Hard - 30 min)

This code has a race condition. Explain what can go wrong and fix it.

```c
volatile uint32_t total_samples = 0;
volatile uint32_t error_count   = 0;

void SENSOR_ISR(void) {
    uint16_t value = read_sensor();
    if (value > 1000) {
        error_count++;
    }
    total_samples++;
}

void main_loop(void) {
    if (total_samples > 0 && error_count > 0) {
        float error_rate = (float)error_count / total_samples;
        printf("Error rate: %.2f%%\n", error_rate * 100);
        total_samples = 0;
        error_count   = 0;
    }
}
```

<details>
<summary>Solution</summary>

**Race Condition 1: Non-atomic read of two variables**
```
Main reads total_samples = 100
ISR fires! total_samples = 101, error_count = 5
Main reads error_count = 5
Main calculates: 5/100 = 5% (WRONG! Should be 5/101)
```

**Race Condition 2: Non-atomic reset**
```
Main sets total_samples = 0
ISR fires! total_samples = 1
Main sets error_count = 0
Result: total_samples=1, error_count=0 (lost the ISR data!)
```

**Fixed: Atomic snapshot with critical section**
```c
volatile uint32_t total_samples = 0;
volatile uint32_t error_count   = 0;

void SENSOR_ISR(void) {
    uint16_t value = read_sensor();
    if (value > 1000) {
        error_count++;
    }
    total_samples++;
}

void main_loop(void) {
    // Atomic snapshot of both variables
    DISABLE_INTERRUPTS();
    uint32_t snap_total  = total_samples;
    uint32_t snap_errors = error_count;
    total_samples = 0;
    error_count   = 0;
    ENABLE_INTERRUPTS();

    // Now use snapshot safely (no ISR interference)
    if (snap_total > 0 && snap_errors > 0) {
        float error_rate = (float)snap_errors / snap_total;
        printf("Error rate: %.2f%%\n", error_rate * 100);
    }
}
```

**Key:** Read AND reset both variables in one critical section.
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Buffer Sizing**
   - Worst-case = ALL operations in one batch (including rare ones like alerts)
   - Worst-case block time ÷ sample interval = max buffer usage
   - Apply 4× safety factor, round to power of 2

2. **Volatile**
   - Every ISR-shared variable MUST be volatile
   - Prevents compiler optimization bugs

3. **ISR Rules**
   - No printf, no blocking, no slow operations
   - Always clear interrupt flag
   - Always check bounds

4. **Drain Pattern**
   - Read entire buffer each iteration
   - Do slow operations ONCE per batch
   - Buffer oscillates 0 ↔ N, never overflows

5. **Critical Sections**
   - Snapshot multiple related variables atomically
   - Keep critical sections as short as possible

---

**Congratulations!** You've mastered Interrupt Handlers — the foundation of all real-time embedded systems!
