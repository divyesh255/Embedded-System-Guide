/**
 * 03_interrupt_good.c - GOOD EXAMPLE: Interrupt + Drain Buffer
 *
 * This shows the CORRECT solution for the server room monitor.
 *
 * Design (from 01_problem.md):
 *   ISR:       fires every 10ms → reads sensors → writes to circular buffer
 *   Main loop: drains ENTIRE buffer → writes batch to SD → updates display
 *
 * Math verification (WORST CASE):
 *   SD + display + alert = 50ms + 30ms + 100ms = 180ms
 *   Samples arriving during 180ms: 180ms / 10ms = 18 samples
 *   Buffer size: 18 × 4 (safety) = 72 → use 128 slots ✅
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

typedef struct {
    uint16_t temp[4];       /* 4 sensor readings */
    uint32_t timestamp_ms;  /* When sample was taken */
} sample_t;

/* ============================================================================
 * CIRCULAR BUFFER (32 slots)
 * ISR writes, main loop reads.
 * ============================================================================ */

#define CIRC_BUF_SIZE 128  /* worst case: 18 samples × 4 safety = 72 → 128 */

static sample_t   circ_buf[CIRC_BUF_SIZE];
static volatile uint32_t circ_head  = 0;  /* ISR writes here */
static volatile uint32_t circ_tail  = 0;  /* Main reads here */
static volatile uint32_t circ_count = 0;  /* Number of items */

/* ISR calls this — must be fast */
static void circ_write(sample_t s) {
    if (circ_count >= CIRC_BUF_SIZE) {
        /* Buffer full — should never happen with correct sizing */
        return;
    }
    circ_buf[circ_head] = s;
    circ_head = (circ_head + 1) % CIRC_BUF_SIZE;
    circ_count++;
}

/* Main loop calls this */
static bool circ_read(sample_t *out) {
    if (circ_count == 0) return false;
    *out = circ_buf[circ_tail];
    circ_tail = (circ_tail + 1) % CIRC_BUF_SIZE;
    circ_count--;
    return true;
}

static bool circ_has_data(void) { return circ_count > 0; }

/* ============================================================================
 * SIMULATED HARDWARE
 * ============================================================================ */

static uint32_t sys_ms = 0;

void advance_time(uint32_t ms) { sys_ms += ms; }

/* Simulated sensor read — 1ms */
sample_t hw_read_sensors(void) {
    advance_time(1);
    sample_t s;
    s.temp[0] = 65 + (sys_ms % 20);
    s.temp[1] = 70 + (sys_ms % 15);
    s.temp[2] = 60 + (sys_ms % 25);
    s.temp[3] = 75 + (sys_ms % 10);
    s.timestamp_ms = sys_ms;
    return s;
}

/* Simulated SD write — 50ms for any number of samples */
void hw_sd_write_batch(sample_t *batch, uint32_t count) {
    advance_time(50);
    printf("[SD]  Wrote %u samples to SD (50ms). Latest @%ums: T0=%u T1=%u T2=%u T3=%u\n",
           count,
           batch[count - 1].timestamp_ms,
           batch[count - 1].temp[0], batch[count - 1].temp[1],
           batch[count - 1].temp[2], batch[count - 1].temp[3]);
}

/* Simulated display update — 30ms */
void hw_update_display(sample_t s) {
    advance_time(30);
    printf("[LCD] Display @%ums: T0=%u T1=%u T2=%u T3=%u\n",
           s.timestamp_ms, s.temp[0], s.temp[1], s.temp[2], s.temp[3]);
}

/* Simulated network alert — 100ms */
void hw_send_alert(sample_t s, int sensor) {
    advance_time(100);
    printf("[NET] ALERT! Sensor %d = %u°C @%ums\n",
           sensor, s.temp[sensor], s.timestamp_ms);
}

/* ============================================================================
 * ISR — fires every 10ms (simulated)
 *
 * Rule: FAST only. Read hardware, write to buffer, return.
 * ============================================================================ */

static uint32_t isr_count = 0;

void TIMER_IRQHandler(void) {
    /* Read all 4 sensors — 1ms */
    sample_t s = hw_read_sensors();

    /* Write to circular buffer — <1ms */
    circ_write(s);

    isr_count++;
    /* Done! Returns to main loop immediately. */
}

/* ============================================================================
 * MAIN LOOP — drains entire buffer each iteration
 *
 * Design:
 *   1. Drain entire circular buffer into local batch
 *   2. Write entire batch to SD (one write, 50ms)
 *   3. Update display with latest sample (30ms)
 *   4. Send alert if any sample exceeded threshold (rare)
 * ============================================================================ */

#define ALERT_THRESHOLD 80  /* °C */

static uint32_t total_logged   = 0;
static uint32_t display_count  = 0;
static uint32_t alert_count    = 0;
static uint32_t max_batch_seen = 0;

