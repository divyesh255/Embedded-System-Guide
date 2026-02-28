# Software Timer Manager

**The pattern for running multiple timed tasks without blocking**

---

## 🎯 What Problem Does This Solve?

Embedded systems need to do many things at different time intervals:

```
LED blink:        every 500ms
Sensor read:      every 1000ms
Heartbeat send:   every 30 seconds
Battery check:    every 5 minutes
Watchdog kick:    every 100ms
```

**The naive solution — blocking delays — breaks everything:**

```c
// WRONG: Can only do ONE thing at a time
while (1) {
    delay(500);   // CPU stuck here for 500ms
    toggle_led();
    // Can't read sensor, can't kick watchdog, can't do anything!
}
```

**The solution: Software Timer Manager**

One hardware timer tick → many software timers running concurrently.

---

## 🔧 How It Works

### Hardware Foundation: The Tick

A hardware timer (SysTick on ARM Cortex-M) fires every **1ms**:

```
Hardware Timer ISR fires every 1ms:
│
├── Increment system tick counter
├── Check all software timers
│   ├── Timer 0: expired? → call callback
│   ├── Timer 1: expired? → call callback
│   └── Timer N: expired? → call callback
└── Return
```

### Software Timer Structure

Each software timer tracks:

```c
typedef struct {
    uint32_t period_ms;      // How often to fire (e.g., 500ms)
    uint32_t remaining_ms;   // Countdown to next fire
    timer_callback_t cb;     // Function to call when expired
    bool     active;         // Is this timer running?
    bool     periodic;       // Repeat (true) or one-shot (false)?
} sw_timer_t;
```

### The Tick Function

Called every 1ms from hardware ISR:

```c
void timer_tick(void) {
    sys_tick_ms++;

    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timers[i].active) continue;

        if (--timers[i].remaining_ms == 0) {
            timers[i].cb();                          // Fire callback

            if (timers[i].periodic) {
                timers[i].remaining_ms = timers[i].period_ms;  // Reload
            } else {
                timers[i].active = false;            // One-shot: stop
            }
        }
    }
}
```

---

## 📐 Timer Types

### Periodic Timer
Fires repeatedly at fixed interval. Used for: LED blink, sensor polling, heartbeat.

```
Period: 500ms
│←500ms→│←500ms→│←500ms→│
▼        ▼        ▼
FIRE     FIRE     FIRE
```

### One-Shot Timer
Fires once after a delay. Used for: debounce, timeout, retry delay.

```
Start    Delay: 200ms    Fire (once)
│←────────200ms─────────→│
                          ▼
                         FIRE (then stops)
```

### Retriggerable Timer
Resets on each trigger. Used for: watchdog, inactivity timeout.

```
Trigger  Trigger  Trigger  [no trigger]  Fire
│←100ms→│←100ms→│←100ms→│←────100ms────→│
         ↺        ↺                       ▼
        reset    reset                   FIRE
```

---

## ⚙️ Key Design Decisions

### 1. Tick Resolution
- **1ms tick** — good for most embedded systems
- Finer resolution = more ISR overhead
- Coarser resolution = less precise timing

### 2. Callback Rules (same as ISR rules)
Callbacks run from the tick ISR context:
- ✅ Set a flag
- ✅ Write to circular buffer
- ✅ Toggle GPIO
- ❌ No printf
- ❌ No blocking operations
- ❌ No malloc

### 3. Timer Pool Size
Fixed array of timers — no dynamic allocation:
```c
#define MAX_TIMERS 16
static sw_timer_t timers[MAX_TIMERS];
```

### 4. Deferred Processing Pattern
Callbacks set flags → main loop does the work:

```c
// In callback (ISR context — fast):
void on_sensor_timer(void) {
    sensor_read_pending = true;  // Just set flag
}

// In main loop (safe context — slow OK):
if (sensor_read_pending) {
    sensor_read_pending = false;
    read_sensor();        // Do actual work here
    process_data();
    send_to_cloud();
}
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│              Application Layer              │
│  led_task()  sensor_task()  heartbeat_task()│
└──────────────────┬──────────────────────────┘
                   │ flags / events
┌──────────────────▼──────────────────────────┐
│           Software Timer Manager            │
│  timer_create()  timer_start()  timer_stop()│
│  timer_reset()   timer_tick()               │
└──────────────────┬──────────────────────────┘
                   │ 1ms tick
┌──────────────────▼──────────────────────────┐
│           Hardware Timer (SysTick)          │
│  1ms interrupt → calls timer_tick()         │
└─────────────────────────────────────────────┘
```

---

## 📊 Comparison

| Approach | Multiple Tasks | CPU Usage | Precision | Complexity |
|----------|---------------|-----------|-----------|------------|
| `delay()` | ❌ One at a time | 100% wasted | ✅ Exact | Simple |
| Polling `millis()` | ✅ Yes | Low | ✅ Good | Medium |
| **SW Timer Manager** | ✅ Yes | Minimal | ✅ Good | Medium |
| RTOS tasks | ✅ Yes | Low | ✅ Good | High |

---

## 🔑 Key Takeaways

1. **One hardware timer → many software timers** — the core idea
2. **Callbacks must be fast** — they run in ISR context
3. **Deferred processing** — callbacks set flags, main loop does work
4. **Fixed pool** — no dynamic allocation, deterministic
5. **Periodic + one-shot** — two timer types cover all use cases

---

**Ready to see the problem?** → `01_problem.md`
