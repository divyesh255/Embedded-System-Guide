# Problem: Smart Washing Machine Controller

## 📋 The Scenario

You're hired by a home appliance company to develop the control software for their new **smart washing machine**. The machine must handle multiple wash cycles safely and efficiently.

### Requirements

**Basic Operations:**
1. Fill water to correct level
2. Wash clothes (rotate drum)
3. Drain water
4. Spin dry clothes
5. Complete cycle

**Safety Requirements:**
1. Never start if door is open
2. Lock door during operation
3. Stop immediately on error
4. Handle power failures gracefully
5. Emergency stop button

**User Features:**
1. Multiple wash programs (Normal, Delicate, Heavy)
2. Pause/Resume capability
3. Add clothes mid-cycle (if safe)
4. Display current status
5. Estimated time remaining

### Business Constraints
- **Time to Market:** 4 months
- **Cost:** Must use low-cost MCU
- **Reliability:** 10-year lifespan, 5000+ cycles
- **Safety:** Must pass UL certification
- **Competition:** Similar products already in market

## 🤔 Your First Attempt

You start coding with if-else statements:

```c
// washing_machine.c - Your first attempt

#include <stdbool.h>
#include <stdint.h>

// Global state variables
bool door_open = false;
bool water_filling = false;
bool water_full = false;
bool washing = false;
bool draining = false;
bool spinning = false;
bool paused = false;
bool error = false;
uint32_t wash_timer = 0;
uint32_t spin_timer = 0;

void washing_machine_control(void) {
    // Check door first
    if (door_open) {
        stop_motor();
        stop_water_valve();
        unlock_door();
        
        if (button_start_pressed) {
            // Can't start with door open
            beep_error();
        }
        
        if (button_pause_pressed) {
            // Already stopped
        }
        
        return;
    }
    
    // Handle pause
    if (paused) {
        stop_motor();
        
        if (button_resume_pressed) {
            paused = false;
            // But what state to resume to?
            if (water_filling) {
                // Resume filling
            } else if (washing) {
                // Resume washing
            } else if (draining) {
                // Resume draining
            }
            // This is getting messy...
        }
        
        if (button_stop_pressed) {
            // Reset everything
            water_filling = false;
            washing = false;
            draining = false;
            spinning = false;
            paused = false;
        }
        
        return;
    }
    
    // Handle filling
    if (water_filling) {
        open_water_valve();
        
        if (water_full) {
            close_water_valve();
            water_filling = false;
            washing = true;
            wash_timer = get_wash_duration();
            start_motor_slow();
        }
        
        if (button_pause_pressed) {
            paused = true;
            close_water_valve();
        }
        
        if (button_stop_pressed) {
            close_water_valve();
            water_filling = false;
            draining = true;
        }
        
        if (door_sensor_error) {
            error = true;
            close_water_valve();
            // What now?
        }
        
        return;
    }
    
    // Handle washing
    if (washing) {
        run_motor_wash_cycle();
        
        wash_timer--;
        if (wash_timer == 0) {
            stop_motor();
            washing = false;
            draining = true;
        }
        
        if (button_pause_pressed) {
            paused = true;
            stop_motor();
        }
        
        if (button_stop_pressed) {
            stop_motor();
            washing = false;
            draining = true;
        }
        
        if (motor_overload) {
            error = true;
            stop_motor();
            // Error handling?
        }
        
        return;
    }
    
    // Handle draining
    if (draining) {
        open_drain_valve();
        
        if (water_drained) {
            close_drain_valve();
            draining = false;
            spinning = true;
            spin_timer = get_spin_duration();
            start_motor_fast();
        }
        
        if (button_pause_pressed) {
            paused = true;
            close_drain_valve();
        }
        
        if (button_stop_pressed) {
            close_drain_valve();
            draining = false;
            // Done?
        }
        
        return;
    }
    
    // Handle spinning
    if (spinning) {
        run_motor_spin_cycle();
        
        spin_timer--;
        if (spin_timer == 0) {
            stop_motor();
            spinning = false;
            unlock_door();
            beep_done();
        }
        
        if (button_pause_pressed) {
            paused = true;
            stop_motor();
        }
        
        if (button_stop_pressed) {
            stop_motor();
            spinning = false;
            unlock_door();
        }
        
        if (vibration_excessive) {
            error = true;
            stop_motor();
            // Error state?
        }
        
        return;
    }
    
    // Handle error
    if (error) {
        stop_motor();
        close_water_valve();
        close_drain_valve();
        display_error();
        
        if (button_reset_pressed) {
            error = false;
            // But what state to go to?
        }
        
        return;
    }
    
    // Idle state (maybe?)
    if (button_start_pressed && !door_open) {
        lock_door();
        water_filling = true;
    }
}
```

## 😱 The Problems Start

### Week 2: Bug Reports

