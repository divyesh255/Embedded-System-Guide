/**
 * 04_production.c - PRODUCTION: Industrial-Grade State Machine
 * 
 * This shows PRODUCTION-READY state machine implementation with:
 * - Error handling
 * - State validation
 * - Logging
 * - Multiple wash programs
 * - Guard conditions
 * - Defensive programming
 * 
 * Study time: 25 minutes
 * This is how professionals implement state machines!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INVALID_STATE,
    STATUS_ERROR_INVALID_EVENT,
    STATUS_ERROR_DOOR_OPEN,
    STATUS_ERROR_HARDWARE
} status_t;

/* ============================================================================
 * STATE MACHINE DEFINITION
 * ============================================================================ */

typedef enum {
    STATE_IDLE,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_DONE,
    STATE_PAUSED,
    STATE_ERROR,
    STATE_MAX  /* For validation */
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
    EVENT_NONE,
    EVENT_MAX  /* For validation */
} wash_event_t;

/* Wash programs */
typedef enum {
    PROGRAM_NORMAL,
    PROGRAM_DELICATE,
    PROGRAM_HEAVY,
    PROGRAM_QUICK
} wash_program_t;

typedef struct {
    uint32_t fill_time;
    uint32_t wash_time;
    uint32_t spin_time;
    uint8_t motor_speed;
} program_config_t;

const program_config_t programs[] = {
    [PROGRAM_NORMAL]   = {10, 30, 20, 100},
    [PROGRAM_DELICATE] = {10, 20, 10, 50},
    [PROGRAM_HEAVY]    = {15, 45, 30, 100},
    [PROGRAM_QUICK]    = {5,  15, 10, 100}
};

/* State machine context */
typedef struct {
    wash_state_t current_state;
    wash_state_t previous_state;
    wash_program_t program;
    uint32_t wash_timer;
    uint32_t spin_timer;
    uint32_t state_entry_time;
    uint32_t error_count;
    bool door_open;
    bool initialized;
} wash_machine_t;

static wash_machine_t machine = {0};

/* State names for logging */
const char* state_names[] = {
    "IDLE", "FILLING", "WASHING", "DRAINING",
    "SPINNING", "DONE", "PAUSED", "ERROR"
};

const char* event_names[] = {
    "START", "WATER_FULL", "WASH_DONE", "DRAIN_DONE",
    "SPIN_DONE", "PAUSE", "RESUME", "STOP",
    "ERROR", "RESET", "NONE"
};

/* ============================================================================
 * LOGGING & DIAGNOSTICS
 * ============================================================================ */

void log_state_change(wash_state_t from, wash_state_t to) {
    printf("[LOG] State: %s -> %s\n", state_names[from], state_names[to]);
}

void log_event(wash_event_t event) {
    if (event != EVENT_NONE) {
        printf("[LOG] Event: %s\n", event_names[event]);
    }
}

void log_error(const char* message) {
    printf("[ERROR] %s\n", message);
    machine.error_count++;
}

/* ============================================================================
 * HARDWARE ABSTRACTION (with error checking)
 * ============================================================================ */

status_t hw_lock_door(void) {
    printf("  [HW] Door locked\n");
    return STATUS_OK;
}

status_t hw_unlock_door(void) {
    printf("  [HW] Door unlocked\n");
    return STATUS_OK;
}

status_t hw_open_water_valve(void) {
    printf("  [HW] Water valve open\n");
    return STATUS_OK;
}

status_t hw_close_water_valve(void) {
    printf("  [HW] Water valve closed\n");
    return STATUS_OK;
}

status_t hw_start_motor(uint8_t speed) {
    printf("  [HW] Motor at %d%% speed\n", speed);
    return STATUS_OK;
}

status_t hw_stop_motor(void) {
    printf("  [HW] Motor stopped\n");
    return STATUS_OK;
}

status_t hw_open_drain_valve(void) {
    printf("  [HW] Drain valve open\n");
    return STATUS_OK;
}

status_t hw_close_drain_valve(void) {
    printf("  [HW] Drain valve closed\n");
    return STATUS_OK;
}

void hw_beep(uint8_t count) {
    printf("  [HW] BEEP! (%d times)\n", count);
}

/* ============================================================================
 * STATE ENTRY/EXIT ACTIONS (with error handling)
 * ============================================================================ */

status_t on_enter_idle(void) {
    hw_unlock_door();
    machine.wash_timer = 0;
    machine.spin_timer = 0;
    return STATUS_OK;
}

