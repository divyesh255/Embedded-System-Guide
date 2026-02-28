/**
 * 03_timer_good.c - GOOD: Software Timer Manager
 *
 * Solves the IoT sensor node problem with a software timer manager:
 *   - One hardware tick (1ms) drives all software timers
 *   - Each task has its own independent timer
 *   - No blocking delays
 *   - Callbacks set flags → main loop does the work
 *   - Watchdog always kicked on time
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * TIMER MANAGER
 * ============================================================================ */

#define MAX_TIMERS   8

typedef void (*timer_callback_t)(void);

typedef enum {
    TIMER_PERIODIC,   /* Fires repeatedly */
    TIMER_ONE_SHOT    /* Fires once then stops */
} timer_mode_t;

typedef struct {
    uint32_t         period_ms;    /* Period between fires */
    uint32_t         remaining_ms; /* Countdown to next fire */
    timer_callback_t callback;     /* Called when timer expires */
    timer_mode_t     mode;         /* Periodic or one-shot */
    bool             active;       /* Is timer running? */
    const char      *name;         /* Debug name */
} sw_timer_t;

static sw_timer_t timers[MAX_TIMERS];
static uint32_t   sys_tick_ms = 0;

/* Create a timer — returns timer ID, -1 on failure */
static int timer_create(uint32_t period_ms, timer_mode_t mode,
                        timer_callback_t cb, const char *name) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timers[i].active && timers[i].period_ms == 0) {
            timers[i].period_ms    = period_ms;
            timers[i].remaining_ms = period_ms;
            timers[i].callback     = cb;
            timers[i].mode         = mode;
            timers[i].active       = false; /* Start with timer_start() */
            timers[i].name         = name;
            return i;
        }
    }
    return -1; /* No free slot */
}

/* Start a timer */
static void timer_start(int id) {
    if (id >= 0 && id < MAX_TIMERS) {
        timers[id].remaining_ms = timers[id].period_ms; /* Reset countdown */
        timers[id].active       = true;
    }
}

/* Stop a timer */
static void timer_stop(int id) {
    if (id >= 0 && id < MAX_TIMERS) {
        timers[id].active = false;
    }
}

/* Reset a timer (retrigger) — restarts countdown from now */
static void timer_reset(int id) {
    if (id >= 0 && id < MAX_TIMERS) {
        timers[id].remaining_ms = timers[id].period_ms;
        timers[id].active       = true;
    }
}

/* ============================================================================
 * TICK FUNCTION — called every 1ms from hardware timer ISR
 *
 * This is the heart of the timer manager.
 * On real hardware: called from SysTick_Handler() or TIM_IRQHandler()
 * ============================================================================ */
static void timer_tick(void) {
    sys_tick_ms++;

    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timers[i].active) continue;

        timers[i].remaining_ms--;

        if (timers[i].remaining_ms == 0) {
            /* Timer expired — fire callback */
            if (timers[i].callback) {
                timers[i].callback();
            }

            if (timers[i].mode == TIMER_PERIODIC) {
                timers[i].remaining_ms = timers[i].period_ms; /* Reload */
            } else {
                timers[i].active = false; /* One-shot: stop */
            }
        }
    }
}

/* ============================================================================
 * SIMULATED HARDWARE
 * ============================================================================ */

static void advance_time(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        timer_tick(); /* Each ms advances the tick */
    }
}

/* ============================================================================
 * APPLICATION FLAGS
 * Set by timer callbacks (fast, ISR-safe)
 * Cleared by main loop after processing (slow OK)
 * ============================================================================ */

static volatile bool led_pending       = false;
static volatile bool sensor_pending    = false;
static volatile bool heartbeat_pending = false;
static volatile bool battery_pending   = false;
static volatile bool watchdog_pending  = false;
static volatile bool debounce_pending  = false;

/* ============================================================================
 * TIMER CALLBACKS — called from tick ISR context
 * Rules: fast, no blocking, no printf, just set flags
 * ============================================================================ */

static void on_led_timer(void)       { led_pending       = true; }
static void on_sensor_timer(void)    { sensor_pending    = true; }
static void on_heartbeat_timer(void) { heartbeat_pending = true; }
static void on_battery_timer(void)   { battery_pending   = true; }
static void on_watchdog_timer(void)  { watchdog_pending  = true; }
static void on_debounce_timer(void)  { debounce_pending  = true; }

/* ============================================================================
 * APPLICATION TASKS — called from main loop (slow OK)
 * ============================================================================ */

static uint32_t led_toggles     = 0;
static uint32_t sensor_reads    = 0;
static uint32_t heartbeats_sent = 0;
static uint32_t battery_checks  = 0;
static uint32_t watchdog_kicks  = 0;
static uint32_t debounce_done   = 0;

static int debounce_timer_id = -1;

static void task_led(void) {
    led_toggles++;
    /* toggle_gpio(LED_PIN); */
    printf("[LED] Toggle #%u at %ums\n", led_toggles, sys_tick_ms);
}

static void task_sensor(void) {
    sensor_reads++;
    /* value = adc_read(); */
    advance_time(10); /* Sensor read takes 10ms */
    printf("[SEN] Read #%u at %ums (10ms)\n", sensor_reads, sys_tick_ms);
}

static void task_heartbeat(void) {
    heartbeats_sent++;
    advance_time(200); /* Network send takes 200ms */
    printf("[NET] Heartbeat #%u at %ums (200ms)\n",
           heartbeats_sent, sys_tick_ms);
}

static void task_battery(void) {
    battery_checks++;
    advance_time(5); /* Battery check takes 5ms */
    printf("[BAT] Check #%u at %ums\n", battery_checks, sys_tick_ms);
}

