# Event Queue - Practice Exercises

Test your understanding of event queues with these hands-on exercises!

---

## 🎯 Exercise 1: Calculate Queue Size (Easy - 10 min)

A data logger receives events from 5 sensors at different rates:

| Source | Rate | Processing Time |
|--------|------|-----------------|
| GPS | 1 Hz | 50ms |
| Accelerometer | 10 Hz | 10ms |
| Temperature | 0.1 Hz | 20ms |
| Pressure | 0.5 Hz | 15ms |
| Battery | 0.01 Hz | 5ms |

**Task:** Calculate minimum queue size for 10-second burst.

<details>
<summary>Solution</summary>

**Events in 10 seconds:**
```
GPS:           10 events  (1 Hz × 10s)
Accelerometer: 100 events (10 Hz × 10s)
Temperature:   1 event    (0.1 Hz × 10s)
Pressure:      5 events   (0.5 Hz × 10s)
Battery:       0 events   (0.01 Hz × 10s)
Total:         116 events
```

**Processing time:**
```
GPS:           10 × 50ms  = 500ms
Accelerometer: 100 × 10ms = 1000ms
Temperature:   1 × 20ms   = 20ms
Pressure:      5 × 15ms   = 75ms
Battery:       0 × 5ms    = 0ms
Total:         1595ms = 1.6 seconds
```

**Queue size:**
```
Events arrive in 10s, processed in 1.6s
All events fit in queue during burst
Minimum: 116 events
With 2× safety: 232 events
Next power of 2: 256 events
```

**Answer:** Use 256-event queue.
</details>

---

## 🎯 Exercise 2: Design Priority Scheme (Medium - 15 min)

Design priority levels for a medical device with these events:

- Patient alarm (critical)
- Sensor reading (routine)
- Button press (user interaction)
- Network sync (background)
- Battery low warning (important)
- Self-test (maintenance)

**Task:** Assign priority values (0-255) and justify.

<details>
<summary>Solution</summary>

```c
#define PRI_CRITICAL    255  // Patient alarm
#define PRI_URGENT      192  // Battery low
#define PRI_HIGH        128  // Button press
#define PRI_NORMAL      64   // Sensor reading
#define PRI_LOW         32   // Network sync
#define PRI_BACKGROUND  16   // Self-test
```

**Justification:**

1. **Patient alarm (255)** — Life-threatening, must process immediately
2. **Battery low (192)** — Device shutdown imminent, urgent but not immediate
3. **Button press (128)** — User expects instant response
4. **Sensor reading (64)** — Regular operation, can wait briefly
5. **Network sync (32)** — Background task, not time-critical
6. **Self-test (16)** — Lowest priority, run when idle

**Gap reasoning:**
- Large gaps allow inserting new priorities later
- Critical events have clear separation from others
- Similar-priority events use FIFO within priority level
</details>

---

## 🎯 Exercise 3: Implement Event Filtering (Medium - 20 min)

Add event filtering to only process specific event types.

**Task:** Implement `event_filter_set()` and modify `event_get()`.

<details>
<summary>Solution</summary>

```c
static uint32_t event_filter_mask = 0xFFFFFFFF;  // All enabled

void event_filter_set(event_type_t type, bool enabled) {
    if (type >= 32) return;  // Max 32 event types
    
    if (enabled) {
        event_filter_mask |= (1 << type);   // Enable
    } else {
        event_filter_mask &= ~(1 << type);  // Disable
    }
}

bool event_filter_enabled(event_type_t type) {
    if (type >= 32) return false;
    return (event_filter_mask & (1 << type)) != 0;
}

bool event_get_filtered(event_t *evt) {
    if (queue.count == 0) return false;
    
    /* Find highest priority ENABLED event */
    int max_idx = -1;
    for (uint32_t i = 0; i < queue.count; i++) {
        if (!event_filter_enabled(queue.events[i].type)) {
            continue;  // Skip disabled events
        }
        
        if (max_idx == -1 || 
            queue.events[i].priority > queue.events[max_idx].priority) {
            max_idx = i;
        }
    }
    
    if (max_idx == -1) return false;  // No enabled events
    
    *evt = queue.events[max_idx];
    
    /* Remove */
    for (uint32_t i = max_idx; i < queue.count - 1; i++) {
        queue.events[i] = queue.events[i + 1];
    }
    queue.count--;
    
    return true;
}

// Usage:
event_filter_set(EVENT_TEMPERATURE, false);  // Disable temp events
event_filter_set(EVENT_MOTION, false);       // Disable motion events
// Now only button, alarm, network events processed
```

**Use cases:**
- Disable non-critical events during emergency
- Filter events during calibration
- Selective logging
</details>

---

## 🎯 Exercise 4: Detect Event Starvation (Hard - 25 min)

Low-priority events never get processed if high-priority events keep arriving.

**Task:** Implement age-based priority boost to prevent starvation.

<details>
<summary>Solution</summary>

