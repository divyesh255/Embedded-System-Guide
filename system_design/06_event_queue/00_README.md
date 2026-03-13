# Event Queue

**The pattern for decoupling event producers from event consumers**

---

## 🎯 What Problem Does This Solve?

Embedded systems receive events from many sources simultaneously:

```
Button press    →  Handle immediately?
Sensor reading  →  Process now?
Network packet  →  Parse right away?
Timer expired   →  Execute callback?
```

**The naive solution — direct callback execution — creates problems:**

```c
// WRONG: Direct execution blocks everything
void on_button_press(void) {
    debounce();           // 50ms
    update_display();     // 100ms
    send_to_cloud();      // 500ms
    // Other events blocked for 650ms!
}
```

**The solution: Event Queue**

Events are posted to a queue → processed later by main loop at controlled pace.

---

## 🔧 How It Works

### The Queue

Events are stored in a FIFO queue:

```
Producer (ISR/callback):     Consumer (main loop):
│                            │
├─ Post event → [Queue] ←────┤─ Get event
│                            │
└─ Returns immediately       └─ Process at own pace
```

### Event Structure

Each event contains:

```c
typedef struct {
    event_type_t type;      // What happened (button, sensor, network)
    uint32_t     data;      // Event-specific data
    uint32_t     timestamp; // When it happened
    uint8_t      priority;  // 0=low, 255=high
} event_t;
```

### Priority Levels

Events can have different priorities:

```
Priority 255 (Critical):  Watchdog, fault
Priority 128 (High):      Button press, alarm
Priority 64  (Normal):    Sensor reading
Priority 0   (Low):       Logging, statistics
```

Higher priority events are processed first.

---

## 📐 Queue Types

### FIFO Queue (First-In-First-Out)
Events processed in arrival order. Used for: logging, data streaming.

```
Post: A → B → C
Get:  A → B → C
```

### Priority Queue
Higher priority events jump ahead. Used for: alarms, critical events.

```
Post: A(10) → B(50) → C(20)
Get:  B(50) → C(20) → A(10)
```

### Ring Buffer Queue
Fixed size, oldest events dropped when full. Used for: sensor data, telemetry.

```
Queue size: 4
Post 5 events: A B C D E
Queue: [B C D E]  (A dropped)
```

---

## ⚙️ Key Design Decisions

### 1. Queue Size
```c
#define EVENT_QUEUE_SIZE  32  // Must hold worst-case burst
```
- Too small → events lost
- Too large → wastes RAM
- Size = (max event rate × max processing time) × 2 safety

### 2. Event Posting (Producer Side)
```c
// From ISR — must be fast!
void on_button_isr(void) {
    event_t evt = {
        .type = EVENT_BUTTON_PRESS,
        .data = button_id,
        .timestamp = sys_tick_ms,
        .priority = 128
    };
    event_post(&evt);  // Just adds to queue, returns immediately
}
```

### 3. Event Processing (Consumer Side)
```c
// Main loop — can be slow
void main_loop(void) {
    event_t evt;
    while (event_get(&evt)) {
        switch (evt.type) {
            case EVENT_BUTTON_PRESS:
                handle_button(evt.data);  // Can take 100ms
                break;
            case EVENT_SENSOR_DATA:
                process_sensor(evt.data);
                break;
        }
    }
}
```

### 4. Overflow Handling

When queue is full:
- **Drop oldest** (ring buffer) — for streaming data
- **Drop newest** (reject) — for critical events
- **Block** (wait) — only if not in ISR context

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│           Event Producers (ISRs)            │
│  Button  Sensor  Timer  Network  Watchdog   │
└──────────────────┬──────────────────────────┘
                   │ event_post()
┌──────────────────▼──────────────────────────┐
│              Event Queue                    │
│  [evt] [evt] [evt] [evt] [evt] [evt] ...    │
│   ↑                                    ↓    │
│  head                                 tail  │
└──────────────────┬──────────────────────────┘
                   │ event_get()
┌──────────────────▼──────────────────────────┐
│           Event Dispatcher                  │
│  switch(evt.type) { ... }                   │
└──────────────────┬──────────────────────────┘
                   │
┌──────────────────▼──────────────────────────┐
│           Event Handlers                    │
│  handle_button()  handle_sensor()  ...      │
└─────────────────────────────────────────────┘
```

---

## 📊 Comparison

| Approach | Decoupling | Priority | Overflow | Complexity |
|----------|------------|----------|----------|------------|
| Direct callback | ❌ None | ❌ No | N/A | Simple |
| Flags + polling | ⚠️ Partial | ❌ No | ❌ Lost | Simple |
| **Event Queue** | ✅ Full | ✅ Yes | ✅ Handled | Medium |
| RTOS mailbox | ✅ Full | ✅ Yes | ✅ Handled | High |

---

## 🔑 Key Takeaways

1. **Decoupling** — producers don't know about consumers
2. **Asynchronous** — post returns immediately, process later
3. **Priority** — critical events processed first
4. **Overflow handling** — graceful degradation when full
5. **Thread-safe** — can post from ISR, get from main loop

---

## 🎯 Use Cases

**Smart Home Hub:**
- Button presses, sensor readings, network commands
- Priority: alarm > button > sensor > log

**Motor Controller:**
- Fault detection, speed updates, position feedback
- Priority: fault > position > speed > telemetry

**Data Logger:**
- Multiple sensors, GPS, RTC, SD card
- Ring buffer: keep latest N samples

**Communication Gateway:**
- UART, SPI, I2C, CAN, Ethernet
- Priority queue: control > data > status

---

**Ready to see the problem?** → `01_problem.md`
