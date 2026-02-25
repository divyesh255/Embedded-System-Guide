# State Machine - The Backbone of Embedded Systems

**Study Time:** 40 minutes  
**Difficulty:** Beginner  
**Industry Use:** Universal - Found in 90%+ of embedded products

## 🎯 What You'll Learn

- What is a Finite State Machine (FSM)?
- Why state machines are essential in embedded systems
- How to design and implement FSMs
- Common patterns and best practices
- Avoiding common pitfalls

## 📖 What is a State Machine?

A **State Machine** (or Finite State Machine - FSM) is a computational model that can be in exactly one of a finite number of states at any given time. It transitions from one state to another in response to events.

### Real-World Analogy: Traffic Light

```
       GREEN
         ↓ (timer expires)
       YELLOW
         ↓ (timer expires)
        RED
         ↓ (timer expires)
       GREEN (cycle repeats)
```

The traffic light is always in ONE state, and transitions happen based on events (timer).

## 🤔 Why Use State Machines?

### Without State Machine (If-Else Hell)
```c
void washing_machine_control() {
    if (door_open) {
        stop_motor();
        if (button_pressed) {
            // Can't start if door open
        }
    } else if (water_filling) {
        if (water_full) {
            start_motor();
            water_filling = false;
            washing = true;
        }
    } else if (washing) {
        if (timer_done) {
            stop_motor();
            drain_water();
            washing = false;
            draining = true;
        }
    } else if (draining) {
        // ... more nested ifs
    }
    // 200+ lines of spaghetti code
}
```

**Problems:**
- ❌ Hard to understand
- ❌ Easy to create invalid states
- ❌ Difficult to test
- ❌ Impossible to visualize
- ❌ Bugs hide in nested conditions

### With State Machine (Clean & Clear)
```c
typedef enum {
    STATE_IDLE,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_DONE
} washing_machine_state_t;

void state_machine_run(event_t event) {
    switch (current_state) {
        case STATE_IDLE:
            if (event == EVENT_START && !door_open) {
                current_state = STATE_FILLING;
            }
            break;
            
        case STATE_FILLING:
            if (event == EVENT_WATER_FULL) {
                current_state = STATE_WASHING;
            }
            break;
            
        // Clear, testable, maintainable
    }
}
```

**Benefits:**
- ✅ Easy to understand
- ✅ Impossible to be in invalid state
- ✅ Easy to test each state
- ✅ Can visualize as diagram
- ✅ Bugs are obvious

## 🏗️ State Machine Components

### 1. States
**Definition:** Distinct conditions or situations the system can be in.

```c
typedef enum {
    STATE_OFF,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_ERROR
} system_state_t;
```

**Rules:**
- System is in EXACTLY ONE state at a time
- Each state represents a distinct behavior
- States should be mutually exclusive

### 2. Events
**Definition:** Triggers that cause state transitions.

```c
typedef enum {
    EVENT_POWER_ON,
    EVENT_START,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET
} system_event_t;
```

**Types:**
- External events (button press, sensor input)
- Internal events (timer expiry, condition met)
- System events (error, reset)

### 3. Transitions
**Definition:** Rules for moving from one state to another.

```
STATE_IDLE + EVENT_START → STATE_RUNNING
STATE_RUNNING + EVENT_STOP → STATE_IDLE
STATE_RUNNING + EVENT_ERROR → STATE_ERROR
```

**Rules:**
- Transitions are triggered by events
- Can have guard conditions
- Can have actions (entry/exit)

### 4. Actions
**Definition:** Operations performed during transitions or in states.

```c
// Entry action (when entering state)
void on_enter_running() {
    start_motor();
    start_timer();
}

// Exit action (when leaving state)
void on_exit_running() {
    stop_motor();
    stop_timer();
}

// Transition action
void on_start_button() {
    log("Starting system");
}
```

## 📊 State Machine Diagram

### Example: Door Lock System

```
                    ┌─────────┐
                    │ LOCKED  │◄─────┐
                    └────┬────┘      │
                         │           │
              EVENT_VALID_PIN        │
                         │           │
                         ▼           │
                    ┌─────────┐      │
                    │UNLOCKED │      │
                    └────┬────┘      │
                         │           │
              EVENT_TIMEOUT           │
              EVENT_LOCK_BUTTON      │
                         │           │
                         └───────────┘
```

## 🎯 State Machine Patterns

### Pattern 1: Simple FSM (Switch-Case)

```c
void fsm_run(event_t event) {
    switch (current_state) {
        case STATE_A:
            // Handle events in state A
            if (event == EVENT_X) {
                current_state = STATE_B;
            }
            break;
            
        case STATE_B:
            // Handle events in state B
            if (event == EVENT_Y) {
                current_state = STATE_A;
            }
            break;
    }
}
```

**Pros:** Simple, fast, easy to understand  
**Cons:** Can become large with many states

### Pattern 2: State Table

