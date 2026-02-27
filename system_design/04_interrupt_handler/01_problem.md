# The Problem: Server Room Temperature Monitor

## 🏭 Real-World Scenario

You're hired to build an embedded system for a data center company. Their server room has 4 server racks, each with a temperature sensor. They need a monitoring system to prevent overheating.

## 📋 Requirements

| # | Requirement | Detail |
|---|-------------|--------|
| 1 | Sample all 4 sensors | Every **10ms** (100 Hz) |
| 2 | Log every sample to SD card | Compliance — auditors check this |
| 3 | Show temperatures on LCD | Real-time display |
| 4 | Send network alert | If any sensor > 80°C |
| 5 | Never miss a sample | Missing data = compliance violation |

## ⏱️ Hardware Timing (Measured)

| Operation | Time |
|-----------|------|
| Read 4 sensors | 1ms |
| Write N samples to SD card | 50ms (one write, any N) |
| Update LCD display | 30ms |
| Send network alert | 100ms |

## ❌ Wrong Approach: Polling

The junior engineer writes this:

```c
void main_loop(void) {
    while (1) {
        if (timer_expired()) {
            read_sensors();      // 1ms
            write_to_sd();       // 50ms
            update_display();    // 30ms
            // Total: 81ms per loop
        }
    }
}
```

### Why It Fails

```
Required sample interval: 10ms
Actual loop time:         81ms

In 81ms, the timer fires at:
  10ms → MISSED (busy writing SD)
  20ms → MISSED (busy writing SD)
  30ms → MISSED (busy writing SD)
  40ms → MISSED (busy writing SD)
  50ms → MISSED (busy writing SD)
  60ms → MISSED (busy updating display)
  70ms → MISSED (busy updating display)
  80ms → MISSED (busy updating display)
  81ms → Caught (loop restarts)

Samples caught:  1 out of 8
Data loss:       87.5%
```

**Consequences:**
- ❌ Compliance audit fails (missing 87.5% of data)
- ❌ Overheating not detected in time
- ❌ Servers damaged

## ✅ Correct Approach: Interrupt + Drain Buffer

### Step 1: ISR handles timing (never misses a sample)

```c
// Hardware timer fires EXACTLY every 10ms
void TIMER_IRQHandler(void) {
    read_sensors();                // 1ms — fast!
    circular_buffer_write(sample); // <1ms — fast!
    // Returns. Main loop resumes.
}
```

The ISR is always called at exactly 10ms regardless of what main is doing.

### Step 2: Main loop drains the ENTIRE buffer each iteration

Key insight: instead of processing one sample at a time, read ALL available samples from the buffer, then do the slow operations ONCE for the whole batch.

```c
void main_loop(void) {
    // Step A: Drain entire buffer into local batch
    sample_t batch[32];
    uint32_t batch_size = 0;

    while (circular_buffer_has_data()) {
        batch[batch_size++] = circular_buffer_read();
    }
    // Buffer is now EMPTY ✅

    if (batch_size == 0) return;

    // Step B: Write entire batch to SD — ONE write for all samples
    sd_write_batch(batch, batch_size);   // 50ms

    // Step C: Update display with latest sample — ONCE per batch
    update_display(batch[batch_size - 1]);  // 30ms

    // Step D: Check alerts — only if threshold exceeded
    for (int i = 0; i < batch_size; i++) {
        if (batch[i].temp > 80) {
            send_alert(batch[i]);  // 100ms (rare)
            break;
        }
    }
}
```

### Step 3: Verify the buffer never overflows

**Worst-case blocking time** = all operations happen in the same batch:

```
SD write:       50ms  (always)
Display update: 30ms  (always)
Network alert:  100ms (rare, but MUST include for worst case!)
─────────────────────
Worst case:     180ms per batch
```

```
Samples arriving during worst-case 180ms:
  = 180ms / 10ms = 18 samples

Buffer size needed:
  = 18 samples × 4 (safety factor) = 72 slots
  → Use 128 (next power of 2) ✅
```

**Why include the alert even though it's rare?**
Buffer sizing is a worst-case calculation. If the alert fires even once and the buffer is too small, data is lost. Always size for the worst case.

### Step 4: Trace the timeline

**Normal batch (no alert): 80ms**
```
Time 0ms:    Buffer = 0, main loop idle
Time 10ms:   ISR → buffer = 1
...
Time 80ms:   ISR → buffer = 8
Time 80ms:   Main: drain 8 samples, buffer = 0
Time 80ms:   Main: SD write (50ms)
Time 130ms:  Main: display update (30ms)
Time 160ms:  Main: done. Buffer has 8 new samples.
```

**Worst-case batch (alert fires): 180ms**
```
Time 0ms:    Buffer = 0
Time 10ms:   ISR → buffer = 1
...
Time 180ms:  ISR → buffer = 18
Time 180ms:  Main: drain 18 samples, buffer = 0
Time 180ms:  Main: SD write (50ms)
Time 230ms:  Main: display update (30ms)
Time 260ms:  Main: send alert (100ms)  ← worst case!
Time 360ms:  Main: done. Buffer has 18 new samples.

Buffer max = 18 samples < 128 slots ✅
Zero samples missed ✅
```

## 📊 Results Comparison

| Metric | Polling | Interrupt + Drain |
|--------|---------|-------------------|
| Sample timing | 81ms (wrong) | 10ms (exact) ✅ |
| Data loss | 87.5% | 0% ✅ |
| Compliance | Fails | Passes ✅ |
| Buffer overflow | N/A | Never (max 18 < 128) ✅ |
| Overheating detection | 81ms delay | 10ms delay ✅ |

## 🎯 Key Insight

**Drain the entire buffer each iteration — don't process one sample at a time.**

- ISR guarantees exact 10ms sampling (hardware-enforced)
- Main loop drains buffer completely → buffer always returns to 0
- Slow operations (SD, display) done ONCE per batch, not per sample
- Buffer (128 slots) handles worst-case 18 samples (SD+display+alert)

## 🚀 Next Files

- **02_polling_bad.c** — See the polling failure in code
- **03_interrupt_good.c** — See the correct solution in code
