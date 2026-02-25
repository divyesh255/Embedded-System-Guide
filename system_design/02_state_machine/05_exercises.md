# State Machine - Practice Exercises

Test your understanding of state machines with these hands-on exercises!

## 🎯 Exercise 1: Identify States (Easy - 15 min)

A traffic light system has these behaviors:
- Green for 30 seconds
- Yellow for 5 seconds  
- Red for 30 seconds
- Emergency mode (all red, flashing)

**Task:** List all states and draw the state diagram.

<details>
<summary>Solution</summary>

**States:**
```c
typedef enum {
    STATE_GREEN,
    STATE_YELLOW,
    STATE_RED,
    STATE_EMERGENCY
} traffic_state_t;
```

**State Diagram:**
```
    ┌─────────┐
    │  GREEN  │ (30s)
    └────┬────┘
         │ timer
         ▼
    ┌─────────┐
    │ YELLOW  │ (5s)
    └────┬────┘
         │ timer
         ▼
    ┌─────────┐
    │   RED   │ (30s)
    └────┬────┘
         │ timer
         └──────► GREEN (cycle)
         
    EMERGENCY ◄──── (from any state on emergency button)
         │
         └──────► RED (on reset)
```

**Events:**
- EVENT_TIMER_EXPIRED
- EVENT_EMERGENCY
- EVENT_RESET
</details>

---

## 🎯 Exercise 2: Fix the Bug (Medium - 20 min)

This state machine has a bug. Find and fix it:

```c
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_STOPPED
} motor_state_t;

motor_state_t state = STATE_IDLE;

void motor_control(event_t event) {
    switch (state) {
        case STATE_IDLE:
            if (event == EVENT_START) {
                start_motor();
                state = STATE_RUNNING;
            }
            break;
            
        case STATE_RUNNING:
            if (event == EVENT_STOP) {
                state = STATE_STOPPED;
            }
            break;
            
        case STATE_STOPPED:
            if (event == EVENT_START) {
                state = STATE_RUNNING;
            }
            break;
    }
}
```

**Hint:** What happens when transitioning to STOPPED?

<details>
<summary>Solution</summary>

**Bug:** Motor is not stopped when entering STATE_STOPPED!

**Fixed Code:**
```c
void motor_control(event_t event) {
    switch (state) {
        case STATE_IDLE:
            if (event == EVENT_START) {
                start_motor();  // Entry action
                state = STATE_RUNNING;
            }
            break;
            
        case STATE_RUNNING:
            if (event == EVENT_STOP) {
                stop_motor();  // EXIT ACTION MISSING!
                state = STATE_STOPPED;
            }
            break;
            
        case STATE_STOPPED:
            if (event == EVENT_START) {
                start_motor();  // Entry action
                state = STATE_RUNNING;
            }
            break;
    }
}
```

**Better Approach with Entry/Exit Actions:**
```c
void on_enter_running(void) {
    start_motor();
}

void on_exit_running(void) {
    stop_motor();
}

void transition_to(motor_state_t new_state) {
    // Exit current state
    if (state == STATE_RUNNING) {
        on_exit_running();
    }
    
    // Change state
    state = new_state;
    
    // Enter new state
    if (state == STATE_RUNNING) {
        on_enter_running();
    }
}
```

**Key Lesson:** Always use entry/exit actions to ensure proper initialization and cleanup!
</details>

---

## 🎯 Exercise 3: Design a State Machine (Medium - 30 min)

**Scenario:** Design a state machine for an ATM (cash machine).

**Requirements:**
- Insert card
- Enter PIN (3 attempts max)
- Select transaction (Withdraw, Deposit, Balance)
- Complete transaction
- Eject card

**Task:** 
1. List all states
2. List all events
3. Draw state diagram
4. Write the state machine code

<details>
<summary>Solution</summary>

**States:**
```c
typedef enum {
    STATE_IDLE,
    STATE_CARD_INSERTED,
    STATE_PIN_ENTRY,
    STATE_MENU,
    STATE_WITHDRAW,
    STATE_DEPOSIT,
    STATE_BALANCE,
    STATE_EJECT_CARD,
    STATE_BLOCKED
} atm_state_t;
```

**Events:**
```c
typedef enum {
    EVENT_CARD_INSERTED,
    EVENT_PIN_CORRECT,
    EVENT_PIN_INCORRECT,
    EVENT_SELECT_WITHDRAW,
    EVENT_SELECT_DEPOSIT,
    EVENT_SELECT_BALANCE,
    EVENT_TRANSACTION_COMPLETE,
    EVENT_CANCEL,
    EVENT_TIMEOUT
} atm_event_t;
```

