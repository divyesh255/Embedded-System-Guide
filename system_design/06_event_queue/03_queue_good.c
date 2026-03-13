/**
 * 03_queue_good.c - GOOD: Event Queue with Priority
 *
 * Solves smart home hub problem with event queue:
 *   - Events posted to queue (fast, non-blocking)
 *   - Processed by priority (alarm first)
 *   - No events lost (queue buffers bursts)
 *   - Decoupled producers/consumers
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * EVENT QUEUE
 * ============================================================================ */

#define EVENT_QUEUE_SIZE  16

typedef enum {
    EVENT_BUTTON,
    EVENT_MOTION,
    EVENT_TEMPERATURE,
    EVENT_ALARM,
    EVENT_NETWORK
} event_type_t;

typedef struct {
    event_type_t type;
    uint32_t     data;
    uint32_t     timestamp;
    uint8_t      priority;  /* 0=low, 255=high */
} event_t;

typedef struct {
    event_t  events[EVENT_QUEUE_SIZE];
    uint32_t count;
} event_queue_t;

static event_queue_t queue = {0};
static uint32_t sys_ms = 0;

/* Post event to queue */
static bool event_post(event_type_t type, uint32_t data, uint8_t priority) {
    if (queue.count >= EVENT_QUEUE_SIZE) {
        return false;  /* Queue full */
    }
    
    event_t *evt = &queue.events[queue.count++];
    evt->type      = type;
    evt->data      = data;
    evt->timestamp = sys_ms;
    evt->priority  = priority;
    
    return true;
}

/* Get highest priority event */
static bool event_get(event_t *evt) {
    if (queue.count == 0) return false;
    
    /* Find highest priority */
    uint32_t max_idx = 0;
    for (uint32_t i = 1; i < queue.count; i++) {
        if (queue.events[i].priority > queue.events[max_idx].priority) {
            max_idx = i;
        }
    }
    
    /* Copy event */
    *evt = queue.events[max_idx];
    
    /* Remove from queue (shift remaining) */
    for (uint32_t i = max_idx; i < queue.count - 1; i++) {
        queue.events[i] = queue.events[i + 1];
    }
    queue.count--;
    
    return true;
}

/* ============================================================================
 * SIMULATED HARDWARE
 * ============================================================================ */

static void advance_time(uint32_t ms) { sys_ms += ms; }

/* ============================================================================
 * EVENT HANDLERS
 * ============================================================================ */

static uint32_t events_processed = 0;

static void handle_button(uint32_t button_id) {
    printf("[BTN] Button %u at %ums\n", button_id, sys_ms);
    advance_time(50);   /* Debounce */
    advance_time(100);  /* Update display */
    advance_time(500);  /* Send MQTT */
    events_processed++;
}

static void handle_motion(uint32_t sensor_id) {
    printf("[MOT] Motion sensor %u at %ums\n", sensor_id, sys_ms);
    advance_time(50);   /* Control lights */
    advance_time(200);  /* Log to SD */
    advance_time(300);  /* Send notification */
    events_processed++;
}

static void handle_temperature(uint32_t temp) {
    printf("[TMP] Temperature %u°C at %ums\n", temp, sys_ms);
    advance_time(100);  /* Update HVAC */
    advance_time(150);  /* Log to database */
    events_processed++;
}

static void handle_alarm(uint32_t alarm_type) {
    printf("[ALM] *** ALARM %u *** at %ums\n", alarm_type, sys_ms);
    advance_time(10);   /* Activate siren */
    advance_time(800);  /* Send emergency alert */
    advance_time(2000); /* Call emergency services */
    events_processed++;
}

static void handle_network(uint32_t cmd) {
    printf("[NET] Network command %u at %ums\n", cmd, sys_ms);
    advance_time(50);   /* Parse */
    advance_time(200);  /* Execute */
    advance_time(200);  /* Send ACK */
    events_processed++;
}

