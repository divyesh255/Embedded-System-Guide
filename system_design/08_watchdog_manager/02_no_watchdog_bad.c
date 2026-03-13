/**
 * 02_no_watchdog_bad.c - BAD: No Watchdog Protection
 *
 * Simulates robot controller without watchdog.
 * Demonstrates:
 *   - System hangs indefinitely
 *   - No auto recovery
 *   - No fault detection
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static uint32_t sys_ms = 0;
static bool simulate_hang = false;

/* Simulated tasks */
void control_task(void) {
    printf("[%ums] Control task running\n", sys_ms);
    
    if (simulate_hang && sys_ms > 500) {
        printf("[%ums] Control task HUNG (infinite loop)!\n", sys_ms);
        while (1) {
            /* Infinite loop - system frozen! */
        }
    }
    
    sys_ms += 100;
}

void safety_task(void) {
    printf("[%ums] Safety task running\n", sys_ms);
    sys_ms += 50;
}

void comm_task(void) {
    printf("[%ums] Communication task running\n", sys_ms);
    sys_ms += 200;
}

int main(void) {
    printf("=== BAD: No Watchdog Protection ===\n\n");
    printf("Simulating robot controller without watchdog\n\n");

    printf("--- Normal operation (first 500ms) ---\n");
    simulate_hang = false;
    
    for (int i = 0; i < 5; i++) {
        control_task();
        safety_task();
        comm_task();
    }

    printf("\n--- Simulating hang at 500ms ---\n");
    simulate_hang = true;
    
    printf("Control task will hang...\n");
    control_task();  /* This will hang forever! */
    
    /* Never reached */
    printf("This line never executes!\n");
    safety_task();
    comm_task();

    printf("\n=== Results ===\n");
    printf("❌ System hung at %ums\n", sys_ms);
    printf("❌ No auto recovery\n");
    printf("❌ Manual reset required\n");
    printf("❌ Safety tasks not running\n");
    printf("❌ Production stopped\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_watchdog_good.c — watchdog manager\n");

    return 0;
}

/*
 * PROBLEMS WITHOUT WATCHDOG:
 *
 * 1. ❌ No fault detection
 *    System hangs → no one knows
 *
 * 2. ❌ No auto recovery
 *    Hangs forever → manual reset required
 *
 * 3. ❌ Safety risk
 *    Safety tasks stop → potential hazard
 *
 * 4. ❌ Downtime
 *    Hours to days until manual intervention
 */
