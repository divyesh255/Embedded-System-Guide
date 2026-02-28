/**
 * 02_delay_bad.c - BAD: Blocking Delays for Multiple Tasks
 *
 * Simulates an IoT sensor node using blocking delay_ms().
 * Demonstrates all the failures:
 *   - Watchdog resets (delay > watchdog period)
 *   - Timing drift (processing time adds to delay)
 *   - Button lag (missed during delay)
 *   - Tasks interfere with each other
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * SIMULATED HARDWARE
 * ============================================================================ */

static uint32_t sys_ms = 0;

/* Blocking delay — CPU does NOTHING during this time */
static void delay_ms(uint32_t ms) {
    sys_ms += ms;
}

/* Simulated tasks with realistic durations */
static void toggle_led(void)     { sys_ms += 1;   }  /* 1ms  */
static void read_sensors(void)   { sys_ms += 10;  }  /* 10ms */
static void send_heartbeat(void) { sys_ms += 200; }  /* 200ms — network */
static void check_battery(void)  { sys_ms += 5;   }  /* 5ms  */
static void kick_watchdog(void)  { /* instant */  }

/* Simulated button press at t=250ms */
static bool button_was_pressed(void) {
    return (sys_ms >= 250 && sys_ms < 260);
}

/* ============================================================================
 * WATCHDOG SIMULATION
 * Must be kicked every 100ms or it fires (resets device)
 * ============================================================================ */

#define WATCHDOG_TIMEOUT_MS  100

static uint32_t last_watchdog_kick = 0;
static uint32_t watchdog_resets    = 0;

static void watchdog_check(void) {
    if (sys_ms - last_watchdog_kick > WATCHDOG_TIMEOUT_MS) {
        watchdog_resets++;
        printf("[WDT] *** WATCHDOG RESET *** at %ums (last kick: %ums, gap: %ums)\n",
               sys_ms, last_watchdog_kick, sys_ms - last_watchdog_kick);
        last_watchdog_kick = sys_ms; /* Reset after firing */
    }
}

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

static uint32_t led_toggles       = 0;
static uint32_t sensor_reads      = 0;
static uint32_t heartbeats_sent   = 0;
static uint32_t battery_checks    = 0;
static uint32_t button_presses    = 0;
static uint32_t button_missed     = 0;
static uint32_t timing_errors     = 0;

/* ============================================================================
 * BAD APPROACH: Blocking delay loop
 *
 * Problems demonstrated:
 * 1. delay_ms(500) blocks for 500ms — watchdog fires (needs 100ms)
 * 2. Processing time adds to delay — timing drifts
 * 3. Button press during delay is missed
 * 4. All tasks interfere with each other
 * ============================================================================ */