status_t on_enter_filling(void) {
    status_t status;
    
    /* Guard: Check door */
    if (machine.door_open) {
        log_error("Cannot fill with door open");
        return STATUS_ERROR_DOOR_OPEN;
    }
    
    status = hw_lock_door();
    if (status != STATUS_OK) return status;
    
    status = hw_open_water_valve();
    if (status != STATUS_OK) return status;
    
    return STATUS_OK;
}

status_t on_exit_filling(void) {
    return hw_close_water_valve();
}

status_t on_enter_washing(void) {
    const program_config_t *config = &programs[machine.program];
    machine.wash_timer = config->wash_time;
    return hw_start_motor(config->motor_speed);
}

status_t on_exit_washing(void) {
    return hw_stop_motor();
}

status_t on_enter_draining(void) {
    return hw_open_drain_valve();
}

status_t on_exit_draining(void) {
    return hw_close_drain_valve();
}

status_t on_enter_spinning(void) {
    const program_config_t *config = &programs[machine.program];
    machine.spin_timer = config->spin_time;
    return hw_start_motor(100);  /* Full speed */
}

status_t on_exit_spinning(void) {
    return hw_stop_motor();
}

status_t on_enter_done(void) {
    hw_unlock_door();
    hw_beep(3);
    return STATUS_OK;
}

status_t on_enter_paused(void) {
    machine.previous_state = machine.current_state;
    hw_stop_motor();
    hw_close_water_valve();
    return STATUS_OK;
}

status_t on_enter_error(void) {
    hw_stop_motor();
    hw_close_water_valve();
    hw_close_drain_valve();
    hw_beep(5);
    return STATUS_OK;
}

/* ============================================================================
 * STATE MACHINE CORE (with validation)
 * ============================================================================ */

status_t state_machine_transition(wash_state_t new_state) {
    status_t status;
    wash_state_t old_state = machine.current_state;
    
    /* Validate new state */
    if (new_state >= STATE_MAX) {
        log_error("Invalid state transition");
        return STATUS_ERROR_INVALID_STATE;
    }
    
    /* Exit actions */
    switch (old_state) {
        case STATE_FILLING:
            status = on_exit_filling();
            break;
        case STATE_WASHING:
            status = on_exit_washing();
            break;
        case STATE_DRAINING:
            status = on_exit_draining();
            break;
        case STATE_SPINNING:
            status = on_exit_spinning();
            break;
        default:
            status = STATUS_OK;
            break;
    }
    
    if (status != STATUS_OK) {
        log_error("Exit action failed");
        return status;
    }
    
    /* Change state */
    machine.current_state = new_state;
    machine.state_entry_time = 0;  /* Reset timer */
    log_state_change(old_state, new_state);
    
    /* Entry actions */
    switch (new_state) {
        case STATE_IDLE:
            status = on_enter_idle();
            break;
        case STATE_FILLING:
            status = on_enter_filling();
            break;
        case STATE_WASHING:
            status = on_enter_washing();
            break;
        case STATE_DRAINING:
            status = on_enter_draining();
            break;
        case STATE_SPINNING:
            status = on_enter_spinning();
            break;
        case STATE_DONE:
            status = on_enter_done();
            break;
        case STATE_PAUSED:
            status = on_enter_paused();
            break;
        case STATE_ERROR:
            status = on_enter_error();
            break;
        default:
            status = STATUS_ERROR_INVALID_STATE;
            break;
    }
    
    if (status != STATUS_OK) {
        log_error("Entry action failed");
        machine.current_state = STATE_ERROR;
        on_enter_error();
    }
    
    return status;
}

