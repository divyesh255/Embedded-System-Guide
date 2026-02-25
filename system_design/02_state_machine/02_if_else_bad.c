/**
 * 02_if_else_bad.c - BAD EXAMPLE: If-Else Hell (No State Machine)
 * 
 * This is the WRONG way to implement control logic.
 * Uses boolean flags instead of proper state machine.
 * 
 * Problems:
 * - State explosion (256 possible combinations!)
 * - Hard to test
 * - Easy to create invalid states
 * - Unmaintainable
 * - Can't visualize behavior
 * 
 * Study time: 15 minutes
 * DO NOT use this pattern in production!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
 * GLOBAL STATE FLAGS - THE PROBLEM!
 * 
 * 8 boolean flags = 2^8 = 256 possible combinations
 * But only 6-8 valid states exist!
 * This means 248+ INVALID states are possible!
 * ============================================================================ */

static bool door_open = false;
static bool water_filling = false;
static bool water_full = false;
static bool washing = false;
static bool draining = false;
static bool spinning = false;
static bool paused = false;
static bool error = false;
static uint32_t wash_timer = 0;
static uint32_t spin_timer = 0;

/* Simulated hardware functions */
void lock_door(void) { printf("  [HW] Door locked\n"); }
void unlock_door(void) { printf("  [HW] Door unlocked\n"); }
void open_water_valve(void) { printf("  [HW] Water valve open\n"); }
void close_water_valve(void) { printf("  [HW] Water valve closed\n"); }
void start_motor_slow(void) { printf("  [HW] Motor slow\n"); }
void start_motor_fast(void) { printf("  [HW] Motor fast\n"); }
void stop_motor(void) { printf("  [HW] Motor stopped\n"); }
void open_drain_valve(void) { printf("  [HW] Drain valve open\n"); }
void close_drain_valve(void) { printf("  [HW] Drain valve closed\n"); }
void beep_error(void) { printf("  [HW] BEEP! Error\n"); }
void beep_done(void) { printf("  [HW] BEEP! Done\n"); }
void display_error(void) { printf("  [HW] Display: ERROR\n"); }

/**
 * THE MONOLITHIC MONSTER
 * 
 * This function tries to handle everything with if-else chains.
 * Result: Unmaintainable spaghetti code!
 */
void washing_machine_control_bad(void) {
    /* ========== Check door first ========== */
    if (door_open) {
        /* Problem: What if we were in middle of washing? */
        stop_motor();
        close_water_valve();
        unlock_door();
        
        /* Can't do anything with door open */
        return;
    }
    
    /* ========== Handle pause state ========== */
    if (paused) {
        /* Problem: We don't know which state we paused from! */
        stop_motor();
        
        /* Try to remember what we were doing... */
        if (water_filling) {
            close_water_valve();
        }
        
        /* Problem: How to resume? Need to check all flags! */
        return;
    }
    
    /* ========== Handle error state ========== */
    if (error) {
        /* Problem: Error handling scattered everywhere */
        stop_motor();
        close_water_valve();
        close_drain_valve();
        display_error();
        
        /* Problem: How to recover? */
        return;
    }
    
    /* ========== Handle filling ========== */
    if (water_filling) {
        open_water_valve();
        
        if (water_full) {
            /* Transition to washing */
            close_water_valve();
            water_filling = false;  /* Clear flag */
            washing = true;         /* Set new flag */
            wash_timer = 30;        /* Hardcoded! */
            start_motor_slow();
        }
        
        /* Problem: What if error during filling? */
        /* Problem: What if pause during filling? */
        /* Need to handle in every state! */
        
        return;
    }
    
    /* ========== Handle washing ========== */
    if (washing) {
        start_motor_slow();
        
        if (wash_timer > 0) {
            wash_timer--;
        } else {
            /* Transition to draining */
            stop_motor();
            washing = false;
            draining = true;
        }
        
        /* Problem: Timer logic mixed with state logic */
        /* Problem: What if motor fails? */
        
        return;
    }
    
    /* ========== Handle draining ========== */
    if (draining) {
        open_drain_valve();
        
        /* Simulate drain complete */
        static int drain_count = 0;
        drain_count++;
        if (drain_count > 10) {
            /* Transition to spinning */
            close_drain_valve();
            draining = false;
            spinning = true;
            spin_timer = 20;
            start_motor_fast();
            drain_count = 0;
        }
        
        return;
    }
    
    /* ========== Handle spinning ========== */
    if (spinning) {
        start_motor_fast();
        
        if (spin_timer > 0) {
            spin_timer--;
        } else {
            /* Done! */
            stop_motor();
            spinning = false;
            unlock_door();
            beep_done();
        }
        
        return;
    }
    
    /* ========== Idle state (maybe?) ========== */
    /* Problem: Is this idle? Or done? Or error? */
    /* No clear state! */
}

