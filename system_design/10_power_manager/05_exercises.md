# Power Manager - Practice Exercises

Test your understanding of power management with these hands-on exercises!

---

## 🎯 Exercise 1: Calculate Battery Life (Easy - 10 min)

A sensor reads every 10 minutes, taking 2 seconds active (50mA), then sleeps (10µA).

**Task:** Calculate average current and battery life (2000mAh battery).

<details>
<summary>Solution</summary>

```
Reading interval: 600 seconds
Active time: 2 seconds (50mA)
Sleep time: 598 seconds (0.01mA)

Average current:
= (50mA × 2s + 0.01mA × 598s) / 600s
= (100 + 5.98) / 600
= 0.177mA

Battery life:
= 2000mAh / 0.177mA
= 11,299 hours
= 471 days
= 1.3 years
```

**Answer:** 1.3 years battery life
</details>

---

## 🎯 Exercise 2: Implement Power State Machine (Medium - 15 min)

Create state machine with 3 states: ACTIVE, IDLE, SLEEP.

**Task:** Implement state transitions based on idle time.

<details>
<summary>Solution</summary>

```c
typedef enum {
    POWER_ACTIVE,
    POWER_IDLE,
    POWER_SLEEP
} power_state_t;

#define IDLE_THRESHOLD_MS 5000
#define SLEEP_THRESHOLD_MS 30000

static power_state_t state = POWER_ACTIVE;
static uint32_t last_activity_ms = 0;

void power_manager(uint32_t current_ms) {
    uint32_t idle_time = current_ms - last_activity_ms;
    
    switch (state) {
        case POWER_ACTIVE:
            if (idle_time > IDLE_THRESHOLD_MS) {
                state = POWER_IDLE;
                printf("Transition: ACTIVE → IDLE\n");
            }
            break;
            
        case POWER_IDLE:
            if (idle_time > SLEEP_THRESHOLD_MS) {
                state = POWER_SLEEP;
                printf("Transition: IDLE → SLEEP\n");
            } else if (idle_time < IDLE_THRESHOLD_MS) {
                state = POWER_ACTIVE;
                printf("Transition: IDLE → ACTIVE\n");
            }
            break;
            
        case POWER_SLEEP:
            if (idle_time < SLEEP_THRESHOLD_MS) {
                state = POWER_IDLE;
                printf("Transition: SLEEP → IDLE\n");
            }
            break;
    }
}

void power_activity(uint32_t current_ms) {
    last_activity_ms = current_ms;
    if (state != POWER_ACTIVE) {
        state = POWER_ACTIVE;
        printf("Wake up: → ACTIVE\n");
    }
}
```

**State transitions:**
```
ACTIVE (idle > 5s) → IDLE
IDLE (idle > 30s) → SLEEP
SLEEP (activity) → ACTIVE
```
</details>

---

## 🎯 Exercise 3: Optimize Sensor Duty Cycle (Medium - 20 min)

Sensor consumes 100mA when on, 1µA when off. Need 1 reading/minute.

**Task:** Calculate optimal on-time to achieve 1 year battery life (2000mAh).

<details>
<summary>Solution</summary>

```
Target battery life: 1 year = 8760 hours
Battery capacity: 2000mAh
Required average current: 2000mAh / 8760h = 0.228mA

Reading interval: 60 seconds
Let active_time = time sensor is on (seconds)

Average current:
0.228mA = (100mA × active_time + 0.001mA × (60 - active_time)) / 60

Solve for active_time:
13.68 = 100 × active_time + 0.001 × (60 - active_time)
13.68 = 100 × active_time + 0.06 - 0.001 × active_time
13.62 = 99.999 × active_time
active_time = 0.136 seconds

Verification:
Avg = (100 × 0.136 + 0.001 × 59.864) / 60
    = (13.6 + 0.06) / 60
    = 0.228mA ✓

Battery life = 2000 / 0.228 = 8772 hours ≈ 1 year ✓
```

**Answer:** Sensor should be on for 136ms per reading
</details>

---

## 🎯 Exercise 4: Implement Wake Sources (Hard - 25 min)

Create power manager with multiple wake sources (button, timer, sensor).

**Task:** Implement wake-up handling.

<details>
<summary>Solution</summary>

