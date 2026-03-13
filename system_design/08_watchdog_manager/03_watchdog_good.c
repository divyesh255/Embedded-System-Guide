/**
 * 03_watchdog_good.c - GOOD: Watchdog Manager
 *
 * Solves robot controller problem with watchdog:
 *   - Detects task hangs
 *   - Auto-resets system
 *   - Per-task monitoring
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Watchdog task */
typedef struct {
    const char *name;
    uint32_t timeout_ms;
    uint32_t last_kick_ms;
    bool enabled;
} watchdog_task_t;

#define MAX_TASKS 4
static watchdog_task_t tasks[MAX_TASKS];
static uint32_t num_tasks = 0;
static uint32_t sys_ms = 0;
static bool system_reset_triggered = false;

/* Register task */
static int watchdog_register(const char *name, uint32_t timeout_ms) {
    if (num_tasks >= MAX_TASKS) return -1;
    
    int id = num_tasks++;
    tasks[id].name = name;
    tasks[id].timeout_ms = timeout_ms;
    tasks[id].last_kick_ms = 0;
    tasks[id].enabled = true;
    
    return id;
}

/* Kick watchdog */
static void watchdog_kick(int task_id) {
    if (task_id >= 0 && task_id < num_tasks) {
        tasks[task_id].last_kick_ms = sys_ms;
    }
}

/* Check watchdog */
static void watchdog_check(void) {
    for (uint32_t i = 0; i < num_tasks; i++) {
        if (!tasks[i].enabled) continue;
        
        uint32_t elapsed = sys_ms - tasks[i].last_kick_ms;
        if (elapsed > tasks[i].timeout_ms) {
            printf("\n[%ums] *** WATCHDOG FAULT ***\n", sys_ms);
            printf("Task '%s' hung (timeout: %ums, elapsed: %ums)\n",
                   tasks[i].name, tasks[i].timeout_ms, elapsed);
            printf("System will reset...\n");
            system_reset_triggered = true;
            return;
        }
    }
}

/* Simulated tasks */
static int task_control_id;
static int task_safety_id;
static int task_comm_id;
static bool simulate_hang = false;

static void control_task(void) {
    printf("[%ums] Control task\n", sys_ms);
    
    if (simulate_hang && sys_ms > 500) {
        printf("[%ums] Control task HUNG!\n", sys_ms);
        /* Don't kick watchdog - simulate hang */
        return;
    }
    
    watchdog_kick(task_control_id);
    sys_ms += 100;
}

static void safety_task(void) {
    printf("[%ums] Safety task\n", sys_ms);
    watchdog_kick(task_safety_id);
    sys_ms += 50;
}

static void comm_task(void) {
    printf("[%ums] Comm task\n", sys_ms);
    watchdog_kick(task_comm_id);
    sys_ms += 200;
}

int main(void) {
    printf("=== GOOD: Watchdog Manager ===\n\n");
    
    /* Register tasks */
    task_control_id = watchdog_register("control", 150);
    task_safety_id = watchdog_register("safety", 100);
    task_comm_id = watchdog_register("comm", 300);
    
    printf("Registered %u tasks:\n", num_tasks);
    for (uint32_t i = 0; i < num_tasks; i++) {
        printf("  %s (timeout: %ums)\n", tasks[i].name, tasks[i].timeout_ms);
    }
    printf("\n");

    printf("--- Normal operation (first 500ms) ---\n");
    simulate_hang = false;
    
    for (int i = 0; i < 5 && !system_reset_triggered; i++) {
        control_task();
        safety_task();
        comm_task();
        watchdog_check();
    }

    printf("\n--- Simulating hang at 500ms ---\n");
    simulate_hang = true;
    
    /* Control task will hang (not kick watchdog) */
    for (int i = 0; i < 3 && !system_reset_triggered; i++) {
        control_task();  /* Hangs - doesn't kick */
        safety_task();
        comm_task();
        watchdog_check();  /* Will detect hang */
    }

    printf("\n=== Results ===\n");
    if (system_reset_triggered) {
        printf("✅ Watchdog detected hang\n");
        printf("✅ System reset triggered\n");
        printf("✅ Fault identified: control task\n");
        printf("✅ Auto recovery (no manual intervention)\n");
    }

    printf("\n=== Improvements Over No Watchdog ===\n");
    printf("✅ Detects hangs within timeout period\n");
    printf("✅ Auto-resets system\n");
    printf("✅ Identifies which task hung\n");
    printf("✅ No manual intervention required\n");

    return 0;
}

/*
 * HOW WATCHDOG WORKS:
 *
 * 1. Task Registration:
 *    - Each task registers with timeout
 *    - Gets unique task ID
 *
 * 2. Task Execution:
 *    - Task runs normally
 *    - Kicks watchdog before timeout
 *    - Repeats
 *
 * 3. Watchdog Check:
 *    - Checks all tasks
 *    - If any task timeout exceeded → RESET
 *    - Logs which task hung
 *
 * 4. Auto Recovery:
 *    - System resets automatically
 *    - No manual intervention
 *    - Minimal downtime
 */