status_t state_machine_run(wash_event_t event) {
    /* Validate event */
    if (event >= EVENT_MAX) {
        return STATUS_ERROR_INVALID_EVENT;
    }
    
    log_event(event);
    
    /* State machine logic */
    switch (machine.current_state) {
        case STATE_IDLE:
            if (event == EVENT_START && !machine.door_open) {
                return state_machine_transition(STATE_FILLING);
            }
            break;
            
        case STATE_FILLING:
            if (event == EVENT_WATER_FULL) {
                return state_machine_transition(STATE_WASHING);
            } else if (event == EVENT_PAUSE) {
                return state_machine_transition(STATE_PAUSED);
            } else if (event == EVENT_ERROR) {
                return state_machine_transition(STATE_ERROR);
            } else if (event == EVENT_STOP) {
                return state_machine_transition(STATE_DRAINING);
            }
            break;
            
        case STATE_WASHING:
            if (machine.wash_timer > 0) {
                machine.wash_timer--;
            }
            
            if (event == EVENT_WASH_DONE || machine.wash_timer == 0) {
                return state_machine_transition(STATE_DRAINING);
            } else if (event == EVENT_PAUSE) {
                return state_machine_transition(STATE_PAUSED);
            } else if (event == EVENT_ERROR) {
                return state_machine_transition(STATE_ERROR);
            }
            break;
            
        case STATE_DRAINING:
            if (event == EVENT_DRAIN_DONE) {
                return state_machine_transition(STATE_SPINNING);
            } else if (event == EVENT_ERROR) {
                return state_machine_transition(STATE_ERROR);
            }
            break;
            
        case STATE_SPINNING:
            if (machine.spin_timer > 0) {
                machine.spin_timer--;
            }
            
            if (event == EVENT_SPIN_DONE || machine.spin_timer == 0) {
                return state_machine_transition(STATE_DONE);
            } else if (event == EVENT_ERROR) {
                return state_machine_transition(STATE_ERROR);
            }
            break;
            
        case STATE_DONE:
            if (event == EVENT_START && !machine.door_open) {
                return state_machine_transition(STATE_FILLING);
            }
            break;
            
        case STATE_PAUSED:
            if (event == EVENT_RESUME) {
                return state_machine_transition(machine.previous_state);
            } else if (event == EVENT_STOP) {
                return state_machine_transition(STATE_IDLE);
            }
            break;
            
        case STATE_ERROR:
            if (event == EVENT_RESET) {
                machine.error_count = 0;
                return state_machine_transition(STATE_IDLE);
            }
            break;
            
        default:
            return STATUS_ERROR_INVALID_STATE;
    }
    
    return STATUS_OK;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

status_t wash_machine_init(wash_program_t program) {
    memset(&machine, 0, sizeof(machine));
    machine.current_state = STATE_IDLE;
    machine.program = program;
    machine.initialized = true;
    printf("[INIT] Washing machine initialized with program: %d\n", program);
    return STATUS_OK;
}

wash_state_t wash_machine_get_state(void) {
    return machine.current_state;
}

uint32_t wash_machine_get_error_count(void) {
    return machine.error_count;
}

/* ============================================================================
 * PRODUCTION FEATURES DEMONSTRATED
 * ============================================================================ */

int main(void) {
    printf("=== PRODUCTION: Industrial State Machine ===\n\n");
    
    /* Initialize with normal program */
    wash_machine_init(PROGRAM_NORMAL);
    
    /* Run a complete cycle */
    printf("\n--- Starting wash cycle ---\n");
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
    state_machine_run(EVENT_DRAIN_DONE);
    
    /* Simulate spinning */
    for (int i = 0; i < 20; i++) {
        state_machine_run(EVENT_NONE);
    }
    
    printf("\n=== Production Features ===\n");
    printf("1. Error handling with status codes\n");
    printf("2. State validation\n");
    printf("3. Event validation\n");
    printf("4. Logging and diagnostics\n");
    printf("5. Multiple wash programs\n");
    printf("6. Guard conditions\n");
    printf("7. Entry/exit action error handling\n");
    printf("8. Error counting\n");
    printf("9. Hardware abstraction\n");
    printf("10. Defensive programming\n");
    
    printf("\nTotal errors: %u\n", wash_machine_get_error_count());
    
    return 0;
}

/*
 * PRODUCTION-GRADE FEATURES:
 * 
 * 1. ERROR HANDLING
 *    - Status codes for all operations
 *    - Error counting and logging
 *    - Graceful degradation
 * 
 * 2. VALIDATION
 *    - State bounds checking
 *    - Event validation
 *    - Guard conditions
 * 
 * 3. CONFIGURABILITY
 *    - Multiple wash programs
 *    - Easy to add new programs
 *    - Centralized configuration
 * 
 * 4. DIAGNOSTICS
 *    - Comprehensive logging
 *    - State change tracking
 *    - Error reporting
 * 
 * 5. SAFETY
 *    - Door interlocks
 *    - Hardware error handling
 *    - Fail-safe defaults
 * 
 * 6. MAINTAINABILITY
 *    - Clear structure
 *    - Well-documented
 *    - Easy to extend
 * 
 * This is how state machines are implemented in real products!
 */
