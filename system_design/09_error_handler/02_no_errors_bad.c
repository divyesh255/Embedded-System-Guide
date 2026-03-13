/**
 * 02_no_errors_bad.c - BAD: No Error Handling
 *
 * Simulates smart home hub without error handling.
 * Demonstrates:
 *   - Silent failures
 *   - No error context
 *   - No recovery
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

static uint32_t sys_ms = 0;
static bool simulate_sensor_fail = false;
static bool simulate_cloud_fail = false;

/* Simulated functions */
int read_temperature(void) {
    if (simulate_sensor_fail) {
        return 0;  /* ERROR! But caller doesn't know */
    }
    return 22;
}

void set_light(bool on) {
    printf("[%ums] Light: %s\n", sys_ms, on ? "ON" : "OFF");
}

void update_cloud(int temp) {
    if (simulate_cloud_fail) {
        return;  /* Silent failure! */
    }
    printf("[%ums] Cloud updated: temp=%d\n", sys_ms, temp);
}

int main(void) {
    printf("=== BAD: No Error Handling ===\n\n");
    printf("Simulating smart home hub without error handling\n\n");

    printf("--- Normal operation ---\n");
    for (int i = 0; i < 3; i++) {
        int temp = read_temperature();
        printf("[%ums] Temperature: %d°C\n", sys_ms, temp);
        set_light(temp > 25);
        update_cloud(temp);
        sys_ms += 1000;
    }

    printf("\n--- Simulating sensor failure ---\n");
    simulate_sensor_fail = true;
    for (int i = 0; i < 3; i++) {
        int temp = read_temperature();  /* Returns 0 on error */
        printf("[%ums] Temperature: %d°C (WRONG!)\n", sys_ms, temp);
        set_light(temp > 25);  /* Never turns on! */
        update_cloud(temp);
        sys_ms += 1000;
    }

    printf("\n--- Simulating cloud failure ---\n");
    simulate_sensor_fail = false;
    simulate_cloud_fail = true;
    for (int i = 0; i < 3; i++) {
        int temp = read_temperature();
        printf("[%ums] Temperature: %d°C\n", sys_ms, temp);
        set_light(temp > 25);
        update_cloud(temp);  /* Fails silently! */
        sys_ms += 1000;
    }

    printf("\n=== Results ===\n");
    printf("❌ Sensor failure: Silent (returned 0)\n");
    printf("❌ Cloud failure: Silent (no update)\n");
    printf("❌ No error messages\n");
    printf("❌ No error logging\n");
    printf("❌ User has no idea system is broken\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_error_handler_good.c — error handler\n");

    return 0;
}

/*
 * PROBLEMS WITHOUT ERROR HANDLING:
 *
 * 1. ❌ Silent failures
 *    Errors occur but no one knows
 *
 * 2. ❌ No error context
 *    Can't debug (what, where, when?)
 *
 * 3. ❌ No recovery
 *    System continues with bad data
 *
 * 4. ❌ No logging
 *    Can't diagnose field issues
 */
