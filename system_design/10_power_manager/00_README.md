# Power Manager

**The pattern for battery-powered embedded systems**

---

## 🎯 What Problem Does This Solve?

Battery-powered devices must optimize power consumption:
- Limited battery capacity
- Need for long runtime
- Multiple power states
- Wake-up sources

```c
// WRONG: Always running at full power
while (1) {
    read_sensors();      // Wastes power
    process_data();      // Even when idle
    update_display();    // Drains battery
    delay_ms(100);
}
```

**The naive solution — always-on — leads to:**
- Short battery life (hours instead of months)
- Frequent battery replacements
- Poor user experience
- High operational costs

**The solution: Power Manager**

Intelligently manage power states to maximize battery life while maintaining functionality.

---

## 🔧 How It Works

### Power States

```c
typedef enum {
    POWER_ACTIVE,       /* Full power, all peripherals on */
    POWER_IDLE,         /* CPU idle, peripherals on */
    POWER_SLEEP,        /* CPU + most peripherals off */
    POWER_DEEP_SLEEP    /* Minimal power, RTC only */
} power_state_t;
```

### Power State Transitions

```
ACTIVE (100mA)
   ↓ No activity for 5s
IDLE (50mA)
   ↓ No activity for 30s
SLEEP (5mA)
   ↓ No activity for 5min
DEEP_SLEEP (10µA)
   ↑ Button press / Timer / Sensor interrupt
ACTIVE
```

### Power Manager

```c
void power_manager(void) {
    uint32_t idle_time = get_idle_time();
    
    if (idle_time > DEEP_SLEEP_THRESHOLD) {
        power_enter_deep_sleep();
    } else if (idle_time > SLEEP_THRESHOLD) {
        power_enter_sleep();
    } else if (idle_time > IDLE_THRESHOLD) {
        power_enter_idle();
    } else {
        power_enter_active();
    }
}
```

---

## 📐 Power Optimization Strategies

### 1. Dynamic Voltage Scaling (DVS)
```c
void set_cpu_frequency(uint32_t freq_mhz) {
    switch (freq_mhz) {
        case 168:  /* Full speed: 100mA */
            RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
            break;
        case 84:   /* Half speed: 60mA */
            RCC->CFGR |= RCC_CFGR_HPRE_DIV2;
            break;
        case 42:   /* Quarter speed: 40mA */
            RCC->CFGR |= RCC_CFGR_HPRE_DIV4;
            break;
    }
}
```

### 2. Peripheral Power Gating
```c
void power_disable_unused_peripherals(void) {
    /* Disable unused peripherals */
    RCC->APB1ENR &= ~(RCC_APB1ENR_TIM2EN |
                      RCC_APB1ENR_TIM3EN |
                      RCC_APB1ENR_USART2EN);
    
    /* Disable unused GPIO */
    GPIOA->MODER = 0;  /* All pins as analog (lowest power) */
}
```

### 3. Sleep Modes
```c
void power_enter_sleep(void) {
    /* Configure wake-up sources */
    enable_wakeup_interrupt(BUTTON_PIN);
    enable_wakeup_interrupt(RTC_ALARM);
    
    /* Enter sleep mode */
    __WFI();  /* Wait For Interrupt */
    
    /* Wake up here */
    disable_wakeup_interrupt(BUTTON_PIN);
}
```

### 4. Duty Cycling
```c
void sensor_read_with_duty_cycle(void) {
    /* Power on sensor */
    sensor_power_on();
    delay_ms(10);  /* Stabilization time */
    
    /* Read sensor */
    int value = sensor_read();
    
    /* Power off sensor */
    sensor_power_off();
    
    /* Sleep until next reading */
    power_sleep_ms(1000);
}
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│          Application Tasks                  │
│  Sensors  Display  Communication            │
└──────────────┬──────────────────────────────┘
               │ power_request()
┌──────────────▼──────────────────────────────┐
│          Power Manager                      │
│  - Track activity                           │
│  - Determine power state                    │
│  - Manage transitions                       │
└──────────────┬──────────────────────────────┘
               │
       ┌───────┴────────┐
       ▼                ▼
┌─────────────┐  ┌─────────────┐
│ CPU/Clock   │  │ Peripherals │
│ Management  │  │ Power Gates │
└─────────────┘  └─────────────┘
```

---

## 📊 Power Consumption Examples

| State | Current | Use Case | Battery Life (2000mAh) |
|-------|---------|----------|------------------------|
| Active | 100mA | Processing | 20 hours |
| Idle | 50mA | Waiting | 40 hours |
| Sleep | 5mA | Periodic wake | 400 hours (16 days) |
| Deep Sleep | 10µA | Long idle | 20,000 hours (833 days) |

---

## 🔑 Key Takeaways

1. **Power States** — multiple levels (active, idle, sleep, deep sleep)
2. **Activity Tracking** — monitor idle time to determine state
3. **Wake Sources** — configure interrupts for wake-up
4. **Peripheral Gating** — disable unused peripherals
5. **Duty Cycling** — power on only when needed

---

## 🎯 Use Cases

**IoT Sensors:**
- Temperature/humidity monitors
- Motion detectors
- Door/window sensors
- Environmental monitoring

**Wearables:**
- Fitness trackers
- Smart watches
- Health monitors
- Location trackers

**Remote Devices:**
- Weather stations
- Wildlife cameras
- Parking sensors
- Asset trackers

**Medical:**
- Glucose monitors
- Heart rate monitors
- Pill dispensers
- Patient trackers

---

**Ready to see the problem?** → `01_problem.md`