```c
typedef enum {
    WAKE_NONE,
    WAKE_BUTTON,
    WAKE_TIMER,
    WAKE_SENSOR
} wake_source_t;

typedef struct {
    bool button_enabled;
    bool timer_enabled;
    bool sensor_enabled;
    uint32_t timer_interval_ms;
    uint32_t last_timer_ms;
} wake_config_t;

static wake_config_t wake_config = {
    .button_enabled = true,
    .timer_enabled = true,
    .sensor_enabled = true,
    .timer_interval_ms = 60000,  /* 1 minute */
    .last_timer_ms = 0
};

wake_source_t power_check_wake_sources(uint32_t current_ms) {
    /* Check button */
    if (wake_config.button_enabled && button_pressed()) {
        return WAKE_BUTTON;
    }
    
    /* Check timer */
    if (wake_config.timer_enabled) {
        uint32_t elapsed = current_ms - wake_config.last_timer_ms;
        if (elapsed >= wake_config.timer_interval_ms) {
            wake_config.last_timer_ms = current_ms;
            return WAKE_TIMER;
        }
    }
    
    /* Check sensor */
    if (wake_config.sensor_enabled && sensor_interrupt()) {
        return WAKE_SENSOR;
    }
    
    return WAKE_NONE;
}

void power_sleep_with_wakeup(void) {
    /* Configure wake sources */
    if (wake_config.button_enabled) {
        enable_gpio_interrupt(BUTTON_PIN);
    }
    if (wake_config.timer_enabled) {
        enable_rtc_alarm(wake_config.timer_interval_ms);
    }
    if (wake_config.sensor_enabled) {
        enable_sensor_interrupt();
    }
    
    /* Enter sleep */
    enter_sleep_mode();
    
    /* Wake up here */
    wake_source_t source = power_check_wake_sources(get_time_ms());
    
    switch (source) {
        case WAKE_BUTTON:
            printf("Woke by button\n");
            handle_button();
            break;
        case WAKE_TIMER:
            printf("Woke by timer\n");
            handle_periodic_task();
            break;
        case WAKE_SENSOR:
            printf("Woke by sensor\n");
            handle_sensor_event();
            break;
        default:
            break;
    }
}
```

**Benefits:**
- Multiple wake sources
- Configurable enables
- Source identification
- Flexible handling
</details>

---

## 🎯 Exercise 5: Power Budget Analysis (Hard - 30 min)

System has: MCU (50mA), Sensor (20mA), Radio (100mA), Display (30mA).

**Task:** Create power budget to achieve 1 month battery life (3000mAh).

<details>
<summary>Solution</summary>

```
Target: 1 month = 720 hours
Battery: 3000mAh
Required avg current: 3000 / 720 = 4.17mA

Components:
- MCU: 50mA active, 10µA sleep
- Sensor: 20mA active, 1µA sleep
- Radio: 100mA TX, 50mA RX, 1µA sleep
- Display: 30mA on, 0µA off

Strategy:
1. MCU: Active 1% (7.2h), Sleep 99% (712.8h)
   Avg = 50 × 0.01 + 0.01 × 0.99 = 0.51mA

2. Sensor: Read 1s every 5min (0.33%)
   Avg = 20 × 0.0033 + 0.001 × 0.9967 = 0.067mA

3. Radio: TX 1s every 15min (0.11%)
   Avg = 100 × 0.0011 + 0.001 × 0.9989 = 0.111mA

4. Display: On 10s every hour (0.28%)
   Avg = 30 × 0.0028 + 0 × 0.9972 = 0.084mA

Total average: 0.51 + 0.067 + 0.111 + 0.084 = 0.772mA

Battery life: 3000 / 0.772 = 3886 hours = 162 days = 5.4 months ✓

Power budget:
Component  | Duty Cycle | Avg Current | % of Budget
-----------|------------|-------------|------------
MCU        | 1.0%       | 0.51mA      | 66%
Radio      | 0.11%      | 0.111mA     | 14%
Display    | 0.28%      | 0.084mA     | 11%
Sensor     | 0.33%      | 0.067mA     | 9%
Total      |            | 0.772mA     | 100%
```

**Key insights:**
- MCU dominates power budget (66%)
- Aggressive sleep modes critical
- Radio usage must be minimized
- Display can be used sparingly
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Battery Life Calculation**
   - Average current = weighted sum
   - Battery life = capacity / avg current
   - Duty cycle optimization

2. **Power State Machine**
   - Multiple power states
   - Idle time thresholds
   - State transitions

3. **Duty Cycling**
   - Minimize active time
   - Maximize sleep time
   - Optimize for target lifetime

4. **Wake Sources**
   - Multiple wake triggers
   - Interrupt-driven wake
   - Source identification

5. **Power Budget**
   - Component analysis
   - Duty cycle allocation
   - Budget optimization

---

**Congratulations!** You've completed ALL 10 essential embedded system design patterns! 🎉

You now have the knowledge to build production-ready embedded systems!
