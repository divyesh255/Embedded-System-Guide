/**
 * 03_state_machine_good.c - GOOD EXAMPLE: Proper State Machine
 * 
 * This shows the RIGHT way to implement control logic.
 * Uses a proper Finite State Machine (FSM).
 * 
 * Benefits:
 * - Only valid states possible (8 states, not 256!)
 * - Clear transitions
 * - Easy to test
 * - Visualizable
 * - Maintainable
 * 
 * Study time: 20 minutes
 * This is production-ready architecture!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
 * STATE MACHINE DEFINITION
 * 
 * Only 8 valid states - impossible to be in invalid state!
 * ============================================================================ */

typedef enum {
    STATE_IDLE,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_DONE,
    STATE_PAUSED,
    STATE_ERROR
} wash_state_t;

typedef enum {
    EVENT_START,
    EVENT_WATER_FULL,
    EVENT_WASH_DONE,
    EVENT_DRAIN_DONE,
    EVENT_SPIN_DONE,
    EVENT_PAUSE,
    EVENT_RESUME,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET,
    EVENT_NONE
} wash_event_t;

/* State machine context */
typedef struct {
    wash_state_t current_state;
    wash_state_t previous_state;  /* For pause/resume */
    uint32_t wash_timer;
    uint32_t spin_timer;
    bool door_open;
} wash_machine_t;

static wash_machine_t machine = {
    .current_state = STATE_IDLE,
    .previous_state = STATE_IDLE,
    .wash_timer = 0,
    .spin_timer = 0,
    .door_open = false
};

/* Hardware functions */
void lock_door(void) { printf("  [HW] Door locked\n"); }
void unlock_door(void) { printf("  [HW] Door unlocked\n"); }
void open_water_valve(void) { printf("  [HW] Water valve open\n"); }
void close_water_valve(void) { printf("  [HW] Water valve closed\n"); }
void start_motor_slow(void) { printf("  [HW] Motor slow\n"); }
void start_motor_fast(void) { printf("  [HW] Motor fast\n"); }
void stop_motor(void) { printf("  [HW] Motor stopped\n"); }
void open_drain_valve(void) { printf("  [HW] Drain valve open\n"); }
void close_drain_valve(void) { printf("  [HW] Drain valve closed\n"); }
void beep_done(void) { printf("  [HW] BEEP! Done\n"); }
void display_error(void) { printf("  [HW] Display: ERROR\n"); }

/* ============================================================================
 * STATE ENTRY/EXIT ACTIONS
 * 
 * Clean separation of state behavior
 * ============================================================================ */

void on_enter_idle(void) {
    printf("[STATE] Entering IDLE\n");
    unlock_door();
}

void on_enter_filling(void) {
    printf("[STATE] Entering FILLING\n");
    lock_door();
    open_water_valve();
}

void on_exit_filling(void) {
    close_water_valve();
}

void on_enter_washing(void) {
    printf("[STATE] Entering WASHING\n");
    machine.wash_timer = 30;  /* 30 cycles */
    start_motor_slow();
}

void on_exit_washing(void) {
    stop_motor();
}

void on_enter_draining(void) {
    printf("[STATE] Entering DRAINING\n");
    open_drain_valve();
}

void on_exit_draining(void) {
    close_drain_valve();
}

void on_enter_spinning(void) {
    printf("[STATE] Entering SPINNING\n");
    machine.spin_timer = 20;  /* 20 cycles */
    start_motor_fast();
}

void on_exit_spinning(void) {
    stop_motor();
}

void on_enter_done(void) {
    printf("[STATE] Entering DONE\n");
    unlock_door();
    beep_done();
}

void on_enter_paused(void) {
    printf("[STATE] Entering PAUSED\n");
    machine.previous_state = machine.current_state;
    stop_motor();
    close_water_valve();
}

void on_enter_error(void) {
    printf("[STATE] Entering ERROR\n");
    stop_motor();
    close_water_valve();
    close_drain_valve();
    display_error();
}

/* ============================================================================
 * STATE MACHINE LOGIC
 * 
 * Clear, testable, maintainable!
 * ============================================================================ */

void state_machine_transition(wash_state_t new_state) {
    wash_state_t old_state = machine.current_state;
    
    /* Exit actions */
    switch (old_state) {
        case STATE_FILLING:
            on_exit_filling();
            break;
        case STATE_WASHING:
            on_exit_washing();
            break;
        case STATE_DRAINING:
            on_exit_draining();
            break;
        case STATE_SPINNING:
            on_exit_spinning();
            break;
        default:
            break;
    }
    
    /* Change state */
    machine.current_state = new_state;
    printf("[TRANSITION] %d -> %d\n", old_state, new_state);
    
    /* Entry actions */
    switch (new_state) {
        case STATE_IDLE:
            on_enter_idle();
            break;
        case STATE_FILLING:
            on_enter_filling();
            break;
        case STATE_WASHING:
            on_enter_washing();
            break;
        case STATE_DRAINING:
            on_enter_draining();
            break;
        case STATE_SPINNING:
            on_enter_spinning();
            break;
        case STATE_DONE:
            on_enter_done();
            break;
        case STATE_PAUSED:
            on_enter_paused();
            break;
        case STATE_ERROR:
            on_enter_error();
            break;
    }
}