**State Diagram:**
```
IDLE
  │ card inserted
  ▼
CARD_INSERTED
  │ request PIN
  ▼
PIN_ENTRY ◄──┐ (3 attempts)
  │          │ incorrect
  │ correct  │
  ▼          │
MENU ────────┘
  │
  ├──► WITHDRAW ──┐
  ├──► DEPOSIT ───┤
  └──► BALANCE ───┤
                  │ complete
                  ▼
              EJECT_CARD
                  │
                  ▼
                IDLE
```

**Implementation:**
```c
typedef struct {
    atm_state_t state;
    uint8_t pin_attempts;
} atm_t;

static atm_t atm = {STATE_IDLE, 0};

void atm_run(atm_event_t event) {
    switch (atm.state) {
        case STATE_IDLE:
            if (event == EVENT_CARD_INSERTED) {
                atm.pin_attempts = 0;
                atm.state = STATE_PIN_ENTRY;
            }
            break;
            
        case STATE_PIN_ENTRY:
            if (event == EVENT_PIN_CORRECT) {
                atm.state = STATE_MENU;
            } else if (event == EVENT_PIN_INCORRECT) {
                atm.pin_attempts++;
                if (atm.pin_attempts >= 3) {
                    atm.state = STATE_BLOCKED;
                }
            } else if (event == EVENT_CANCEL) {
                atm.state = STATE_EJECT_CARD;
            }
            break;
            
        case STATE_MENU:
            if (event == EVENT_SELECT_WITHDRAW) {
                atm.state = STATE_WITHDRAW;
            } else if (event == EVENT_SELECT_DEPOSIT) {
                atm.state = STATE_DEPOSIT;
            } else if (event == EVENT_SELECT_BALANCE) {
                atm.state = STATE_BALANCE;
            } else if (event == EVENT_CANCEL) {
                atm.state = STATE_EJECT_CARD;
            }
            break;
            
        case STATE_WITHDRAW:
        case STATE_DEPOSIT:
        case STATE_BALANCE:
            if (event == EVENT_TRANSACTION_COMPLETE) {
                atm.state = STATE_EJECT_CARD;
            }
            break;
            
        case STATE_EJECT_CARD:
            // Eject card, then return to idle
            atm.state = STATE_IDLE;
            break;
            
        case STATE_BLOCKED:
            // Card blocked, eject and notify
            atm.state = STATE_EJECT_CARD;
            break;
    }
}
```
</details>

---

## 🎯 Exercise 4: Convert If-Else to State Machine (Hard - 45 min)

Convert this if-else code to a proper state machine:

```c
bool heating = false;
bool cooling = false;
float target_temp = 22.0;

void thermostat_control(float current_temp) {
    if (current_temp < target_temp - 2.0) {
        if (!heating) {
            turn_on_heater();
            heating = true;
            cooling = false;
        }
    } else if (current_temp > target_temp + 2.0) {
        if (!cooling) {
            turn_on_cooler();
            cooling = true;
            heating = false;
        }
    } else {
        if (heating) {
            turn_off_heater();
            heating = false;
        }
        if (cooling) {
            turn_off_cooler();
            cooling = false;
        }
    }
}
```

<details>
<summary>Solution</summary>

**State Machine Design:**

```c
typedef enum {
    STATE_IDLE,
    STATE_HEATING,
    STATE_COOLING
} thermostat_state_t;

typedef struct {
    thermostat_state_t state;
    float target_temp;
} thermostat_t;

static thermostat_t thermostat = {
    .state = STATE_IDLE,
    .target_temp = 22.0
};

void on_enter_heating(void) {
    turn_on_heater();
}

void on_exit_heating(void) {
    turn_off_heater();
}

void on_enter_cooling(void) {
    turn_on_cooler();
}

void on_exit_cooling(void) {
    turn_off_cooler();
}

void thermostat_transition(thermostat_state_t new_state) {
    // Exit actions
    if (thermostat.state == STATE_HEATING) {
        on_exit_heating();
    } else if (thermostat.state == STATE_COOLING) {
        on_exit_cooling();
    }
    
    // Change state
    thermostat.state = new_state;
    
    // Entry actions
    if (new_state == STATE_HEATING) {
        on_enter_heating();
    } else if (new_state == STATE_COOLING) {
        on_enter_cooling();
    }
}

void thermostat_control(float current_temp) {
    float target = thermostat.target_temp;
    
    switch (thermostat.state) {
        case STATE_IDLE:
            if (current_temp < target - 2.0) {
                thermostat_transition(STATE_HEATING);
            } else if (current_temp > target + 2.0) {
                thermostat_transition(STATE_COOLING);
            }
            break;
            
        case STATE_HEATING:
            if (current_temp >= target) {
                thermostat_transition(STATE_IDLE);
            }
            break;
            
        case STATE_COOLING:
            if (current_temp <= target) {
                thermostat_transition(STATE_IDLE);
            }
            break;
    }
}
```

**Benefits:**
- ✅ Only 3 valid states (not 4 with boolean flags!)
- ✅ Clear entry/exit actions
- ✅ No forgotten cleanup
- ✅ Easy to test
- ✅ Can visualize as diagram
</details>

