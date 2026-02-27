# Interrupt Handler - Real-Time Response in Embedded Systems

**Study Time:** 40 minutes  
**Difficulty:** Intermediate  
**Industry Use:** Critical - Used in 100% of real-time embedded systems

## 🎯 What You'll Learn

- What are interrupts and why they're essential
- How the interrupt system works (ISR, NVIC, vector table)
- ISR design rules (keep it short!)
- Interrupt priorities and nesting
- Sharing data safely between ISR and main code
- Common pitfalls and how to avoid them

## 📖 What is an Interrupt?

An **Interrupt** is a hardware signal that temporarily stops the CPU's current execution to handle an urgent event. The CPU saves its state, runs the ISR (Interrupt Service Routine), then resumes exactly where it left off.

### Real-World Analogy: Emergency Call

```
You're writing a report (main loop running)
↓
Phone rings — emergency! (hardware event fires)
↓
You bookmark your page (CPU saves context)
↓
Answer the call, handle it quickly (ISR executes)
↓
Hang up, return to report (CPU restores context)
↓
Continue writing from exactly where you stopped
```

## 🤔 Why Use Interrupts?

### Without Interrupts (Polling)

```c
void main_loop(void) {
    while (1) {
        if (timer_expired())   { handle_timer(); }
        if (uart_data_ready()) { read_uart(); }
        if (button_pressed())  { handle_button(); }

        // Problem: what if handle_timer() takes a long time?
        // uart_data_ready fires during it → MISSED!
    }
}
```

**Problems:**
- ❌ Wastes CPU cycles checking constantly
- ❌ Timing inaccurate (depends on loop speed)
- ❌ Events missed during slow operations
- ❌ Not scalable with more peripherals

### With Interrupts (Event-Driven)

```c
// Hardware calls this at EXACT intervals — no matter what main is doing
void TIMER_IRQHandler(void) {
    data = read_hardware();            // Fast!
    circular_buffer_write(data);       // Fast!
    // Done. Returns to main loop.
}

// Main loop processes at its own pace
void main_loop(void) {
    while (1) {
        if (circular_buffer_has_data()) {
            process_data();
        }
    }
}
```

**Benefits:**
- ✅ Hardware-guaranteed timing (exact intervals)
- ✅ Never miss events
- ✅ CPU free between interrupts
- ✅ Scalable to many sources

## 🏗️ Interrupt System Components

### 1. Interrupt Source
Hardware that generates interrupts:
- Timer overflow (periodic events)
- UART data received
- Button pressed (GPIO edge)
- ADC conversion complete
- DMA transfer done

### 2. Interrupt Controller (NVIC on ARM)
Manages interrupt priorities and routing:
```c
NVIC_EnableIRQ(TIMER_IRQn);          // Enable timer interrupt
NVIC_SetPriority(TIMER_IRQn, 1);     // Set priority (0 = highest)
```

### 3. Interrupt Service Routine (ISR)
The function that handles the interrupt:
```c
void TIMER_IRQHandler(void) {
    data = read_hardware();
    circular_buffer_write(data);
    TIMER_CLEAR_FLAG();  // Must clear or fires again!
}
```

### 4. Interrupt Vector Table
Maps interrupt numbers to ISR functions:
```c
const void* vector_table[] = {
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    TIMER_IRQHandler,   // Timer interrupt → this function
    UART_IRQHandler,    // UART interrupt → this function
};
```

## 📊 ISR Execution Flow

### ISR Interrupts Main Loop
```
Main loop:  ████████░░░░████████░░░░████████
                    ↑           ↑
ISR fires:          ██          ██
                  (fast)      (fast)
                  resumes     resumes
```

### Nested Interrupts (Priority)
```
Low ISR:    ████████░░░░░░░░████████
                    ↑
High ISR:           ████████
                  (interrupts low ISR)
```

## 🎯 Common Use Cases

### 1. Periodic Sampling
```c
void TIMER_IRQHandler(void) {
    uint16_t value = read_sensor();
    circular_buffer_write(&buf, value);
}

void main_loop(void) {
    while (circular_buffer_has_data(&buf)) {
        uint16_t value = circular_buffer_read(&buf);
        process(value);
    }
}
```

### 2. UART Data Reception
```c
void UART_IRQHandler(void) {
    char byte = UART_DATA_REG;
    circular_buffer_write(&uart_buf, byte);
    UART_CLEAR_FLAG();
}

void main_loop(void) {
    while (circular_buffer_has_data(&uart_buf)) {
        char byte = circular_buffer_read(&uart_buf);
        parse_protocol(byte);
    }
}
```

### 3. Button / GPIO Event
```c
volatile bool event_flag = false;

void GPIO_IRQHandler(void) {
    event_flag = true;
    GPIO_CLEAR_FLAG();
}

void main_loop(void) {
    if (event_flag) {
        handle_event();
        event_flag = false;
    }
}
```

### 4. ADC Conversion Complete
```c
void ADC_IRQHandler(void) {
    uint16_t value = ADC_DATA_REG;
    circular_buffer_write(&adc_buf, value);
}
```

## 🔧 ISR Design Rules

### Rule 1: Keep It SHORT

ISR must finish quickly and return. All heavy work goes in the main loop.

