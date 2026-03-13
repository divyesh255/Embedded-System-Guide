/**
 * 02_always_on_bad.c - BAD: Always-On Power
 *
 * Simulates wireless sensor without power management.
 * Demonstrates:
 *   - Always-on CPU (50mA)
 *   - Short battery life (2.5 days)
 *   - Wasted power
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdint.h>

#define BATTERY_CAPACITY_MAH 3000
#define ACTIVE_CURRENT_MA 50

static uint32_t time_hours = 0;
static uint32_t battery_mah = BATTERY_CAPACITY_MAH;

int main(void) {
    printf("=== BAD: Always-On Power ===\n\n");
    printf("Battery: %umAh\n", BATTERY_CAPACITY_MAH);
    printf("Current: %umA (always-on)\n\n", ACTIVE_CURRENT_MA);

    printf("--- Simulating battery drain ---\n");
    
    while (battery_mah > 0) {
        /* Simulate 1 hour */
        battery_mah -= ACTIVE_CURRENT_MA;
        time_hours++;
        
        if (time_hours % 24 == 0) {
            printf("Day %u: Battery %umAh remaining\n", 
                   time_hours / 24, battery_mah);
        }
    }

    printf("\n=== Results ===\n");
    printf("Battery life: %u hours = %.1f days\n", 
           time_hours, time_hours / 24.0);
    printf("Target: 2 years (17,520 hours)\n");
    printf("Actual: %.1f days (2.5 days)\n", time_hours / 24.0);
    printf("Gap: %.0f× worse than target!\n", 17520.0 / time_hours);

    printf("\n=== Problems ===\n");
    printf("❌ CPU always running (50mA)\n");
    printf("❌ No sleep modes\n");
    printf("❌ Battery dies in 2.5 days\n");
    printf("❌ Product not viable\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_power_manager_good.c — power management\n");

    return 0;
}

/*
 * PROBLEMS WITH ALWAYS-ON:
 *
 * 1. ❌ Wasted power
 *    CPU running even when idle
 *
 * 2. ❌ Short battery life
 *    60 hours instead of 17,520 hours
 *
 * 3. ❌ High maintenance cost
 *    292 battery changes per year
 *
 * 4. ❌ Product not viable
 *    $14.6M/year maintenance cost
 */