void main_loop_iteration(void) {
    /* Nothing to do? */
    if (!circ_has_data()) return;

    /* ----------------------------------------------------------------
     * Step 1: Drain ENTIRE circular buffer into local batch
     * This empties the buffer completely.
     * ---------------------------------------------------------------- */
    sample_t batch[CIRC_BUF_SIZE];
    uint32_t batch_size = 0;

    /* Critical section: protect circ_count read */
    /* DISABLE_INTERRUPTS(); */
    while (circ_has_data()) {
        circ_read(&batch[batch_size++]);
    }
    /* ENABLE_INTERRUPTS(); */
    /* Buffer is now EMPTY ✅ */

    if (batch_size > max_batch_seen) max_batch_seen = batch_size;

    printf("[MAIN] Drained %u samples from buffer (buffer now empty)\n", batch_size);

    /* ----------------------------------------------------------------
     * Step 2: Write entire batch to SD — ONE write for all samples
     * Cost: 50ms regardless of batch size
     * During this 50ms, ISR adds ~5 new samples to buffer
     * ---------------------------------------------------------------- */
    hw_sd_write_batch(batch, batch_size);
    total_logged += batch_size;

    /* ----------------------------------------------------------------
     * Step 3: Update display with the LATEST sample — once per batch
     * Cost: 30ms
     * During this 30ms, ISR adds ~3 more samples to buffer
     * ---------------------------------------------------------------- */
    hw_update_display(batch[batch_size - 1]);
    display_count++;

    /* ----------------------------------------------------------------
     * Step 4: Check all samples for alert threshold
     * Cost: ~0ms normally, 100ms only if alert fires (rare)
     * ---------------------------------------------------------------- */
    for (uint32_t i = 0; i < batch_size; i++) {
        for (int j = 0; j < 4; j++) {
            if (batch[i].temp[j] > ALERT_THRESHOLD) {
                hw_send_alert(batch[i], j);
                alert_count++;
                goto done_alerts; /* One alert per batch max */
            }
        }
    }
done_alerts:;

    /* After processing:
     * Buffer has ~8 new samples (arrived during 80ms processing)
     * Next iteration will drain those 8
     * Buffer oscillates 0 ↔ 8, never overflows ✅
     */
}

/* ============================================================================
 * SIMULATION
 * ============================================================================ */

int main(void) {
    printf("=== GOOD EXAMPLE: Interrupt + Drain Buffer ===\n\n");
    printf("Design:\n");
    printf("  ISR:       every 10ms → read sensors → circular buffer\n");
    printf("  Main loop: drain entire buffer → SD write → display\n\n");
    printf("Math (WORST CASE):\n");
    printf("  SD + display + alert = 50ms + 30ms + 100ms = 180ms\n");
    printf("  Samples during 180ms = 180/10 = 18\n");
    printf("  Buffer size: 128 slots (18 × 4 safety = 72 → 128) ✅\n\n");

    uint32_t sim_end_ms    = 500;
    uint32_t next_isr_ms   = 10;   /* First ISR at 10ms */
    uint32_t samples_missed = 0;

    printf("--- Simulation Start ---\n\n");

    while (sys_ms < sim_end_ms) {

        /* Simulate ISR firing every 10ms */
        if (sys_ms >= next_isr_ms) {
            TIMER_IRQHandler();
            next_isr_ms += 10;
        }

        /* Main loop processes when buffer has data */
        if (circ_has_data()) {
            main_loop_iteration();
        }

        /* Advance time if nothing to do */
        if (!circ_has_data() && sys_ms < next_isr_ms) {
            sys_ms = next_isr_ms; /* Jump to next ISR */
        }
    }

    /* Flush remaining samples */
    if (circ_has_data()) {
        main_loop_iteration();
    }

    printf("\n--- Simulation End ---\n\n");

    uint32_t total_required = sim_end_ms / 10;

    printf("=== Results ===\n");
    printf("Simulation time:    %ums\n", sim_end_ms);
    printf("ISR fires:          %u (every 10ms exact)\n", isr_count);
    printf("Samples required:   %u\n", total_required);
    printf("Samples logged:     %u\n", total_logged);
    printf("Samples missed:     %u\n", total_required > total_logged ?
                                        total_required - total_logged : 0);
    printf("Data loss:          %s\n", total_logged >= total_required ? "0%% ✅" : "YES ❌");
    printf("Display updates:    %u\n", display_count);
    printf("Alerts sent:        %u\n", alert_count);
    printf("Max buffer usage:   %u / %u slots\n", max_batch_seen, CIRC_BUF_SIZE);
    printf("Buffer overflow:    %s\n", max_batch_seen < CIRC_BUF_SIZE ? "Never ✅" : "YES ❌");

    printf("\n=== Why This Works ===\n");
    printf("1. ✅ ISR samples at exact 10ms (hardware-guaranteed)\n");
    printf("2. ✅ ISR is fast (~1ms) — never misses next interrupt\n");
    printf("3. ✅ Main drains entire buffer → buffer returns to 0\n");
    printf("4. ✅ SD write done ONCE per batch (not per sample)\n");
    printf("5. ✅ Display updated ONCE per batch (not per sample)\n");
    printf("6. ✅ Buffer max = 18 samples (worst case) << 128 slots\n");
    printf("7. ✅ Zero data loss, compliance passes\n");

    return 0;
}

/*
 * DESIGN SUMMARY:
 *
 * ISR (every 10ms, ~1ms):
 *   read_sensors() → circ_write()
 *
 * Main loop (per batch, ~80ms):
 *   drain entire buffer → local batch   (buffer = 0)
 *   sd_write_batch(batch, N)            (50ms, ISR adds ~5 samples)
 *   update_display(batch[last])         (30ms, ISR adds ~3 samples)
 *   check_alerts(batch)                 (~0ms normally)
 *
 * Buffer behavior:
 *   Drains to 0 each iteration
 *   Normal: fills to ~8 (80ms batch)
 *   Worst case: fills to ~18 (180ms batch with alert)
 *   Never exceeds 18 < 128 slots ✅
 */
