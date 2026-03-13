# Watchdog Manager

**The pattern for detecting and recovering from system hangs**

---

## 🎯 What Problem Does This Solve?

Embedded systems can hang due to:
- Infinite loops
- Deadlocks
- Hardware faults
- Software bugs

```c
// WRONG: No watchdog protection
while (1) {
    process_data();  // If this hangs, system freezes forever!
}
```

**The naive solution — no watchdog — leads to:**
- System hangs requiring manual reset
- Lost data and corrupted state
- Customer complaints and returns
- Safety hazards in critical systems

**The solution: Watchdog Manager**

Periodically "kick" watchdog → if system hangs, watchdog resets system automatically.

---

## 🔧 How It Works

### Hardware Watchdog

A timer that resets the system if not refreshed:

```
System running:
  t=0ms:   Kick watchdog (reset timer to 1000ms)
  t=500ms: Kick watchdog (reset timer to 1000ms)
  t=900ms: Kick watchdog (reset timer to 1000ms)
  System keeps running...

System hangs:
  t=0ms:   Kick watchdog (reset timer to 1000ms)
  t=500ms: System hangs! No more kicks
  t=1000ms: Watchdog expires → SYSTEM RESET
```

### Software Watchdog Manager

Coordinates multiple tasks to ensure all are alive:

```c
typedef struct {
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
    bool     enabled;
} watchdog_task_t;

watchdog_task_t tasks[MAX_TASKS];

void watchdog_manager(void) {
    // Check all tasks
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].enabled) {
            uint32_t elapsed = sys_ms - tasks[i].last_kick_ms;
            if (elapsed > tasks[i].timeout_ms) {
                // Task hung! Reset system
                system_reset();
            }
        }
    }
    
    // All tasks alive → kick hardware watchdog
    hardware_watchdog_kick();
}
```

---

## 📐 Watchdog Types

### Hardware Watchdog
Built into MCU. Resets system if not kicked.

```
Timeout: 1000ms
If not kicked within 1000ms → RESET
```

### Software Watchdog
Monitors multiple tasks. Kicks hardware watchdog only if all tasks alive.

```
Task A timeout: 500ms
Task B timeout: 1000ms
Task C timeout: 2000ms

All alive → kick hardware watchdog
Any hung → don't kick → hardware watchdog resets system
```

### Window Watchdog
Must be kicked within a time window (not too early, not too late).

```
Window: 100-900ms
Kick at 50ms  → RESET (too early)
Kick at 500ms → OK
Kick at 950ms → RESET (too late)
```

---

## ⚙️ Key Design Decisions

### 1. Timeout Period
```c
#define WATCHDOG_TIMEOUT_MS  1000  // Must be > longest task execution
```
- Too short → false resets
- Too long → slow fault detection
- Typical: 100ms - 5000ms

### 2. Task Registration
```c
int watchdog_register(const char *name, uint32_t timeout_ms) {
    int id = find_free_slot();
    tasks[id].name = name;
    tasks[id].timeout_ms = timeout_ms;
    tasks[id].enabled = true;
    return id;
}
```

### 3. Task Kick
```c
void watchdog_kick(int task_id) {
    tasks[task_id].last_kick_ms = sys_ms;
}
```

### 4. Manager Loop
```c
void watchdog_manager(void) {
    // Check all tasks
    bool all_alive = true;
    for (int i = 0; i < num_tasks; i++) {
        if (task_is_hung(&tasks[i])) {
            all_alive = false;
            log_fault(tasks[i].name);
            break;
        }
    }
    
    // Kick hardware watchdog only if all tasks alive
    if (all_alive) {
        hardware_watchdog_kick();
    }
}
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│              Application Tasks              │
│  Task A    Task B    Task C    Task D       │
└──────┬───────┬────────┬─────────┬───────────┘
       │       │        │         │
       │ kick  │ kick   │ kick    │ kick
       ▼       ▼        ▼         ▼
┌─────────────────────────────────────────────┐
│          Software Watchdog Manager          │
│  - Monitor all tasks                        │
│  - Detect hung tasks                        │
│  - Log faults                               │
└──────────────────┬──────────────────────────┘
                   │ kick (if all alive)
┌──────────────────▼──────────────────────────┐
│          Hardware Watchdog Timer            │
│  - Countdown from timeout                   │
│  - Reset system if expires                  │
└─────────────────────────────────────────────┘
```

---

## 📊 Comparison

| Approach | Fault Detection | Recovery | Complexity |
|----------|----------------|----------|------------|
| No watchdog | ❌ None | ❌ Manual | Simple |
| Hardware only | ⚠️ Basic | ✅ Auto | Simple |
| **Software + Hardware** | ✅ Per-task | ✅ Auto | Medium |
| RTOS watchdog | ✅ Per-thread | ✅ Auto | High |

---

## 🔑 Key Takeaways

1. **Fault Detection** — automatically detect system hangs
2. **Auto Recovery** — reset system without manual intervention
3. **Per-Task Monitoring** — identify which task hung
4. **Graceful Degradation** — log fault before reset
5. **Safety Critical** — required for medical, automotive, industrial

---

## 🎯 Use Cases

**Medical Devices:**
- Infusion pumps (IEC 62304)
- Patient monitors
- Ventilators
- Defibrillators

**Automotive:**
- ECUs (ISO 26262)
- ADAS systems
- Engine control
- Brake systems

**Industrial:**
- PLCs (IEC 61508)
- Motor controllers
- Safety systems
- Process control

**IoT:**
- Remote sensors
- Smart meters
- Gateway devices
- Edge computing

---

**Ready to see the problem?** → `01_problem.md`
