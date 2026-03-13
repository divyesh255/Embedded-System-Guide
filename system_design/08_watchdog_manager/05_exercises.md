# Watchdog Manager - Practice Exercises

Test your understanding of watchdog managers with these hands-on exercises!

---

## 🎯 Exercise 1: Calculate Timeout (Easy - 10 min)

A system has 3 tasks:
- Control loop: runs every 100ms, takes 50ms
- Safety check: runs every 50ms, takes 20ms
- Communication: runs every 500ms, takes 200ms

**Task:** Calculate appropriate watchdog timeout for each task.

<details>
<summary>Solution</summary>

```
Timeout = (execution period × 2) + safety margin

Control loop:
  Period: 100ms
  Timeout: (100ms × 2) + 50ms = 250ms

Safety check:
  Period: 50ms
  Timeout: (50ms × 2) + 25ms = 125ms

Communication:
  Period: 500ms
  Timeout: (500ms × 2) + 100ms = 1100ms
```

**Reasoning:**
- 2× period allows one missed cycle
- Safety margin accounts for jitter
- Critical tasks (safety) have shorter timeout

**Answer:**
- Control: 250ms
- Safety: 125ms
- Communication: 1100ms
</details>

---

## 🎯 Exercise 2: Implement Window Watchdog (Medium - 15 min)

Implement a window watchdog that requires kicks within a time window (not too early, not too late).

**Task:** Add min/max timeout checking.

<details>
<summary>Solution</summary>

```c
typedef struct {
    const char *name;
    uint32_t min_timeout_ms;  /* Kick not before this */
    uint32_t max_timeout_ms;  /* Kick not after this */
    uint32_t last_kick_ms;
    bool enabled;
} window_watchdog_task_t;

bool watchdog_kick(int task_id) {
    if (task_id < 0 || task_id >= num_tasks) return false;
    
    window_watchdog_task_t *task = &tasks[task_id];
    uint32_t elapsed = sys_ms - task->last_kick_ms;
    
    /* Check if too early */
    if (elapsed < task->min_timeout_ms) {
        printf("ERROR: Task '%s' kicked too early (%ums < %ums)\n",
               task->name, elapsed, task->min_timeout_ms);
        return false;
    }
    
    /* Check if too late */
    if (elapsed > task->max_timeout_ms) {
        printf("ERROR: Task '%s' kicked too late (%ums > %ums)\n",
               task->name, elapsed, task->max_timeout_ms);
        return false;
    }
    
    /* Valid kick */
    task->last_kick_ms = sys_ms;
    return true;
}

// Usage:
window_watchdog_task_t task = {
    .name = "control",
    .min_timeout_ms = 80,   /* Not before 80ms */
    .max_timeout_ms = 120,  /* Not after 120ms */
};

// Kick at 50ms  → ERROR (too early)
// Kick at 100ms → OK
// Kick at 150ms → ERROR (too late)
```

**Benefits:**
- Detects tasks running too fast (possible bug)
- Detects tasks running too slow (hang)
- More precise fault detection
</details>

---

## 🎯 Exercise 3: Add Fault Recovery Strategy (Medium - 20 min)

Implement different recovery strategies based on fault type.

**Task:** Add recovery actions (reset, disable task, safe mode).

<details>
<summary>Solution</summary>

```c
typedef enum {
    RECOVERY_RESET,      /* Reset system */
    RECOVERY_DISABLE,    /* Disable hung task */
    RECOVERY_SAFE_MODE   /* Enter safe mode */
} recovery_action_t;

typedef struct {
    const char *name;
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
    bool enabled;
    recovery_action_t recovery;
    uint32_t fault_count;
} watchdog_task_t;

void watchdog_check(void) {
    for (uint32_t i = 0; i < num_tasks; i++) {
        if (!tasks[i].enabled) continue;
        
        uint32_t elapsed = sys_ms - tasks[i].last_kick_ms;
        if (elapsed > tasks[i].timeout_ms) {
            tasks[i].fault_count++;
            
            printf("Task '%s' hung (fault #%u)\n",
                   tasks[i].name, tasks[i].fault_count);
            
            switch (tasks[i].recovery) {
                case RECOVERY_RESET:
                    printf("Action: System reset\n");
                    system_reset();
                    break;
                    
                case RECOVERY_DISABLE:
                    printf("Action: Disable task\n");
                    tasks[i].enabled = false;
                    /* Continue with other tasks */
                    break;
                    
                case RECOVERY_SAFE_MODE:
                    printf("Action: Enter safe mode\n");
                    enter_safe_mode();
                    break;
            }
        }
    }
}

// Usage:
watchdog_register("safety", 100, RECOVERY_RESET);      /* Critical */
watchdog_register("logging", 1000, RECOVERY_DISABLE);  /* Non-critical */
watchdog_register("control", 200, RECOVERY_SAFE_MODE); /* Degraded mode */
```

**Recovery strategies:**
- **RESET**: Critical tasks (safety, control)
- **DISABLE**: Non-critical tasks (logging, telemetry)
- **SAFE_MODE**: Degraded operation (reduced functionality)
</details>

---

## 🎯 Exercise 4: Implement Watchdog Chain (Hard - 25 min)

Create a chain of watchdogs where each level monitors the level below.

**Task:** Implement 3-level watchdog hierarchy.

<details>
<summary>Solution</summary>

