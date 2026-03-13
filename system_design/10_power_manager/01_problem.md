# Problem: Wireless Sensor — Battery Drains in Days

## 📋 The Scenario

You're building a **wireless temperature sensor** for smart buildings:

### Requirements
1. **Temperature monitoring** (every 5 minutes)
2. **Wireless transmission** (LoRa, every 15 minutes)
3. **Battery powered** (2× AA batteries, 3000mAh)
4. **Target lifetime** (2 years without battery change)
5. **Cost** ($20 per unit, 1000 units deployed)

### Business Constraints
- **Battery Life:** Must last 2 years (17,520 hours)
- **Maintenance:** Battery replacement costs $50/visit
- **Reliability:** 99% uptime required
- **Cost:** Premature battery failure = $50K/year

## 🤔 Your First Attempt

You start without power management:

```c
// sensor.c - First attempt without power management

void main_loop(void) {
    while (1) {
        // Always running at full power!
        int temp = read_temperature();
        
        if (should_transmit()) {
            transmit_data(temp);
        }
        
        delay_ms(5000);  // 5 second delay
    }
}
```

## 😱 The Problems Start

### Week 1: Battery Drain Test

**Test setup:** Deploy 10 sensors, measure battery life

**Results:**
```
Sensor always-on current: 50mA
Battery capacity: 3000mAh
Battery life: 3000mAh / 50mA = 60 hours = 2.5 days!
```

**Impact:** Sensors die in 2.5 days instead of 2 years! 292× worse than target!

### Week 2: The Math Doesn't Work

**Cost analysis:**
```
Target: 2 years (730 days)
Actual: 2.5 days
Battery changes per year: 730 / 2.5 = 292 changes

Cost per battery change: $50
Annual cost per sensor: 292 × $50 = $14,600
Total cost (1000 sensors): $14,600,000/year!

This is 730× the product cost!
```

**Business impact:** Project cancelled. Product not viable.

### Week 3: Attempt #2 - Longer Delays

**Idea:** Increase delay between readings

```c
void main_loop(void) {
    while (1) {
        int temp = read_temperature();
        
        if (should_transmit()) {
            transmit_data(temp);
        }
        
        delay_ms(300000);  // 5 minute delay
    }
}
```

**Results:**
```
Still consuming 50mA during delay!
Battery life: Still only 60 hours
```

**Problem:** CPU still running at full power during delay. No improvement!

### Month 2: Competitor Analysis

**Competitor's sensor:**
```
Battery life: 2+ years
How? Power management!

Active (reading): 50mA for 1 second
Sleep (idle): 10µA for 299 seconds
Average: (50mA × 1s + 0.01mA × 299s) / 300s = 0.18mA

Battery life: 3000mAh / 0.18mA = 16,667 hours = 694 days ≈ 2 years!
```

**Realization:** Need power management to compete!

## 💭 Think About It (5 minutes)

1. **Why does always-on waste power?**
   - CPU running when idle
   - Peripherals powered unnecessarily
   - No sleep modes used

2. **What are power states?**
   - Active: Full power
   - Idle: CPU stopped, peripherals on
   - Sleep: CPU + peripherals off
   - Deep sleep: Minimal power

3. **How to save power?**
   - Sleep when idle
   - Duty cycle peripherals
   - Wake on interrupt
   - Disable unused peripherals

## 🎯 The Core Problems

### Problem 1: Always-On CPU
```c
while (1) {
    delay_ms(5000);  // CPU running at full power!
}
```

### Problem 2: Always-On Peripherals
```c
// Sensor powered 24/7
// Display powered 24/7
// Radio powered 24/7
```

### Problem 3: No Sleep Modes
```c
// Never enters sleep
// Wastes 99% of battery on idle
```

### Problem 4: No Activity Tracking
```c
// Can't determine when to sleep
// No idle time measurement
```

## 📊 Impact Analysis

| Issue | Power Waste | Battery Life Impact |
|-------|-------------|---------------------|
| Always-on CPU | 50mA | 60 hours (2.5 days) |
| Always-on peripherals | +20mA | 43 hours (1.8 days) |
| No sleep modes | 99% waste | 292× worse than target |
| Total | 70mA | Unusable product |

## 💡 The Solution Preview

What if system could sleep when idle and wake on demand?

```c
// Power manager — intelligent power states

typedef enum {
    POWER_ACTIVE,       /* 50mA - reading/transmitting */
    POWER_SLEEP         /* 10µA - waiting */
} power_state_t;

void main_loop(void) {
    while (1) {
        /* Wake up */
        power_set_state(POWER_ACTIVE);
        
        /* Read sensor (1 second) */
        int temp = read_temperature();
        
        /* Transmit if needed */
        if (should_transmit()) {
            transmit_data(temp);
        }
        
        /* Sleep until next reading (5 minutes) */
        power_set_state(POWER_SLEEP);
        power_sleep_until(next_reading_time);
    }
}

// Power consumption:
// Active: 50mA × 1s = 50mAs
// Sleep: 0.01mA × 299s = 2.99mAs
// Average: (50 + 2.99) / 300 = 0.18mA
// Battery life: 3000mAh / 0.18mA = 16,667 hours = 2 years!
```

**Benefits:**
- ✅ 278× power reduction (50mA → 0.18mA)
- ✅ 2 year battery life (target met!)
- ✅ $50K/year savings (no premature replacements)
- ✅ Competitive product
- ✅ Customer satisfaction

## 🚀 Next Steps

1. **02_always_on_bad.c** — Always-on simulation (2.5 day battery life)
2. **03_power_manager_good.c** — Power manager solution (2 year battery life)
3. **04_production.c** — Production-grade with multiple power states

---

**Key Takeaway:** Without power management, battery-powered devices are not viable. Power manager enables 278× power reduction through intelligent sleep modes, making 2-year battery life achievable.