```c
// BAD: Heavy work in ISR
void TIMER_IRQHandler(void) {
    data = read_hardware();
    heavy_processing(data);   // NEVER in ISR!
    blocking_operation(data); // NEVER in ISR!
}

// GOOD: ISR only reads hardware and buffers data
void TIMER_IRQHandler(void) {
    data = read_hardware();        // Fast
    circular_buffer_write(data);   // Fast
    // Done! Main loop handles the rest.
}
```

### Rule 2: No Blocking Operations
```c
// NEVER in ISR:
delay_ms(10);      // Blocks
printf("...");     // Can block (UART)
malloc(100);       // Can block (heap lock)
blocking_write();  // Blocks!
```

### Rule 3: Use Volatile for Shared Variables
```c
// Shared between ISR and main loop
volatile uint32_t event_count = 0;   // MUST be volatile
volatile bool     data_ready  = false;

// Without volatile: compiler may cache in register
// ISR writes never seen by main loop → silent bug!
```

### Rule 4: Critical Sections for Multi-Byte Reads
```c
// 32-bit read on 8-bit CPU can be non-atomic
uint32_t count;
DISABLE_INTERRUPTS();   // Very short!
count = event_count;    // Safe atomic read
ENABLE_INTERRUPTS();
```

### Rule 5: Always Clear Interrupt Flag
```c
void TIMER_IRQHandler(void) {
    data = read_hardware();
    buffer_write(data);
    TIMER_CLEAR_FLAG();  // MUST clear or ISR fires again immediately!
}
```

## 🎯 Interrupt Priorities

```c
// Lower number = Higher priority (ARM Cortex-M)
NVIC_SetPriority(TIMER_IRQn, 0);   // Highest — most critical
NVIC_SetPriority(UART_IRQn,  1);   // High
NVIC_SetPriority(ADC_IRQn,   2);   // Medium
NVIC_SetPriority(GPIO_IRQn,  3);   // Low
```

Higher priority ISR can interrupt a lower priority ISR (nesting).

## 🐛 Common Pitfalls

### Pitfall 1: Forgetting to Clear Flag
```c
// BAD: ISR fires continuously!
void TIMER_IRQHandler(void) {
    data = read_hardware();
    // Forgot TIMER_CLEAR_FLAG() → infinite ISR loop!
}
```

### Pitfall 2: Missing Volatile
```c
// BAD: Compiler optimizes away the check
bool data_ready = false;
void ISR(void) { data_ready = true; }
void main(void) {
    while (!data_ready);  // Compiler: "never changes here"
                          // Optimizes to: while (true) → hangs!
}

// GOOD:
volatile bool data_ready = false;
```

### Pitfall 3: Heavy Work in ISR
```c
// BAD: Heavy operation in ISR — misses next interrupts!
void TIMER_IRQHandler(void) {
    heavy_processing();  // WRONG!
}
```

### Pitfall 4: Race Condition on Shared Data
```c
// BAD: Non-atomic read of 32-bit value
void main(void) {
    if (event_count > 1000) {  // ISR can change this mid-read!
        reset_count();
    }
}

// GOOD: Critical section
DISABLE_INTERRUPTS();
uint32_t count = event_count;
ENABLE_INTERRUPTS();
if (count > 1000) { reset_count(); }
```

## 📏 Design Guidelines

### DO's ✅

1. **Keep ISR as short as possible**
   - Only read hardware and write to buffer
   - All processing goes in main loop

2. **Use circular buffer between ISR and main**
   - ISR writes, main reads
   - Decouples timing from processing

3. **Batch slow operations in main loop**
   - Collect multiple items, process together
   - Amortizes cost of slow operations

4. **Use volatile for all ISR-shared variables**
   - Prevents compiler optimization bugs

5. **Always clear interrupt flag**
   - Prevents infinite ISR re-entry

### DON'Ts ❌

1. **Don't do heavy work in ISR**
   - No blocking calls, no slow operations

2. **Don't forget volatile**
   - Silent bugs that only appear with compiler optimization

3. **Don't forget to clear interrupt flag**
   - Causes infinite ISR loop

4. **Don't access shared data without critical section**
   - Race conditions corrupt data

## 🎓 When to Use Interrupts

### Perfect For ✅
- Periodic sampling (timer interrupt)
- UART/SPI/I2C data reception
- Button presses (GPIO interrupt)
- ADC conversion complete
- Any time-critical event

### Not Ideal For ❌
- Heavy processing (do in main loop)
- Blocking operations (never in ISR)
- Complex algorithms (do in main loop)

## 📊 Interrupts vs Polling

| Aspect | Polling | Interrupts |
|--------|---------|------------|
| **Timing accuracy** | Poor (loop-dependent) | Exact (hardware) |
| **CPU usage** | 100% (checking) | Low (sleep between) |
| **Events missed** | Yes (during processing) | No |
| **Response time** | Slow (ms) | Fast (µs) |
| **Scalability** | Poor | Excellent |

## 🚀 Next Steps

1. **01_problem.md** — Server room monitor: requirements, wrong approach, why it fails
2. **02_polling_bad.c** — Polling: misses samples, wrong timing
3. **03_interrupt_good.c** — Interrupt + batch: correct solution with full math
4. **04_production.c** — Production-grade implementation
5. **05_exercises.md** — Practice problems

---

**Remember:** Interrupts are the heart of real-time embedded systems. Master this pattern, and you'll build responsive, efficient systems that never miss an event!
