# Problem: IoT Sensor Node — Multiple Timing Tasks

## 📋 The Scenario

You're building an **IoT environmental sensor node** that monitors temperature, humidity, and air quality in a smart building. The device must:

### Requirements
1. **Blink status LED** every 500ms (device alive indicator)
2. **Read sensors** every 1000ms (temperature, humidity, CO2)
3. **Send heartbeat** to server every 30 seconds
4. **Check battery** every 5 minutes
5. **Kick watchdog** every 100ms (safety — resets MCU if software hangs)
6. **Debounce button** 50ms after press (user config button)

### Business Constraints
- **Cost:** Low-cost MCU (no RTOS license)
- **Power:** Battery-powered, must sleep between tasks
- **Reliability:** Watchdog MUST be kicked — missing it resets the device
- **Responsiveness:** Button press must feel instant

## 🤔 Your First Attempt

You start with the simplest approach — blocking delays:

```c
// iot_sensor.c - First attempt with blocking delays

#include <stdint.h>
#include <stdbool.h>

void delay_ms(uint32_t ms);  // Busy-wait delay

int main(void) {
    while (1) {
        /* Task 1: Blink LED every 500ms */
        toggle_led();
        delay_ms(500);   // ← CPU BLOCKED for 500ms

        /* Task 2: Read sensors */
        read_sensors();  // Takes ~10ms

        /* Task 3: Send heartbeat every 30s */
        static uint32_t heartbeat_count = 0;
        if (++heartbeat_count >= 60) {  // 60 × 500ms = 30s
            send_heartbeat();           // Takes ~200ms
            heartbeat_count = 0;
        }

        /* Task 4: Kick watchdog */
        kick_watchdog();  // Must happen every 100ms!
    }
}
```

## 😱 The Problems Start

### Problem 1: Watchdog Resets

**Symptom:** Device randomly resets every few minutes.

**Root cause:**
```
delay_ms(500)  ← CPU blocked here for 500ms
               ← Watchdog needs kick every 100ms
               ← 500ms >> 100ms → WATCHDOG FIRES → RESET!
```

**Attempted fix:** Kick watchdog inside delay:
```c
void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        delay_1ms();
        kick_watchdog();  // Hack: kick inside delay
    }
}
```
**Result:** Works, but now watchdog logic is scattered everywhere. What about `send_heartbeat()` which takes 200ms?

### Problem 2: Timing Drift

**Symptom:** Heartbeat arrives at server every 32 seconds, not 30.

**Root cause:**
```
Expected: 60 × 500ms = 30,000ms
Actual:   60 × (500ms delay + 10ms sensor + 200ms heartbeat) = 43,200ms
```
Processing time adds up — timing is completely wrong!

### Problem 3: Button Feels Unresponsive

**Symptom:** User presses button, nothing happens for up to 500ms.

**Root cause:**
```
Button pressed at t=0ms
CPU is inside delay_ms(500) until t=500ms
Button press not detected until next loop iteration!
```

### Problem 4: Can't Add New Tasks

**Symptom:** Adding battery check every 5 minutes breaks everything.

```c
static uint32_t battery_count = 0;
if (++battery_count >= 600) {  // 600 × 500ms = 300s = 5min
    check_battery();           // Takes ~50ms
    battery_count = 0;
}
```

Now the loop takes even longer, making all other timings wrong. The counter math becomes a nightmare to maintain.

### Week 2: The Fundamental Problem

```
Task periods:
  Watchdog:   100ms  ←─── shortest
  LED blink:  500ms
  Sensor:     1000ms
  Heartbeat:  30,000ms
  Battery:    300,000ms ←─── longest

Ratio: 300,000 / 100 = 3000×

You cannot satisfy all these with a single blocking delay!
```

The real problem is **you're trying to do multiple independent tasks with a single sequential loop**. Each `delay()` blocks everything else.

```
Timeline with blocking delays:
─────────────────────────────────────────────────────
t=0:    delay(500) ████████████████████████████████
t=500:  sensor()   ████
t=510:  delay(500) ████████████████████████████████
t=1010: sensor()   ████
...
Watchdog: ✅ only if hacked into delay
Button:   ❌ missed for up to 500ms
Heartbeat: ❌ timing wrong by 40%
```

### Week 3: Attempted Fixes (All Messy)