```c
/* Level 1: Application tasks */
typedef struct {
    const char *name;
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
} app_task_t;

app_task_t app_tasks[MAX_APP_TASKS];

/* Level 2: Software watchdog (monitors app tasks) */
typedef struct {
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
} sw_watchdog_t;

sw_watchdog_t sw_watchdog = { .timeout_ms = 500 };

/* Level 3: Hardware watchdog (monitors software watchdog) */
#define HW_WATCHDOG_TIMEOUT_MS 1000

void app_task_kick(int task_id) {
    app_tasks[task_id].last_kick_ms = sys_ms;
}

void sw_watchdog_check(void) {
    /* Check all app tasks */
    bool all_alive = true;
    for (int i = 0; i < num_app_tasks; i++) {
        uint32_t elapsed = sys_ms - app_tasks[i].last_kick_ms;
        if (elapsed > app_tasks[i].timeout_ms) {
            printf("App task '%s' hung\n", app_tasks[i].name);
            all_alive = false;
            break;
        }
    }
    
    /* If all app tasks alive, kick software watchdog */
    if (all_alive) {
        sw_watchdog.last_kick_ms = sys_ms;
    }
}

void hw_watchdog_check(void) {
    /* Check software watchdog */
    uint32_t elapsed = sys_ms - sw_watchdog.last_kick_ms;
    if (elapsed > sw_watchdog.timeout_ms) {
        printf("Software watchdog hung\n");
        /* Don't kick hardware watchdog → system reset */
        return;
    }
    
    /* Software watchdog alive → kick hardware watchdog */
    hardware_watchdog_kick();
}

/* Main loop */
void main_loop(void) {
    while (1) {
        /* Level 1: App tasks run and kick */
        run_app_tasks();
        
        /* Level 2: Software watchdog checks app tasks */
        sw_watchdog_check();
        
        /* Level 3: Hardware watchdog checks software watchdog */
        hw_watchdog_check();
        
        delay_ms(10);
    }
}
```

**Hierarchy:**
```
Hardware Watchdog (1000ms)
    ↓ monitors
Software Watchdog (500ms)
    ↓ monitors
App Tasks (100-200ms each)
```

**Benefits:**
- Multi-level fault detection
- Software watchdog can log faults
- Hardware watchdog is final safety net
</details>

---

## 🎯 Exercise 5: Add Watchdog Test Mode (Hard - 30 min)

Implement a test mode that verifies watchdog functionality.

**Task:** Create self-test that intentionally triggers watchdog.

<details>
<summary>Solution</summary>

```c
typedef enum {
    TEST_NONE,
    TEST_TASK_HANG,
    TEST_SW_WATCHDOG_HANG,
    TEST_EARLY_KICK,
    TEST_LATE_KICK
} watchdog_test_t;

static watchdog_test_t test_mode = TEST_NONE;
static bool test_passed = false;

void watchdog_self_test(watchdog_test_t test) {
    printf("\n=== Watchdog Self-Test: %d ===\n", test);
    test_mode = test;
    test_passed = false;
    
    switch (test) {
        case TEST_TASK_HANG:
            printf("Test: Task hang detection\n");
            /* Don't kick task watchdog */
            for (int i = 0; i < 20; i++) {
                /* Task doesn't kick */
                watchdog_check();
                if (watchdog_fault_detected()) {
                    printf("✅ PASS: Hang detected\n");
                    test_passed = true;
                    break;
                }
                delay_ms(10);
            }
            break;
            
        case TEST_EARLY_KICK:
            printf("Test: Early kick detection\n");
            watchdog_kick(task_id);
            delay_ms(10);  /* Too early! */
            if (!watchdog_kick(task_id)) {
                printf("✅ PASS: Early kick rejected\n");
                test_passed = true;
            }
            break;
            
        case TEST_LATE_KICK:
            printf("Test: Late kick detection\n");
            delay_ms(timeout_ms + 100);  /* Too late! */
            watchdog_check();
            if (watchdog_fault_detected()) {
                printf("✅ PASS: Late kick detected\n");
                test_passed = true;
            }
            break;
    }
    
    test_mode = TEST_NONE;
    return test_passed;
}

/* Run all tests */
void watchdog_run_all_tests(void) {
    int passed = 0;
    int total = 3;
    
    if (watchdog_self_test(TEST_TASK_HANG)) passed++;
    if (watchdog_self_test(TEST_EARLY_KICK)) passed++;
    if (watchdog_self_test(TEST_LATE_KICK)) passed++;
    
    printf("\n=== Test Results: %d/%d passed ===\n", passed, total);
}
```

**Benefits:**
- Verifies watchdog works before deployment
- Catches configuration errors
- Required for safety certification
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Timeout Calculation**
   - 2× execution period + margin
   - Critical tasks have shorter timeouts
   - Account for worst-case jitter

2. **Window Watchdog**
   - Detects too-early kicks (bugs)
   - Detects too-late kicks (hangs)
   - More precise fault detection

3. **Recovery Strategies**
   - Reset for critical tasks
   - Disable for non-critical tasks
   - Safe mode for degraded operation

4. **Watchdog Hierarchy**
   - Multi-level monitoring
   - Software + hardware watchdogs
   - Defense in depth

5. **Self-Test**
   - Verify watchdog functionality
   - Required for certification
   - Catch configuration errors

---

**Congratulations!** You've mastered Watchdog Managers — essential for reliable embedded systems!
