/**
 * 02_polling_bad.c - BAD EXAMPLE: Polling Approach
 *
 * This shows WHY polling fails for the server room monitor.
 *
 * Requirements:
 *   - Sample 4 sensors every 10ms (100 Hz)
 *   - Log every sample to SD card (50ms per write)
 *   - Update LCD display (30ms)
 *   - Send alert if temp > 80°C
 *   - NEVER miss a sample
 *
 * Problem:
 *   Loop takes 81ms but samples must arrive every 10ms.
 *   Result: 87.5% data loss.
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/* ============================================================================
 * SIMULATED HARDWARE
 * ============================================================================ */

static uint32_t sys_ms = 0;          /* Simulated system time */
static uint32_t timer_last_fire = 0; /* When timer last fired */

/* Advance simulated time */
void advance_time(uint32_t ms) {
    sys_ms += ms;
}

/* Check if 10ms timer has expired */
bool timer_expired(void) {
    if (sys_ms - timer_last_fire >= 10) {
        timer_last_fire = sys_ms;
        return true;
    }
    return false;
}

/* Simulated sensor read — 1ms */
typedef struct {
    uint16_t temp[4];
    uint32_t timestamp_ms;
} sample_t;

sample_t read_sensors(void) {
    advance_time(1); /* 1ms to read 4 sensors */
    sample_t s;
    s.temp[0] = 65 + (sys_ms % 20);
    s.temp[1] = 70 + (sys_ms % 15);
    s.temp[2] = 60 + (sys_ms % 25);
    s.temp[3] = 75 + (sys_ms % 10);
    s.timestamp_ms = sys_ms;
    return s;
}

/* Simulated SD write — 50ms */
void write_to_sd(sample_t s) {
    advance_time(50); /* 50ms to write */
    printf("[SD]  Wrote sample @%ums: T0=%u T1=%u T2=%u T3=%u\n",
           s.timestamp_ms, s.temp[0], s.temp[1], s.temp[2], s.temp[3]);
}

/* Simulated display update — 30ms */
void update_display(sample_t s) {
    advance_time(30); /* 30ms to update */
    printf("[LCD] Display @%ums: T0=%u T1=%u T2=%u T3=%u\n",
           s.timestamp_ms, s.temp[0], s.temp[1], s.temp[2], s.temp[3]);
}

/* Simulated network alert — 100ms */
void send_alert(sample_t s, int sensor) {
    advance_time(100);
    printf("[NET] ALERT! Sensor %d = %u°C @%ums\n",
           sensor, s.temp[sensor], s.timestamp_ms);
}

/* ============================================================================
 * POLLING APPROACH — THE WRONG WAY
 * ============================================================================ */

int main(void) {
    printf("=== BAD EXAMPLE: Polling Approach ===\n\n");
    printf("Requirements:\n");
    printf("  Sample every 10ms (100 Hz)\n");
    printf("  SD write: 50ms\n");
    printf("  Display:  30ms\n");
    printf("  Total loop: 81ms\n\n");

    uint32_t samples_required = 0; /* How many samples SHOULD have been taken */
    uint32_t samples_taken    = 0; /* How many samples ACTUALLY taken */
    uint32_t sim_end_ms       = 500; /* Simulate 500ms */

    printf("--- Simulation Start ---\n");

    while (sys_ms < sim_end_ms) {

        /* Count how many samples SHOULD have fired by now */
        uint32_t expected = sys_ms / 10;
        if (expected > samples_required) {
            uint32_t missed = expected - samples_required - 1;
            if (missed > 0) {
                printf("[!!!] MISSED %u samples between %ums and %ums!\n",
                       missed,
                       samples_required * 10,
                       sys_ms);
            }
            samples_required = expected;
        }

        /* Check timer */
        if (timer_expired()) {
            samples_taken++;

            /* Read sensors — 1ms */
            sample_t s = read_sensors();
            printf("[READ] Sample %u @%ums\n", samples_taken, s.timestamp_ms);

            /* Write to SD — 50ms (BLOCKS HERE!) */
            write_to_sd(s);
            /* ↑ Timer fires 5 times during this 50ms — all MISSED! */

            /* Update display — 30ms (BLOCKS HERE!) */
            update_display(s);
            /* ↑ Timer fires 3 times during this 30ms — all MISSED! */

            /* Check alert */
            for (int i = 0; i < 4; i++) {
                if (s.temp[i] > 80) {
                    send_alert(s, i);
                    /* ↑ Timer fires 10 times during this 100ms — all MISSED! */
                }
            }
        } else {
            /* Timer not expired — advance time slightly */
            advance_time(1);
        }
    }

    printf("\n--- Simulation End ---\n\n");

    /* Calculate actual data loss */
    uint32_t total_required = sim_end_ms / 10;
    uint32_t missed = total_required - samples_taken;
    float loss_pct = (missed * 100.0f) / total_required;

    printf("=== Results ===\n");
    printf("Simulation time:   %ums\n", sim_end_ms);
    printf("Samples required:  %u (every 10ms)\n", total_required);
    printf("Samples taken:     %u\n", samples_taken);
    printf("Samples missed:    %u\n", missed);
    printf("Data loss:         %.1f%%\n\n", loss_pct);

    printf("=== Why Polling Fails ===\n");
    printf("Loop time breakdown:\n");
    printf("  Read sensors:  1ms\n");
    printf("  SD write:     50ms  ← 5 samples missed here!\n");
    printf("  Display:      30ms  ← 3 samples missed here!\n");
    printf("  Total:        81ms\n\n");
    printf("Required interval: 10ms\n");
    printf("Actual interval:   81ms\n");
    printf("Data loss:         87.5%%\n\n");

    printf("=== Problems ===\n");
    printf("1. ❌ Timing wrong: 81ms instead of 10ms\n");
    printf("2. ❌ 87.5%% data loss\n");
    printf("3. ❌ Compliance audit fails\n");
    printf("4. ❌ Overheating detected 81ms late\n");
    printf("5. ❌ Cannot fix without interrupts\n");

    return 0;
}

/*
 * KEY LESSON:
 *
 * Polling cannot meet real-time requirements when:
 *   Loop time > Sample interval
 *   81ms      > 10ms  → FAILS
 *
 * The only fix is interrupts:
 *   ISR samples at exact 10ms (hardware-guaranteed)
 *   Main loop processes at its own pace
 *   Circular buffer bridges the gap
 *
 * See 03_interrupt_good.c for the correct solution.
 */
