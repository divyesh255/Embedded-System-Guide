# Problem: Smart Home Hub — Event Overload

## 📋 The Scenario

You're building a **smart home hub** that controls lights, sensors, and appliances. The hub must handle events from multiple sources simultaneously:

### Requirements
1. **Button presses** — 4 physical buttons (lights, fan, alarm, mode)
2. **Motion sensors** — 3 PIR sensors (living room, bedroom, hallway)
3. **Temperature sensors** — 2 sensors (indoor, outdoor)
4. **Network commands** — WiFi/MQTT from phone app
5. **Timer events** — scheduled actions (lights on/off at specific times)
6. **Alarm triggers** — smoke detector, door sensor

### Business Constraints
- **Responsiveness:** Button press must feel instant (< 100ms)
- **Reliability:** Alarm events MUST be processed immediately
- **Cost:** Low-cost MCU (limited RAM, no RTOS)
- **Scalability:** Easy to add new event sources

## 🤔 Your First Attempt

You start with direct callback execution:

```c
// smart_home.c - First attempt with direct callbacks

#include <stdint.h>
#include <stdbool.h>

/* Direct callback handlers */

void on_button_press(uint8_t button_id) {
    printf("Button %d pressed\n", button_id);
    
    // Debounce
    delay_ms(50);
    
    // Update display
    update_display(button_id);  // 100ms
    
    // Send to cloud
    send_mqtt_update(button_id);  // 500ms
    
    // Total: 650ms blocked!
}

void on_motion_detected(uint8_t sensor_id) {
    printf("Motion in room %d\n", sensor_id);
    
    // Turn on lights
    control_lights(sensor_id, true);  // 50ms
    
    // Log event
    log_to_sd_card(sensor_id);  // 200ms
    
    // Send notification
    send_push_notification(sensor_id);  // 300ms
    
    // Total: 550ms blocked!
}

void on_alarm_triggered(uint8_t alarm_type) {
    printf("ALARM: %d\n", alarm_type);
    
    // Sound siren
    activate_siren();  // 10ms
    
    // Send emergency notification
    send_emergency_alert(alarm_type);  // 800ms
    
    // Call emergency services (if configured)
    make_emergency_call();  // 2000ms
    
    // Total: 2810ms blocked!
}

void on_temperature_reading(uint8_t sensor_id, int16_t temp) {
    // Process temperature
    update_hvac_control(temp);  // 100ms
    
    // Log to database
    log_temperature(sensor_id, temp);  // 150ms
}

void on_network_command(uint8_t cmd, uint32_t data) {
    // Parse command
    parse_mqtt_command(cmd, data);  // 50ms
    
    // Execute action
    execute_command(cmd, data);  // Variable, 10-500ms
    
    // Send acknowledgment
    send_mqtt_ack(cmd);  // 200ms
}
```

## 😱 The Problems Start

### Week 1: Button Feels Sluggish

**Symptom:** User presses button, nothing happens for 1-2 seconds.

**Root cause:**
```
t=0ms:    User presses button
t=0ms:    on_button_press() starts
t=0ms:    Motion sensor fires → on_motion_detected() WAITS
t=650ms:  on_button_press() completes
t=650ms:  on_motion_detected() starts
t=1200ms: on_motion_detected() completes
t=1200ms: Button finally processed!
```

Button callback blocks motion callback. User experiences 1.2 second lag!

### Week 2: Alarm Delayed

**Symptom:** Smoke alarm triggered, but siren didn't sound for 3 seconds.

**Root cause:**
```
t=0ms:    Network command arrives → on_network_command() starts (500ms)
t=200ms:  SMOKE ALARM TRIGGERED! → on_alarm_triggered() WAITS
t=500ms:  on_network_command() completes
t=500ms:  on_alarm_triggered() starts
t=3310ms: Siren finally sounds

3.3 second delay for CRITICAL alarm!
```

### Week 3: Events Lost

**Symptom:** Some button presses completely ignored.

**Root cause:**
```
Button ISR:
  if (button_handler_busy) {
      return;  // Drop event!
  }
  button_handler_busy = true;
  on_button_press(button_id);  // 650ms
  button_handler_busy = false;

If user presses button twice within 650ms → second press LOST!
```

### Week 4: The Math Doesn't Work