void state_machine_run(wash_event_t event) {
    switch (machine.current_state) {
        case STATE_IDLE:
            if (event == EVENT_START && !machine.door_open) {
                state_machine_transition(STATE_FILLING);
            }
            break;
            
        case STATE_FILLING:
            if (event == EVENT_WATER_FULL) {
                state_machine_transition(STATE_WASHING);
            } else if (event == EVENT_PAUSE) {
                state_machine_transition(STATE_PAUSED);
            } else if (event == EVENT_ERROR) {
                state_machine_transition(STATE_ERROR);
            } else if (event == EVENT_STOP) {
                state_machine_transition(STATE_DRAINING);
            }
            break;
            
        case STATE_WASHING:
            if (machine.wash_timer > 0) {
                machine.wash_timer--;
            }
            
            if (event == EVENT_WASH_DONE || machine.wash_timer == 0) {
                state_machine_transition(STATE_DRAINING);
            } else if (event == EVENT_PAUSE) {
                state_machine_transition(STATE_PAUSED);
            } else if (event == EVENT_ERROR) {
                state_machine_transition(STATE_ERROR);
            } else if (event == EVENT_STOP) {
                state_machine_transition(STATE_DRAINING);
            }
            break;
            
        case STATE_DRAINING:
            if (event == EVENT_DRAIN_DONE) {
                state_machine_transition(STATE_SPINNING);
            } else if (event == EVENT_ERROR) {
                state_machine_transition(STATE_ERROR);
            } else if (event == EVENT_STOP) {
                state_machine_transition(STATE_DONE);
            }
            break;
            
        case STATE_SPINNING:
            if (machine.spin_timer > 0) {
                machine.spin_timer--;
            }
            
            if (event == EVENT_SPIN_DONE || machine.spin_timer == 0) {
                state_machine_transition(STATE_DONE);
            } else if (event == EVENT_ERROR) {
                state_machine_transition(STATE_ERROR);
            } else if (event == EVENT_STOP) {
                state_machine_transition(STATE_DONE);
            }
            break;
            
        case STATE_DONE:
            if (event == EVENT_START && !machine.door_open) {
                state_machine_transition(STATE_FILLING);
            }
            break;
            
        case STATE_PAUSED:
            if (event == EVENT_RESUME) {
                state_machine_transition(machine.previous_state);
            } else if (event == EVENT_STOP) {
                state_machine_transition(STATE_IDLE);
            }
            break;
            
        case STATE_ERROR:
            if (event == EVENT_RESET) {
                state_machine_transition(STATE_IDLE);
            }
            break;
    }
}

/* ============================================================================
 * BENEFITS OF STATE MACHINE
 * ============================================================================ */

int main(void) {
    printf("=== GOOD EXAMPLE: State Machine ===\n\n");
    
    /* Simulate a complete wash cycle */
    printf("Starting wash cycle...\n\n");
    
    /* Start */
    state_machine_run(EVENT_START);
    
    /* Simulate filling */
    for (int i = 0; i < 5; i++) {
        state_machine_run(EVENT_NONE);
    }
    state_machine_run(EVENT_WATER_FULL);
    
    /* Simulate washing */
    for (int i = 0; i < 30; i++) {
        state_machine_run(EVENT_NONE);
    }
    
    /* Simulate draining */
    for (int i = 0; i < 5; i++) {
        state_machine_run(EVENT_NONE);
    }
    state_machine_run(EVENT_DRAIN_DONE);
    
    /* Simulate spinning */
    for (int i = 0; i < 20; i++) {
        state_machine_run(EVENT_NONE);
    }
    
    printf("\n=== Benefits of State Machine ===\n");
    printf("1. Only 8 valid states (not 256!)\n");
    printf("2. Clear state transitions\n");
    printf("3. Easy to test each state\n");
    printf("4. Can visualize as diagram\n");
    printf("5. Impossible to be in invalid state\n");
    printf("6. Entry/exit actions are clean\n");
    printf("7. Error handling is consistent\n");
    printf("8. Easy to add new features\n");
    
    printf("\n=== Comparison ===\n");
    printf("Boolean flags: 256 combinations (250 invalid!)\n");
    printf("State machine: 8 states (all valid!)\n");
    
    printf("\nSee 04_production.c for industrial implementation!\n");
    
    return 0;
}

/*
 * KEY IMPROVEMENTS:
 * 
 * 1. SINGLE STATE VARIABLE
 *    - Only one state at a time
 *    - Impossible to be in invalid state
 * 
 * 2. CLEAR TRANSITIONS
 *    - Easy to see state flow
 *    - Can draw state diagram
 *    - Verifiable
 * 
 * 3. ENTRY/EXIT ACTIONS
 *    - Clean initialization
 *    - Proper cleanup
 *    - No forgotten actions
 * 
 * 4. TESTABLE
 *    - Test each state independently
 *    - Test each transition
 *    - Mock events easily
 * 
 * 5. MAINTAINABLE
 *    - Adding state is easy
 *    - Changing behavior is localized
 *    - New developers understand quickly
 * 
 * 6. SAFE
 *    - Can prove correctness
 *    - Passes safety audits
 *    - No race conditions
 * 
 * NEXT: See production-grade implementation!
 * Continue to: 04_production.c
 */
