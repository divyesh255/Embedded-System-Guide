/**
 * 02_callback_bad.c - BAD: Direct Callback Execution
 *
 * Simulates smart home hub using direct callbacks.
 * Demonstrates failures:
 *   - Events block each other
 *   - No priority (alarm delayed by temperature)
 *   - Events lost during processing
 *   - Poor responsiveness
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
static void advance_time(uint32_t ms) { sys_ms += ms; }

/* ============================================================================
 * EVENT TYPES
 * ============================================================================ */

typedef enum {
    EVENT_BUTTON,
    EVENT_MOTION,
    EVENT_TEMPERATURE,
    EVENT_ALARM,
    EVENT_NETWORK
} event_type_t;

/* ============================================================================
 * STATISTICS
 * ============================================================================ */

static uint32_t events_processed = 0;
static uint32_t events_dropped   = 0;
static uint32_t alarm_delay_ms   = 0;
static uint32_t button_delay_ms  = 0;

/* ============================================================================
 * DIRECT CALLBACK HANDLERS — Block everything!
 * ============================================================================ */

static bool handler_busy = false;

static void handle_button(uint8_t button_id) {
    uint32_t start = sys_ms;
    printf("[BTN] Button %d at %ums\n", button_id, sys_ms);
    advance_time(50);   /* Debounce */
    advance_time(100);  /* Update display */
    advance_time(500);  /* Send MQTT */
    events_processed++;
    button_delay_ms = sys_ms - start;
}

static void handle_motion(uint8_t sensor_id) {
    printf("[MOT] Motion sensor %d at %ums\n", sensor_id, sys_ms);
    advance_time(50);   /* Control lights */
    advance_time(200);  /* Log to SD */
    advance_time(300);  /* Send notification */
    events_processed++;
}

static void handle_temperature(int16_t temp) {
    printf("[TMP] Temperature %d°C at %ums\n", temp, sys_ms);
    advance_time(100);  /* Update HVAC */
    advance_time(150);  /* Log to database */
    events_processed++;
}

static void handle_alarm(uint8_t alarm_type) {
    uint32_t start = sys_ms;
    printf("[ALM] *** ALARM %d *** at %ums\n", alarm_type, sys_ms);
    advance_time(10);   /* Activate siren */
    advance_time(800);  /* Send emergency alert */
    advance_time(2000); /* Call emergency services */
    events_processed++;
    alarm_delay_ms = sys_ms - start;
}

static void handle_network(uint8_t cmd) {
    printf("[NET] Network command %d at %ums\n", cmd, sys_ms);
    advance_time(50);   /* Parse */
    advance_time(200);  /* Execute */
    advance_time(200);  /* Send ACK */
    events_processed++;
}

/* ============================================================================
 * EVENT PRODUCERS — Call handlers directly (BAD!)
 * ============================================================================ */

static void on_button_event(uint8_t button_id) {
    if (handler_busy) {
        printf("[BTN] *** DROPPED *** (handler busy) at %ums\n", sys_ms);
        events_dropped++;
        return;
    }
    handler_busy = true;
    handle_button(button_id);
    handler_busy = false;
}

static void on_motion_event(uint8_t sensor_id) {
    if (handler_busy) {
        printf("[MOT] *** DROPPED *** (handler busy) at %ums\n", sys_ms);
        events_dropped++;
        return;
    }
    handler_busy = true;
    handle_motion(sensor_id);
    handler_busy = false;
}

static void on_temperature_event(int16_t temp) {
    if (handler_busy) {
        printf("[TMP] *** DROPPED *** (handler busy) at %ums\n", sys_ms);
        events_dropped++;
        return;
    }
    handler_busy = true;
    handle_temperature(temp);
    handler_busy = false;
}

static void on_alarm_event(uint8_t alarm_type) {
    if (handler_busy) {
        printf("[ALM] *** CRITICAL ALARM DELAYED *** (handler busy) at %ums\n", sys_ms);
        /* Alarm must wait! */
    }
    handler_busy = true;
    handle_alarm(alarm_type);
    handler_busy = false;
}

static void on_network_event(uint8_t cmd) {
    if (handler_busy) {
        printf("[NET] *** DROPPED *** (handler busy) at %ums\n", sys_ms);
        events_dropped++;
        return;
    }
    handler_busy = true;
    handle_network(cmd);
    handler_busy = false;
}

/* ============================================================================
 * SIMULATION
 * ============================================================================ */

int main(void) {
    printf("=== BAD: Direct Callback Execution ===\n\n");
    printf("Simulating smart home hub with direct callbacks\n");
    printf("Events: Button, Motion, Temperature, Alarm, Network\n\n");

    printf("--- Simulation Start ---\n\n");

    /* t=0ms: Button press */
    on_button_event(1);

    /* t=650ms: Motion detected (during button processing) */
    sys_ms = 100;
    on_motion_event(1);  /* DROPPED — button still processing */

    /* t=700ms: Temperature reading */
    sys_ms = 700;
    on_temperature_event(22);

    /* t=950ms: CRITICAL ALARM (during temperature processing) */
    sys_ms = 800;
    on_alarm_event(1);  /* DELAYED — temperature still processing */

    /* t=3760ms: Network command */
    sys_ms = 3760;
    on_network_event(5);

    /* t=4210ms: Another button press */
    sys_ms = 4000;
    on_button_event(2);  /* DROPPED — network still processing */

    printf("\n--- Simulation End at %ums ---\n\n", sys_ms);

    /* ============================================================
     * RESULTS
     * ============================================================ */
    printf("=== Results ===\n");
    printf("Events processed: %u\n", events_processed);
    printf("Events dropped:   %u\n", events_dropped);
    printf("Button delay:     %ums (target: <100ms)\n", button_delay_ms);
    printf("Alarm delay:      %ums (target: immediate!)\n", alarm_delay_ms);

    printf("\n=== Failures ===\n");
    if (events_dropped > 0)
        printf("❌ %u events dropped — data loss!\n", events_dropped);
    if (button_delay_ms > 100)
        printf("❌ Button delay %ums — poor UX!\n", button_delay_ms);
    if (alarm_delay_ms > 100)
        printf("❌ Alarm delay %ums — SAFETY RISK!\n", alarm_delay_ms);

    printf("\n=== Root Cause ===\n");
    printf("Direct callback execution blocks all other events\n");
    printf("No priority — alarm waits for temperature\n");
    printf("No buffering — events dropped when handler busy\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_queue_good.c — event queue with priority\n");

    return 0;
}

/*
 * PROBLEMS WITH DIRECT CALLBACKS:
 *
 * 1. ❌ Blocking
 *    handle_button() takes 650ms → all other events wait
 *
 * 2. ❌ No priority
 *    Temperature (250ms) blocks CRITICAL alarm
 *    Alarm should preempt low-priority events
 *
 * 3. ❌ Events dropped
 *    If handler busy → event lost
 *    No buffering for bursts
 *
 * 4. ❌ Poor responsiveness
 *    Button press can wait up to 2810ms (if alarm processing)
 *    User expects < 100ms
 *
 * 5. ❌ Coupling
 *    Event producer directly calls handler
 *    Hard to test, change, or extend
 */