int main(void) {
    printf("=== BAD: Blocking Delays for Multiple Tasks ===\n\n");
    printf("Tasks:\n");
    printf("  LED blink:   every 500ms\n");
    printf("  Sensor read: every 1000ms\n");
    printf("  Heartbeat:   every 5000ms (shortened for demo)\n");
    printf("  Battery:     every 10000ms (shortened for demo)\n");
    printf("  Watchdog:    must kick every 100ms\n");
    printf("  Button:      pressed at t=250ms\n\n");

    uint32_t sim_end_ms       = 12000;
    uint32_t heartbeat_count  = 0;
    uint32_t battery_count    = 0;
    bool     button_handled   = false;

    printf("--- Simulation Start ---\n\n");

    while (sys_ms < sim_end_ms) {

        /* ----------------------------------------------------------------
         * Task 1: Blink LED
         * ---------------------------------------------------------------- */
        toggle_led();
        led_toggles++;
        printf("[LED] Toggle at %ums\n", sys_ms);

        /* ----------------------------------------------------------------
         * Task 2: BLOCKING DELAY — CPU does nothing for 500ms
         * Watchdog needs kick every 100ms → FIRES during this delay!
         * Button press at 250ms → MISSED during this delay!
         * ---------------------------------------------------------------- */
        uint32_t before_delay = sys_ms;
        delay_ms(500);  /* ← PROBLEM: blocks everything */
        watchdog_check(); /* Check AFTER delay — too late! */

        /* ----------------------------------------------------------------
         * Task 3: Read sensors (takes 10ms)
         * This adds to the loop time → timing drift!
         * ---------------------------------------------------------------- */
        uint32_t expected_sensor_time = before_delay + 500;
        if (sys_ms > expected_sensor_time + 5) {
            timing_errors++;
        }
        read_sensors();
        sensor_reads++;

        /* ----------------------------------------------------------------
         * Task 4: Heartbeat every ~5000ms
         * Counter-based: counts loop iterations × 500ms
         * But loop takes 500ms + 10ms + processing = MORE than 500ms!
         * ---------------------------------------------------------------- */
        heartbeat_count++;
        if (heartbeat_count >= 10) { /* 10 × ~510ms ≈ 5100ms (not 5000ms!) */
            uint32_t hb_time = sys_ms;
            send_heartbeat();
            heartbeats_sent++;
            printf("[NET] Heartbeat sent at %ums (expected ~%ums)\n",
                   hb_time, heartbeats_sent * 5000);
            heartbeat_count = 0;
        }

        /* ----------------------------------------------------------------
         * Task 5: Battery check every ~10000ms
         * ---------------------------------------------------------------- */
        battery_count++;
        if (battery_count >= 20) {
            check_battery();
            battery_checks++;
            printf("[BAT] Battery check at %ums\n", sys_ms);
            battery_count = 0;
        }

        /* ----------------------------------------------------------------
         * Task 6: Kick watchdog
         * Called here, but delay_ms(500) already caused watchdog to fire!
         * ---------------------------------------------------------------- */
        kick_watchdog();
        last_watchdog_kick = sys_ms;

        /* ----------------------------------------------------------------
         * Task 7: Check button
         * Button pressed at t=250ms — but we're inside delay_ms(500)!
         * We only check AFTER the delay → button press is MISSED
         * ---------------------------------------------------------------- */
        if (button_was_pressed() && !button_handled) {
            /* This code never runs during the delay */
            printf("[BTN] Button handled at %ums\n", sys_ms);
            button_presses++;
            button_handled = true;
        } else if (!button_handled && sys_ms > 260) {
            /* Button window passed — we missed it */
            printf("[BTN] *** BUTTON MISSED *** (pressed at 250ms, checked at %ums)\n",
                   sys_ms);
            button_missed++;
            button_handled = true; /* Don't report again */
        }
    }

    printf("\n--- Simulation End at %ums ---\n\n", sys_ms);

    /* ============================================================
     * RESULTS — Show all the failures
     * ============================================================ */
    printf("=== Results ===\n");
    printf("LED toggles:       %u (expected ~%u)\n",
           led_toggles, sim_end_ms / 500);
    printf("Sensor reads:      %u (expected ~%u)\n",
           sensor_reads, sim_end_ms / 1000);
    printf("Heartbeats sent:   %u (expected ~%u)\n",
           heartbeats_sent, sim_end_ms / 5000);
    printf("Battery checks:    %u (expected ~%u)\n",
           battery_checks, sim_end_ms / 10000);
    printf("Button presses:    %u\n", button_presses);
    printf("Button missed:     %u\n", button_missed);
    printf("Timing errors:     %u\n", timing_errors);
    printf("Watchdog resets:   %u\n", watchdog_resets);

    printf("\n=== Failures ===\n");
    if (watchdog_resets > 0)
        printf("❌ Watchdog fired %u times — device would have reset!\n",
               watchdog_resets);
    if (button_missed > 0)
        printf("❌ Button press missed — user gets no response!\n");
    if (timing_errors > 0)
        printf("❌ Timing drift — heartbeat arrives late at server!\n");

    printf("\n=== Root Cause ===\n");
    printf("delay_ms(500) blocks CPU for 500ms\n");
    printf("Watchdog needs kick every 100ms → 500ms >> 100ms → RESET\n");
    printf("Button pressed during delay → not checked → MISSED\n");
    printf("Processing time (10ms sensor + 200ms heartbeat) adds to\n");
    printf("loop time → timing drifts from expected values\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_timer_good.c — software timer manager\n");
    printf("No blocking delays. Each task runs independently.\n");

    return 0;
}

/*
 * PROBLEMS WITH BLOCKING DELAYS:
 *
 * 1. ❌ Watchdog starvation
 *    delay_ms(500) > watchdog_timeout(100ms) → device resets
 *
 * 2. ❌ Timing drift
 *    Loop time = delay + all task durations
 *    Expected: 500ms per iteration
 *    Actual:   500 + 10 + 200 (when heartbeat) = 710ms
 *    Heartbeat arrives 42% late!
 *
 * 3. ❌ Button missed
 *    Button pressed during delay_ms(500)
 *    Not checked until delay completes → up to 500ms lag
 *
 * 4. ❌ Tasks interfere
 *    send_heartbeat() takes 200ms → delays everything else
 *    No isolation between tasks
 *
 * 5. ❌ Doesn't scale
 *    Adding new task = more drift, more interference
 *    Counter math becomes unmaintainable
 */