/**
 * PROBLEMS WITH THIS CODE:
 * 
 * 1. STATE EXPLOSION
 *    - 8 flags = 256 combinations
 *    - Only ~6 valid states
 *    - 250 invalid states possible!
 *    - Example invalid: water_filling=true AND washing=true
 * 
 * 2. NO CLEAR TRANSITIONS
 *    - Transitions hidden in if blocks
 *    - Can't visualize flow
 *    - Hard to verify correctness
 * 
 * 3. SCATTERED LOGIC
 *    - Error handling in every state
 *    - Pause handling in every state
 *    - Code duplication everywhere
 * 
 * 4. IMPOSSIBLE TO TEST
 *    - How to test all 256 combinations?
 *    - How to verify only valid states?
 *    - How to test transitions?
 * 
 * 5. UNMAINTAINABLE
 *    - Adding feature requires changes everywhere
 *    - High risk of breaking existing code
 *    - New developers can't understand flow
 * 
 * 6. RACE CONDITIONS
 *    - Multiple flags can change simultaneously
 *    - No atomic state transitions
 *    - Timing-dependent bugs
 * 
 * 7. NO VISUALIZATION
 *    - Can't draw state diagram
 *    - Can't explain to stakeholders
 *    - Can't prove safety
 * 
 * 8. HARDCODED VALUES
 *    - Timers hardcoded in logic
 *    - Can't support multiple programs
 *    - Configuration nightmare
 */

/**
 * EXAMPLE BUGS THAT CAN HAPPEN:
 * 
 * Bug 1: Invalid State
 *   water_filling = true;
 *   washing = true;  // Both true! Invalid!
 * 
 * Bug 2: Lost State
 *   paused = true;
 *   // But we forgot which state we paused from!
 * 
 * Bug 3: Forgotten Cleanup
 *   washing = false;
 *   draining = true;
 *   // Forgot to stop motor!
 * 
 * Bug 4: Missing Error Handling
 *   if (filling) {
 *       // What if motor error here?
 *       // No error handling!
 *   }
 * 
 * Bug 5: Race Condition
 *   if (!door_open) {
 *       // Door could open here!
 *       start_motor();  // Danger!
 *   }
 */

int main(void) {
    printf("=== BAD EXAMPLE: If-Else Hell ===\n\n");
    
    printf("Simulating wash cycle...\n");
    
    /* Simulate a wash cycle */
    water_filling = true;
    
    for (int i = 0; i < 100; i++) {
        washing_machine_control_bad();
        
        /* Simulate water full */
        if (i == 5 && water_filling) {
            water_full = true;
        }
    }
    
    printf("\n=== Problems with this approach ===\n");
    printf("1. 256 possible flag combinations (only ~6 valid!)\n");
    printf("2. Can't visualize state flow\n");
    printf("3. Easy to create invalid states\n");
    printf("4. Impossible to test thoroughly\n");
    printf("5. Unmaintainable as features grow\n");
    printf("6. No clear error handling strategy\n");
    printf("7. Race conditions possible\n");
    printf("8. Can't prove safety for certification\n");
    
    printf("\n=== Real-World Consequences ===\n");
    printf("- Failed safety audits\n");
    printf("- Customer damage claims\n");
    printf("- Months of delays\n");
    printf("- $500,000+ in rework\n");
    
    printf("\nSee 03_state_machine_good.c for the RIGHT way!\n");
    
    return 0;
}

/*
 * KEY LESSONS:
 * 
 * 1. Boolean flags create state explosion
 * 2. If-else chains become unmaintainable
 * 3. No clear state transitions
 * 4. Impossible to visualize
 * 5. Can't prove correctness
 * 
 * SOLUTION: Use a proper state machine!
 * 
 * NEXT: See how state machine fixes ALL these problems!
 * Continue to: 03_state_machine_good.c
 */