```c
typedef struct {
    state_t current_state;
    event_t event;
    state_t next_state;
    void (*action)(void);
} transition_t;

const transition_t transitions[] = {
    {STATE_A, EVENT_X, STATE_B, action_ab},
    {STATE_B, EVENT_Y, STATE_A, action_ba},
    // ...
};
```

**Pros:** Data-driven, easy to modify  
**Cons:** More memory, slightly slower

### Pattern 3: Function Pointer Table

```c
typedef void (*state_handler_t)(event_t);

state_handler_t state_handlers[] = {
    [STATE_A] = handle_state_a,
    [STATE_B] = handle_state_b,
    // ...
};

void fsm_run(event_t event) {
    state_handlers[current_state](event);
}
```

**Pros:** Very clean, extensible  
**Cons:** Requires function pointers

## 🏭 Industry Examples

### Automotive: Engine Control

```
CRANKING → STARTING → RUNNING → STOPPING → OFF
```

States handle:
- Fuel injection timing
- Ignition timing
- Sensor monitoring
- Error detection

### Medical: Infusion Pump

```
IDLE → PRIMING → INFUSING → PAUSED → ALARMING → STOPPED
```

Each state ensures:
- Patient safety
- Correct dosage
- Error handling
- Audit logging

### IoT: Smart Thermostat

```
OFF → HEATING → COOLING → AUTO → ECO_MODE
```

States manage:
- Temperature control
- Energy optimization
- User preferences
- Cloud sync

## 📏 Design Guidelines

### DO's ✅

1. **Keep States Simple**
   - Each state should have clear purpose
   - Avoid complex logic in states

2. **Use Enums for States**
   ```c
   typedef enum {
       STATE_IDLE,
       STATE_ACTIVE
   } state_t;
   ```

3. **Document Transitions**
   - Draw state diagrams
   - Comment transition conditions

4. **Handle All Events**
   - Every state should handle all possible events
   - Use default case for unexpected events

5. **Use Entry/Exit Actions**
   - Initialize on entry
   - Cleanup on exit

### DON'Ts ❌

1. **Don't Use Magic Numbers**
   ```c
   // BAD
   if (state == 3) { ... }
   
   // GOOD
   if (state == STATE_RUNNING) { ... }
   ```

2. **Don't Nest State Machines**
   - Use hierarchical state machines instead
   - Keep FSM flat when possible

3. **Don't Forget Error States**
   - Always have error handling
   - Provide recovery paths

4. **Don't Mix Business Logic**
   - Keep state machine logic separate
   - Actions should be in separate functions

## 🐛 Common Pitfalls

### Pitfall 1: Missing Transitions

```c
// BAD: What if EVENT_ERROR in STATE_IDLE?
switch (state) {
    case STATE_RUNNING:
        if (event == EVENT_ERROR) {
            state = STATE_ERROR;
        }
        break;
    // STATE_IDLE doesn't handle EVENT_ERROR!
}
```

**Solution:** Handle all events in all states (even if no action).

### Pitfall 2: Invalid State Transitions

```c
// BAD: Direct jump from IDLE to DONE
if (event == EVENT_SKIP) {
    state = STATE_DONE;  // Skipped important states!
}
```

**Solution:** Enforce valid transition paths.

### Pitfall 3: State Explosion

```c
// BAD: Too many states
STATE_IDLE_DOOR_OPEN
STATE_IDLE_DOOR_CLOSED
STATE_RUNNING_DOOR_OPEN
STATE_RUNNING_DOOR_CLOSED
// ... 50 more states
```

**Solution:** Use state variables or hierarchical FSM.

## 🎓 When to Use State Machines

### Perfect For ✅
- Protocol implementations (UART, TCP, etc.)
- User interface flows
- Device control (motors, pumps, etc.)
- Communication handlers
- Mode management
- Safety-critical systems

### Not Ideal For ❌
- Simple on/off control
- Continuous calculations
- Data processing pipelines
- Stateless operations

## 📊 State Machine vs If-Else

| Aspect | State Machine | If-Else |
|--------|---------------|---------|
| **Clarity** | High | Low (when complex) |
| **Testability** | Easy | Hard |
| **Maintainability** | Easy | Hard |
| **Visualization** | Yes (diagrams) | No |
| **Invalid States** | Impossible | Easy to create |
| **Code Size** | Moderate | Can be large |
| **Performance** | Fast | Fast |

## 🚀 Next Steps

Now that you understand the concept, let's see it in action:

1. **01_problem.md** - Real-world problem (washing machine)
2. **02_if_else_bad.c** - Bad approach (nested if-else)
3. **03_state_machine_good.c** - Good approach (FSM)
4. **04_production.c** - Industrial implementation
5. **05_exercises.md** - Practice problems

---

**Remember:** State machines are the backbone of embedded systems. Master this pattern, and you'll write cleaner, more maintainable code!