**Event arrival rates:**
```
Buttons:      ~1 per second (user interaction)
Motion:       ~5 per minute (people moving)
Temperature:  Every 10 seconds
Network:      ~10 per minute (app commands)
Timers:       ~5 per minute (scheduled actions)
Alarms:       Rare, but CRITICAL

Total: ~30 events per minute average
Burst: Up to 10 events per second
```

**Processing times:**
```
Button:       650ms
Motion:       550ms
Temperature:  250ms
Network:      Variable, 50-700ms
Alarm:        2810ms (CRITICAL!)
```

**Problem:** If events arrive faster than they can be processed → queue builds up → delays increase → events lost.

### Week 5: Priority Inversion

**Symptom:** Low-priority temperature reading blocks high-priority alarm.

```
t=0ms:    Temperature reading starts (250ms)
t=50ms:   ALARM TRIGGERED! → Must wait for temperature to finish
t=250ms:  Temperature completes
t=250ms:  Alarm finally starts

Low-priority event blocked high-priority event!
```

## 💭 Think About It (5 minutes)

Before looking at the solution, ask yourself:

1. **What's the core problem?**
   - Why do events block each other?
   - How can we process events without blocking?

2. **What about priorities?**
   - Alarm should be processed before temperature
   - How to implement priority?

3. **What about bursts?**
   - 10 events arrive in 1 second
   - How to handle without losing events?

## 🎯 The Core Problems

### Problem 1: Synchronous Execution
```c
void on_button_press(uint8_t id) {
    // Executes immediately, blocks everything
    send_mqtt_update(id);  // 500ms
}
// Other events wait 500ms!
```

### Problem 2: No Priority
```c
// All events treated equally
on_temperature_reading();  // 250ms, low priority
on_alarm_triggered();      // Must wait! High priority!
```

### Problem 3: No Buffering
```c
// If handler is busy, event is lost
if (busy) return;  // DROP EVENT!
```

### Problem 4: Coupling
```c
// Event producer knows about consumer
void on_button_isr(void) {
    on_button_press(button_id);  // Direct call
}
// Hard to change, test, or extend
```

## 📊 Impact Analysis

| Issue | Effect | Severity |
|-------|--------|----------|
| Button lag | Poor UX, feels broken | High |
| Alarm delay | Safety risk, liability | Critical |
| Events lost | Missed commands, data loss | High |
| No priority | Critical events delayed | Critical |
| Coupling | Hard to maintain, test | Medium |

**Total cost:** Product recall risk, safety liability, poor reviews.

## 💡 The Solution Preview

What if events were queued instead of executed immediately?

```c
// Event queue — decouple producers from consumers

typedef struct {
    event_type_t type;
    uint32_t     data;
    uint8_t      priority;  // 0=low, 255=high
} event_t;

// Producer (ISR) — just posts event, returns immediately
void on_button_isr(uint8_t button_id) {
    event_t evt = {
        .type = EVENT_BUTTON_PRESS,
        .data = button_id,
        .priority = 128  // High priority
    };
    event_post(&evt);  // Fast! Just adds to queue
}

void on_alarm_isr(uint8_t alarm_type) {
    event_t evt = {
        .type = EVENT_ALARM,
        .data = alarm_type,
        .priority = 255  // CRITICAL priority
    };
    event_post(&evt);  // Alarm jumps to front of queue!
}

// Consumer (main loop) — processes events by priority
void main_loop(void) {
    event_t evt;
    while (event_get(&evt)) {  // Gets highest priority event
        switch (evt.type) {
            case EVENT_ALARM:
                handle_alarm(evt.data);  // Processed FIRST
                break;
            case EVENT_BUTTON_PRESS:
                handle_button(evt.data);
                break;
            case EVENT_TEMPERATURE:
                handle_temperature(evt.data);  // Processed LAST
                break;
        }
    }
}
```

**Benefits:**
- ✅ ISRs return immediately (< 1ms)
- ✅ Events processed by priority (alarm first)
- ✅ No events lost (queue buffers bursts)
- ✅ Decoupled (easy to add new events)

## 🚀 Next Steps

1. **02_callback_bad.c** — Direct callback simulation (shows all failures)
2. **03_queue_good.c** — Event queue solution
3. **04_production.c** — Production-grade with priority queue

---

**Key Takeaway:** Direct callback execution blocks other events and has no priority. An event queue decouples producers from consumers, buffers bursts, and processes events by priority.