**Issue #1:** "Machine started with door open!"
- Investigation: Race condition between door check and start
- Root cause: Multiple boolean flags, no atomic state
- Fix: Add more checks... but where?

**Issue #2:** "Can't resume after pause"
- Problem: Lost track of which state we were in
- Tried to remember with more flags
- Created more bugs

**Issue #3:** "Machine stuck in washing mode"
- Timer didn't decrement in some code path
- Spent 2 days finding the bug
- Fixed one place, broke another

### Week 4: New Feature Request

**Manager:** "Add 'Quick Wash' mode"
- Need different timings for each state
- Current code has timings hardcoded everywhere
- Would need to modify 20+ places
- High risk of breaking existing modes

### Week 6: Safety Audit

**Safety Engineer:** "Show me all possible states"
- You: "Uh... there's filling, washing, draining..."
- Engineer: "What if filling AND washing are both true?"
- You: "That... shouldn't happen?"
- Engineer: "But CAN it happen?"
- You: "..." (realizes it can)

**Result:** Failed safety audit

### Week 8: Customer Complaints

**Issue:** "Machine flooded my house!"
- Water valve stuck open
- Error handling didn't close valve in all paths
- $50,000 damage claim
- Company reputation at risk

### Week 10: Code Review

**Senior Engineer:** "This is unmaintainable"
- 300+ lines in one function
- 8 boolean flags (256 possible combinations!)
- Only 6 valid states
- 250 invalid states possible!
- No way to visualize behavior
- Can't prove correctness

### Week 12: The Realization

**Cost of Poor Design:**
- 2 months behind schedule
- Failed safety certification
- Customer damage claims
- Team morale destroyed
- Product launch delayed

**Total Impact:** $500,000+ in delays and rework

## 💭 Think About It (5 minutes)

Before looking at the solution, ask yourself:

1. **What went wrong?**
   - Why are boolean flags problematic?
   - How many states are actually needed?
   - What makes this code hard to test?

2. **How would YOU fix it?**
   - How to ensure only valid states?
   - How to make transitions clear?
   - How to handle errors safely?

3. **What if...**
   - You need to add 5 more wash programs?
   - You need to support different languages?
   - You need to log all state changes?
   - Certification requires state diagram?

## 🎯 The Core Problems

### Problem 1: State Explosion
```c
// 8 boolean flags = 2^8 = 256 possible combinations
// But only 6 valid states!
// 250 ways to be in invalid state!
```

### Problem 2: No Clear Transitions
```c
// How do we get from WASHING to SPINNING?
// Code scattered across multiple if blocks
// Can't visualize the flow
```

### Problem 3: Error Handling Chaos
```c
// Error can happen in any state
// Each state handles errors differently
// Easy to forget error handling
// No consistent recovery
```

### Problem 4: Impossible to Test
```c
// How to test all 256 combinations?
// How to verify only valid states?
// How to test transitions?
// How to prove safety?
```

### Problem 5: Unmaintainable
```c
// Adding feature requires changes everywhere
// High risk of breaking existing code
// Can't understand flow without running
// New developers can't contribute
```

## 📊 Impact Analysis

| Issue | Time Lost | Cost Impact |
|-------|-----------|-------------|
| Bug fixing | 3 weeks | $30,000 |
| Failed audit | 4 weeks | $40,000 |
| Redesign | 6 weeks | $60,000 |
| Customer claims | - | $50,000 |
| Delayed launch | 3 months | $300,000 |
| **TOTAL** | **~4 months** | **$480,000** |

## 💡 The Solution Preview

What if the code looked like this instead?

```c
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

void state_machine_run(event_t event) {
    switch (current_state) {
        case STATE_IDLE:
            if (event == EVENT_START && !door_open) {
                lock_door();
                current_state = STATE_FILLING;
            }
            break;
            
        case STATE_FILLING:
            if (event == EVENT_WATER_FULL) {
                current_state = STATE_WASHING;
            } else if (event == EVENT_PAUSE) {
                current_state = STATE_PAUSED;
            } else if (event == EVENT_ERROR) {
                current_state = STATE_ERROR;
            }
            break;
            
        // Clear, testable, safe!
    }
}
```

**Benefits:**
- ✅ Only 8 valid states (not 256!)
- ✅ Clear transitions
- ✅ Easy to test
- ✅ Can draw diagram
- ✅ Safe by design

## 🚀 Next Steps

Now that you understand the problem, let's see the solution:

1. **02_if_else_bad.c** - The problematic code (what we just saw)
2. **03_state_machine_good.c** - Refactored with FSM
3. **04_production.c** - Industrial-grade implementation

---

**Key Takeaway:** Boolean flags create state explosion. State machines provide structure, safety, and clarity. This is why 90%+ of embedded products use state machines!

**Ready to see the solution?** Continue to the next file!