**Attempt 1: Shorter delay loop**
```c
while (1) {
    delay_ms(10);  // Smallest common interval
    
    led_counter += 10;
    if (led_counter >= 500) { toggle_led(); led_counter = 0; }
    
    sensor_counter += 10;
    if (sensor_counter >= 1000) { read_sensors(); sensor_counter = 0; }
    
    watchdog_counter += 10;
    if (watchdog_counter >= 100) { kick_watchdog(); watchdog_counter = 0; }
    
    // ... 3 more counters ...
}
```
**Result:** Works for simple cases, but:
- Counters drift when tasks take variable time
- Adding a new task requires adding another counter
- `read_sensors()` takes 10ms — now the 10ms loop takes 20ms!
- Doesn't scale

**Attempt 2: Timestamps with `millis()`**
```c
uint32_t last_led = 0, last_sensor = 0, last_watchdog = 0;

while (1) {
    uint32_t now = millis();
    
    if (now - last_led >= 500) { toggle_led(); last_led = now; }
    if (now - last_sensor >= 1000) { read_sensors(); last_sensor = now; }
    if (now - last_watchdog >= 100) { kick_watchdog(); last_watchdog = now; }
}
```
**Result:** Better! But:
- Still polling — wastes CPU checking all conditions every loop
- No callbacks — task logic mixed with timing logic
- Hard to add/remove tasks at runtime
- No one-shot timers (debounce)
- Doesn't scale to 10+ tasks

## 💭 Think About It (5 minutes)

Before looking at the solution, ask yourself:

1. **What's the real requirement?**
   - "Run function X every N milliseconds"
   - How can we express this cleanly?

2. **What if we separated timing from task logic?**
   - Timer: "call this function every 500ms"
   - Task: just the function, no timing code inside

3. **What's the minimum hardware needed?**
   - Do we need one hardware timer per task?
   - Or can one hardware timer serve all software timers?

## 🎯 The Core Problems

### Problem 1: Blocking Delays Monopolize CPU
```c
delay_ms(500);  // CPU does NOTHING for 500ms
                // Watchdog starves, button missed
```

### Problem 2: Timing Logic Mixed with Task Logic
```c
static uint32_t counter = 0;
if (++counter >= 60) {  // Timing logic
    send_heartbeat();   // Task logic
    counter = 0;
}
// These should be separate!
```

### Problem 3: No Abstraction for "Run Every N ms"
```c
// Every new task needs its own counter variable
// Every counter drifts differently
// No standard interface
```

## 📊 Impact Analysis

| Issue | Effect |
|-------|--------|
| Watchdog resets | Device restarts mid-operation |
| Timing drift | Heartbeat 40% late → server thinks device offline |
| Button lag | Poor user experience |
| Code complexity | Each new task adds 3+ lines of counter logic |
| Maintenance | Changing one period breaks all others |

## 💡 The Solution Preview

What if timing was completely separate from task logic?

```c
// Timer manager — register tasks with periods
timer_id_t led_timer      = timer_create(500,     PERIODIC, toggle_led);
timer_id_t sensor_timer   = timer_create(1000,    PERIODIC, read_sensors);
timer_id_t heartbeat_timer= timer_create(30000,   PERIODIC, send_heartbeat);
timer_id_t battery_timer  = timer_create(300000,  PERIODIC, check_battery);
timer_id_t watchdog_timer = timer_create(100,     PERIODIC, kick_watchdog);

// Start all timers
timer_start(led_timer);
timer_start(sensor_timer);
timer_start(heartbeat_timer);
timer_start(battery_timer);
timer_start(watchdog_timer);

// Main loop — just process pending work
while (1) {
    // Timer callbacks set flags, main loop does the work
    if (sensor_data_ready) process_sensor_data();
    if (button_pressed)    handle_button();
    
    sleep_until_next_event();  // CPU sleeps — saves power!
}
```

**Benefits:**
- ✅ Each task has its own independent timer
- ✅ No blocking delays
- ✅ Watchdog always kicked on time
- ✅ Adding new task = one line
- ✅ CPU can sleep between events

## 🚀 Next Steps

1. **02_delay_bad.c** — Blocking delay simulation (shows all failures)
2. **03_timer_good.c** — Software timer manager solution
3. **04_production.c** — Production-grade implementation

---

**Key Takeaway:** Blocking delays can only do one thing at a time. A software timer manager decouples timing from task logic, letting multiple independent tasks run concurrently on a single CPU.