```c
#define MAX_EVENT_AGE_MS  5000  // Boost after 5 seconds

typedef struct {
    event_type_t type;
    uint32_t     data;
    uint32_t     timestamp;
    uint8_t      base_priority;    // Original priority
    uint8_t      effective_priority;  // Boosted priority
    uint16_t     sequence;
} event_t;

bool event_post(event_type_t type, uint32_t data, uint8_t priority) {
    /* ... existing code ... */
    
    evt->base_priority      = priority;
    evt->effective_priority = priority;  // Initially same
    
    return true;
}

void event_update_priorities(uint32_t current_time) {
    for (uint32_t i = 0; i < queue.count; i++) {
        uint32_t age = current_time - queue.events[i].timestamp;
        
        if (age > MAX_EVENT_AGE_MS) {
            /* Boost priority based on age */
            uint32_t boost = (age - MAX_EVENT_AGE_MS) / 1000;  // +1 per second
            uint32_t new_pri = queue.events[i].base_priority + boost;
            
            if (new_pri > 255) new_pri = 255;
            queue.events[i].effective_priority = new_pri;
        }
    }
}

bool event_get(event_t *evt) {
    if (queue.count == 0) return false;
    
    /* Update priorities before selecting */
    event_update_priorities(sys_ms);
    
    /* Find highest EFFECTIVE priority */
    uint32_t max_idx = 0;
    for (uint32_t i = 1; i < queue.count; i++) {
        if (queue.events[i].effective_priority > 
            queue.events[max_idx].effective_priority) {
            max_idx = i;
        }
    }
    
    *evt = queue.events[max_idx];
    
    /* Remove */
    for (uint32_t i = max_idx; i < queue.count - 1; i++) {
        queue.events[i] = queue.events[i + 1];
    }
    queue.count--;
    
    return true;
}
```

**How it works:**
```
t=0s:    Low-priority event posted (pri=32)
t=1s:    High-priority events keep arriving (pri=128)
t=5s:    Low-priority event age > 5s → boost starts
t=6s:    Effective priority = 32 + 1 = 33
t=7s:    Effective priority = 32 + 2 = 34
...
t=101s:  Effective priority = 32 + 96 = 128 (equals high-priority!)
t=102s:  Low-priority event finally processed
```

**Benefits:**
- Prevents starvation
- Old events eventually get processed
- Maintains priority ordering for recent events
</details>

---

## 🎯 Exercise 5: Implement Event Coalescing (Hard - 30 min)

Multiple similar events can be combined into one (e.g., 10 temperature readings → 1 average).

**Task:** Implement event coalescing for temperature events.

<details>
<summary>Solution</summary>

```c
typedef struct {
    event_type_t type;
    uint32_t     data;
    uint32_t     timestamp;
    uint8_t      priority;
    uint16_t     sequence;
    
    /* Coalescing support */
    bool         coalescable;
    uint32_t     coalesce_count;  // How many events merged
    uint32_t     coalesce_sum;    // Sum for averaging
} event_t;

bool event_post_coalescable(event_type_t type, uint32_t data, 
                             uint8_t priority, bool coalescable) {
    if (queue.count >= EVENT_QUEUE_SIZE) {
        return false;
    }
    
    /* Check if we can coalesce with existing event */
    if (coalescable) {
        for (uint32_t i = 0; i < queue.count; i++) {
            if (queue.events[i].type == type && 
                queue.events[i].coalescable) {
                /* Found matching event — coalesce! */
                queue.events[i].coalesce_count++;
                queue.events[i].coalesce_sum += data;
                queue.events[i].data = queue.events[i].coalesce_sum / 
                                       queue.events[i].coalesce_count;
                queue.events[i].timestamp = sys_ms;  // Update timestamp
                return true;
            }
        }
    }
    
    /* No match — add new event */
    event_t *evt = &queue.events[queue.count++];
    evt->type           = type;
    evt->data           = data;
    evt->timestamp      = sys_ms;
    evt->priority       = priority;
    evt->sequence       = queue.sequence++;
    evt->coalescable    = coalescable;
    evt->coalesce_count = 1;
    evt->coalesce_sum   = data;
    
    return true;
}

// Usage:
void on_temperature_reading(int16_t temp) {
    event_post_coalescable(EVENT_TEMPERATURE, temp, 32, true);
    // Multiple readings coalesced into one averaged event
}

void on_button_press(uint8_t button_id) {
    event_post_coalescable(EVENT_BUTTON, button_id, 128, false);
    // Button presses NOT coalesced — each is unique
}
```

**Example:**
```
Post: Temp(22°C) → Queue: [Temp(22, count=1)]
Post: Temp(23°C) → Queue: [Temp(22.5, count=2)]  // Coalesced!
Post: Temp(24°C) → Queue: [Temp(23, count=3)]    // Coalesced!
Get:  Temp(23°C, count=3)  // Average of 3 readings
```

**Benefits:**
- Reduces queue usage
- Averages noisy sensor data
- Prevents queue overflow from high-rate sources
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Queue Sizing**
   - Calculate based on event rate and processing time
   - Add safety margin for bursts
   - Use power-of-2 sizes

2. **Priority Design**
   - Critical events highest priority
   - Leave gaps for future priorities
   - FIFO within same priority

3. **Event Filtering**
   - Selectively enable/disable event types
   - Useful for emergency modes
   - Bitmask for efficiency

4. **Starvation Prevention**
   - Age-based priority boost
   - Ensures old events eventually processed
   - Maintains fairness

5. **Event Coalescing**
   - Combine similar events
   - Reduces queue pressure
   - Useful for high-rate sensors

---

**Congratulations!** You've mastered Event Queues — the foundation of event-driven embedded systems!