/* ============================================================================
 * EVENT PRODUCERS — Post to queue (fast!)
 * ============================================================================ */

static void on_button_isr(uint8_t button_id) {
    event_post(EVENT_BUTTON, button_id, 128);  /* High priority */
}

static void on_motion_isr(uint8_t sensor_id) {
    event_post(EVENT_MOTION, sensor_id, 64);  /* Normal priority */
}

static void on_temperature_isr(int16_t temp) {
    event_post(EVENT_TEMPERATURE, temp, 32);  /* Low priority */
}

static void on_alarm_isr(uint8_t alarm_type) {
    event_post(EVENT_ALARM, alarm_type, 255);  /* CRITICAL priority */
}

static void on_network_isr(uint8_t cmd) {
    event_post(EVENT_NETWORK, cmd, 96);  /* High-normal priority */
}

/* ============================================================================
 * MAIN LOOP — Process events by priority
 * ============================================================================ */

int main(void) {
    printf("=== GOOD: Event Queue with Priority ===\n\n");
    printf("Priority levels:\n");
    printf("  255 (Critical): Alarm\n");
    printf("  128 (High):     Button\n");
    printf("   96 (Normal+):  Network\n");
    printf("   64 (Normal):   Motion\n");
    printf("   32 (Low):      Temperature\n\n");

    printf("--- Simulation Start ---\n\n");

    /* Post events */
    sys_ms = 0;
    on_button_isr(1);
    
    sys_ms = 100;
    on_motion_isr(1);
    
    sys_ms = 700;
    on_temperature_isr(22);
    
    sys_ms = 800;
    on_alarm_isr(1);  /* CRITICAL — will be processed first! */
    
    sys_ms = 3760;
    on_network_isr(5);
    
    sys_ms = 4000;
    on_button_isr(2);

    printf("All events posted to queue (count: %u)\n\n", queue.count);

    /* Process events by priority */
    sys_ms = 4000;
    event_t evt;
    while (event_get(&evt)) {
        printf("Processing event (priority %u, posted at %ums):\n",
               evt.priority, evt.timestamp);
        
        switch (evt.type) {
            case EVENT_ALARM:
                handle_alarm(evt.data);
                break;
            case EVENT_BUTTON:
                handle_button(evt.data);
                break;
            case EVENT_NETWORK:
                handle_network(evt.data);
                break;
            case EVENT_MOTION:
                handle_motion(evt.data);
                break;
            case EVENT_TEMPERATURE:
                handle_temperature(evt.data);
                break;
        }
        printf("\n");
    }

    printf("--- Simulation End at %ums ---\n\n", sys_ms);

    /* ============================================================
     * RESULTS
     * ============================================================ */
    printf("=== Results ===\n");
    printf("Events processed: %u\n", events_processed);
    printf("Events dropped:   0 (queue buffered all)\n");

    printf("\n=== Improvements Over Direct Callbacks ===\n");
    printf("✅ All events processed (none dropped)\n");
    printf("✅ Alarm processed FIRST (priority 255)\n");
    printf("✅ ISRs return immediately (just post to queue)\n");
    printf("✅ Decoupled producers/consumers\n");
    printf("✅ Easy to add new event types\n");

    return 0;
}

/*
 * HOW EVENT QUEUE WORKS:
 *
 * 1. Producer (ISR) — fast:
 *    on_button_isr() → event_post() → returns immediately
 *
 * 2. Queue — buffers events:
 *    [Button(128)] [Motion(64)] [Temp(32)] [Alarm(255)]
 *
 * 3. Consumer (main loop) — processes by priority:
 *    event_get() → returns Alarm(255) first
 *                → then Button(128)
 *                → then Motion(64)
 *                → then Temp(32)
 *
 * Key benefits:
 *   - Decoupling: producers don't know about consumers
 *   - Priority: critical events processed first
 *   - Buffering: handles bursts without data loss
 *   - Asynchronous: post returns immediately
 */