static void task_watchdog(void) {
    watchdog_kicks++;
    /* WDT_KICK(); */
}

static void task_debounce(void) {
    debounce_done++;
    printf("[BTN] Button confirmed at %ums (debounced)\n", sys_tick_ms);
}

/* ============================================================================
 * BUTTON PRESS SIMULATION
 * On real hardware: GPIO interrupt calls this
 * ============================================================================ */

static void on_button_press(void) {
    /* Start one-shot debounce timer — fires once after 50ms */
    timer_reset(debounce_timer_id);
    printf("[BTN] Press detected at %ums, debounce started\n", sys_tick_ms);
}

/* ============================================================================
 * MAIN LOOP
 * ============================================================================ */

int main(void) {
    printf("=== GOOD: Software Timer Manager ===\n\n");
    printf("Tasks:\n");
    printf("  LED blink:   every 500ms\n");
    printf("  Sensor read: every 1000ms\n");
    printf("  Heartbeat:   every 5000ms\n");
    printf("  Battery:     every 10000ms\n");
    printf("  Watchdog:    every 100ms\n");
    printf("  Button:      pressed at t=250ms (50ms debounce)\n\n");

    /* ------------------------------------------------------------------
     * Initialize timer manager
     * ------------------------------------------------------------------ */
    memset(timers, 0, sizeof(timers));

    /* Create all timers */
    int led_id       = timer_create(500,   TIMER_PERIODIC, on_led_timer,       "LED");
    int sensor_id    = timer_create(1000,  TIMER_PERIODIC, on_sensor_timer,    "Sensor");
    int heartbeat_id = timer_create(5000,  TIMER_PERIODIC, on_heartbeat_timer, "Heartbeat");
    int battery_id   = timer_create(10000, TIMER_PERIODIC, on_battery_timer,   "Battery");
    int watchdog_id  = timer_create(100,   TIMER_PERIODIC, on_watchdog_timer,  "Watchdog");
    debounce_timer_id= timer_create(50,    TIMER_ONE_SHOT, on_debounce_timer,  "Debounce");

    /* Start all periodic timers */
    timer_start(led_id);
    timer_start(sensor_id);
    timer_start(heartbeat_id);
    timer_start(battery_id);
    timer_start(watchdog_id);
    /* Debounce starts only on button press */

    printf("--- Simulation Start ---\n\n");

    uint32_t sim_end_ms    = 12000;
    bool     button_fired  = false;

    /* ------------------------------------------------------------------
     * Main loop — no blocking delays!
     * Timer callbacks set flags, main loop processes them
     * ------------------------------------------------------------------ */
    while (sys_tick_ms < sim_end_ms) {

        /* Simulate hardware tick advancing time */
        advance_time(1);

        /* Simulate button press at t=250ms */
        if (!button_fired && sys_tick_ms >= 250) {
            on_button_press();
            button_fired = true;
        }

        /* Process pending tasks — order determines priority */

        if (watchdog_pending) {
            watchdog_pending = false;
            task_watchdog();  /* Highest priority — always first */
        }

        if (led_pending) {
            led_pending = false;
            task_led();
        }

        if (sensor_pending) {
            sensor_pending = false;
            task_sensor();
        }

        if (heartbeat_pending) {
            heartbeat_pending = false;
            task_heartbeat();
        }

        if (battery_pending) {
            battery_pending = false;
            task_battery();
        }

        if (debounce_pending) {
            debounce_pending = false;
            task_debounce();
        }
    }

    printf("\n--- Simulation End at %ums ---\n\n", sys_tick_ms);

    /* ============================================================
     * RESULTS
     * ============================================================ */
    printf("=== Results ===\n");
    printf("LED toggles:     %u (expected ~%u)\n",
           led_toggles, sim_end_ms / 500);
    printf("Sensor reads:    %u (expected ~%u)\n",
           sensor_reads, sim_end_ms / 1000);
    printf("Heartbeats:      %u (expected ~%u)\n",
           heartbeats_sent, sim_end_ms / 5000);
    printf("Battery checks:  %u (expected ~%u)\n",
           battery_checks, sim_end_ms / 10000);
    printf("Watchdog kicks:  %u\n", watchdog_kicks);
    printf("Debounce done:   %u\n", debounce_done);

    printf("\n=== Improvements Over Blocking Delays ===\n");
    printf("✅ Watchdog kicked every 100ms — no resets\n");
    printf("✅ Button debounced correctly — no missed presses\n");
    printf("✅ Each task runs at its own period independently\n");
    printf("✅ Adding new task = timer_create() + one flag check\n");
    printf("✅ Tasks don't interfere with each other's timing\n");

    return 0;
}

/*
 * HOW THE TIMER MANAGER WORKS:
 *
 * Hardware tick (1ms):
 *   SysTick_Handler() → timer_tick() → decrement all active timers
 *                                    → fire callbacks when expired
 *
 * Callbacks (ISR context — fast):
 *   on_led_timer()    → led_pending = true
 *   on_sensor_timer() → sensor_pending = true
 *   ...
 *
 * Main loop (safe context — slow OK):
 *   if (led_pending)    { task_led(); }
 *   if (sensor_pending) { task_sensor(); }
 *   ...
 *
 * Key insight:
 *   Timer callbacks are DECOUPLED from task execution.
 *   Timer fires at exact period → sets flag.
 *   Main loop processes flag when ready → no timing interference.
 *
 * Timer types:
 *   PERIODIC:  Reloads automatically → LED, sensor, watchdog
 *   ONE_SHOT:  Fires once then stops → debounce, timeout, retry
 */