---

## 🎯 Exercise 5: Add Error Handling (Hard - 45 min)

Add comprehensive error handling to this state machine:

```c
typedef enum {
    STATE_IDLE,
    STATE_PUMPING,
    STATE_DONE
} pump_state_t;

pump_state_t state = STATE_IDLE;

void pump_control(event_t event) {
    switch (state) {
        case STATE_IDLE:
            if (event == EVENT_START) {
                start_pump();
                state = STATE_PUMPING;
            }
            break;
            
        case STATE_PUMPING:
            if (event == EVENT_LEVEL_REACHED) {
                stop_pump();
                state = STATE_DONE;
            }
            break;
            
        case STATE_DONE:
            if (event == EVENT_RESET) {
                state = STATE_IDLE;
            }
            break;
    }
}
```

**Requirements:**
- Add ERROR state
- Handle pump failure
- Handle sensor failure
- Add timeout protection
- Add error recovery

<details>
<summary>Solution</summary>

```c
typedef enum {
    STATE_IDLE,
    STATE_PUMPING,
    STATE_DONE,
    STATE_ERROR
} pump_state_t;

typedef enum {
    EVENT_START,
    EVENT_LEVEL_REACHED,
    EVENT_RESET,
    EVENT_PUMP_FAILURE,
    EVENT_SENSOR_FAILURE,
    EVENT_TIMEOUT,
    EVENT_ERROR_CLEARED
} pump_event_t;

typedef enum {
    ERROR_NONE,
    ERROR_PUMP_FAILURE,
    ERROR_SENSOR_FAILURE,
    ERROR_TIMEOUT
} error_code_t;

typedef struct {
    pump_state_t state;
    error_code_t last_error;
    uint32_t pump_timer;
    uint32_t error_count;
} pump_t;

static pump_t pump = {
    .state = STATE_IDLE,
    .last_error = ERROR_NONE,
    .pump_timer = 0,
    .error_count = 0
};

#define PUMP_TIMEOUT 300  /* 5 minutes */

void on_enter_error(error_code_t error) {
    stop_pump();
    pump.last_error = error;
    pump.error_count++;
    log_error(error);
    alert_operator();
}

void pump_control(pump_event_t event) {
    switch (pump.state) {
        case STATE_IDLE:
            if (event == EVENT_START) {
                if (check_pump_ok() && check_sensor_ok()) {
                    start_pump();
                    pump.pump_timer = 0;
                    pump.state = STATE_PUMPING;
                } else {
                    pump.state = STATE_ERROR;
                    on_enter_error(ERROR_PUMP_FAILURE);
                }
            }
            break;
            
        case STATE_PUMPING:
            pump.pump_timer++;
            
            if (event == EVENT_LEVEL_REACHED) {
                stop_pump();
                pump.state = STATE_DONE;
            } else if (event == EVENT_PUMP_FAILURE) {
                pump.state = STATE_ERROR;
                on_enter_error(ERROR_PUMP_FAILURE);
            } else if (event == EVENT_SENSOR_FAILURE) {
                pump.state = STATE_ERROR;
                on_enter_error(ERROR_SENSOR_FAILURE);
            } else if (pump.pump_timer > PUMP_TIMEOUT) {
                pump.state = STATE_ERROR;
                on_enter_error(ERROR_TIMEOUT);
            }
            break;
            
        case STATE_DONE:
            if (event == EVENT_RESET) {
                pump.state = STATE_IDLE;
            }
            break;
            
        case STATE_ERROR:
            if (event == EVENT_ERROR_CLEARED) {
                if (pump.error_count < 3) {
                    pump.last_error = ERROR_NONE;
                    pump.state = STATE_IDLE;
                } else {
                    // Too many errors, require manual reset
                    log_error("Too many errors, manual reset required");
                }
            }
            break;
    }
}
```

**Key Features:**
- ✅ Comprehensive error handling
- ✅ Error counting
- ✅ Timeout protection
- ✅ Error recovery logic
- ✅ Safety interlocks
- ✅ Operator alerts
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **State Identification**
   - How to identify states from requirements
   - How to draw state diagrams

2. **Bug Prevention**
   - Importance of entry/exit actions
   - Common state machine bugs

3. **System Design**
   - How to design state machines from scratch
   - How to handle complex requirements

4. **Code Conversion**
   - How to convert if-else to state machines
   - Benefits of state machine approach

5. **Error Handling**
   - How to add robust error handling
   - Recovery strategies

## 🚀 Next Steps

1. **Practice:** Refactor your own code to use state machines
2. **Study:** Look at state machines in open-source projects
3. **Apply:** Use state machines in your next embedded project
4. **Continue:** Move to next design pattern (Hierarchical State Machines)

---

**Congratulations!** You've mastered State Machines - the backbone of embedded systems!