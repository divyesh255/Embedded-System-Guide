# Problem: Industrial Robot Controller — System Hangs

## 📋 The Scenario

You're building an **industrial robot controller** for a manufacturing line. The controller must:

### Requirements
1. **Control robot arm** (position, speed, torque)
2. **Monitor safety sensors** (collision detection, emergency stop)
3. **Communicate with PLC** (receive commands, send status)
4. **Log operations** to SD card
5. **Run 24/7** without manual intervention

### Business Constraints
- **Safety:** Must detect faults within 100ms (IEC 61508)
- **Reliability:** < 1 unplanned stop per month
- **Cost:** Downtime costs $10,000/hour
- **Compliance:** Safety certification required

## 🤔 Your First Attempt

You start without watchdog protection:

```c
// robot_controller.c - First attempt without watchdog

void main_loop(void) {
    while (1) {
        // Read sensors
        read_position_sensors();
        read_safety_sensors();
        
        // Control robot
        calculate_trajectory();
        update_motor_control();
        
        // Communication
        process_plc_commands();
        send_status_to_plc();
        
        // Logging
        log_to_sd_card();
        
        delay_ms(10);  // 100Hz control loop
    }
}
```

## 😱 The Problems Start

### Week 1: First Hang

**Symptom:** Robot froze mid-operation. Required power cycle to recover.

**Root cause:** Infinite loop in trajectory calculation:

```c
void calculate_trajectory(void) {
    while (distance_to_target() > 0.1) {
        // BUG: target unreachable, infinite loop!
        move_towards_target();
    }
}
```

**Impact:** Production line stopped for 2 hours. Cost: $20,000.

### Week 2: Deadlock

**Symptom:** System hung during PLC communication.

**Root cause:** Deadlock between two tasks:

```c
// Task A
mutex_lock(&sensor_mutex);
mutex_lock(&comm_mutex);  // Waits forever

// Task B
mutex_lock(&comm_mutex);
mutex_lock(&sensor_mutex);  // Waits forever
```

**Impact:** Overnight production run failed. Cost: $80,000.

### Week 3: Hardware Fault

**Symptom:** System hung after power glitch.

**Root cause:** MCU entered undefined state, no recovery mechanism.

```
Power glitch → MCU registers corrupted → Program counter invalid
→ System hung → Manual reset required
```

**Impact:** Weekend production lost. Cost: $160,000.

### Month 2: The Math Doesn't Work

**Failure rate:**
```
Hangs per month: 4
Downtime per hang: 2 hours average
Cost per hour: $10,000
Monthly cost: 4 × 2 × $10,000 = $80,000

Annual cost: $960,000 in downtime!
```

**Customer complaints:**
- "Robot freezes randomly"
- "Requires constant monitoring"
- "Not suitable for lights-out manufacturing"

### Month 3: Safety Incident

**Symptom:** Robot continued moving after emergency stop pressed.

**Root cause:** Safety monitoring task hung, emergency stop not processed.

```c
void safety_monitor(void) {
    while (1) {
        if (emergency_stop_pressed()) {
            stop_robot();  // Never reached due to hang!
        }
        
        // BUG: Hung in sensor reading
        read_safety_sensors();  // Infinite loop
    }
}
```

**Impact:** Near-miss safety incident. Regulatory investigation. Production halted.

## 💭 Think About It (5 minutes)

1. **Why do systems hang?**
   - Infinite loops
   - Deadlocks
   - Hardware faults
   - Software bugs

2. **How to detect hangs?**
   - Watchdog timer
   - Task monitoring
   - Heartbeat signals

3. **How to recover?**
   - Automatic reset
   - Graceful degradation
   - Fault logging

## 🎯 The Core Problems

### Problem 1: No Fault Detection
```c
while (1) {
    process();  // If hangs, no detection!
}
```

### Problem 2: No Auto Recovery
```
System hangs → waits forever → manual reset required
Downtime: hours to days
```

### Problem 3: No Fault Logging
```
System resets → no record of what failed
Can't diagnose or prevent recurrence
```

### Problem 4: Safety Risk
```
Safety task hangs → emergency stop not processed
Potential injury or equipment damage
```

## 📊 Impact Analysis

| Issue | Effect | Severity |
|-------|--------|----------|
| System hangs | Production stops | Critical |
| No auto recovery | Manual intervention required | High |
| No fault logging | Can't diagnose issues | High |
| Safety risk | Potential injury | Critical |
| Downtime cost | $960K/year | Critical |

## 💡 The Solution Preview

What if system could detect and recover from hangs automatically?

```c
// Watchdog manager — detect hangs and auto-reset

typedef struct {
    const char *name;
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
} watchdog_task_t;

watchdog_task_t tasks[] = {
    { "control",  100 },  // Must kick every 100ms
    { "safety",   50 },   // Must kick every 50ms (critical!)
    { "comm",     500 },  // Must kick every 500ms
    { "logging",  1000 }  // Must kick every 1000ms
};

void watchdog_manager(void) {
    // Check all tasks
    for (int i = 0; i < num_tasks; i++) {
        uint32_t elapsed = sys_ms - tasks[i].last_kick_ms;
        if (elapsed > tasks[i].timeout_ms) {
            // Task hung! Log and reset
            log_fault(tasks[i].name);
            system_reset();
        }
    }
    
    // All tasks alive → kick hardware watchdog
    hardware_watchdog_kick();
}

// Each task kicks watchdog
void control_task(void) {
    while (1) {
        calculate_trajectory();
        update_motors();
        watchdog_kick(TASK_CONTROL);  // I'm alive!
        delay_ms(10);
    }
}

void safety_task(void) {
    while (1) {
        read_safety_sensors();
        check_emergency_stop();
        watchdog_kick(TASK_SAFETY);  // I'm alive!
        delay_ms(5);
    }
}
```

**Benefits:**
- ✅ Detects hangs within timeout period (50-1000ms)
- ✅ Auto-resets system (no manual intervention)
- ✅ Logs fault before reset (for diagnosis)
- ✅ Per-task monitoring (identifies which task hung)
- ✅ Safety compliant (IEC 61508, ISO 26262)

## 🚀 Next Steps

1. **02_no_watchdog_bad.c** — No watchdog simulation (shows hang)
2. **03_watchdog_good.c** — Watchdog manager solution
3. **04_production.c** — Production-grade with fault logging

---

**Key Takeaway:** Without watchdog protection, systems hang indefinitely requiring manual reset. Watchdog manager detects hangs and auto-recovers, ensuring high reliability and safety compliance.
