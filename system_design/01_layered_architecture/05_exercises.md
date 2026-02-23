# Layered Architecture - Practice Exercises

Test your understanding of layered architecture with these hands-on exercises!

## 🎯 Exercise 1: Identify the Layers (Easy - 15 min)

Given this code, identify which layer each function belongs to:

```c
void motor_set_speed(uint8_t speed);
void hal_pwm_set_duty(uint8_t duty);
void vehicle_cruise_control(void);
void config_get_max_speed(void);
uint16_t hal_timer_read(void);
```

<details>
<summary>Solution</summary>

- `hal_pwm_set_duty()` → **HAL Layer** (hardware abstraction)
- `hal_timer_read()` → **HAL Layer** (hardware abstraction)
- `motor_set_speed()` → **Driver Layer** (device driver)
- `config_get_max_speed()` → **Service Layer** (configuration service)
- `vehicle_cruise_control()` → **Application Layer** (business logic)

**Layer Hierarchy:**
```
Application: vehicle_cruise_control()
    ↓
Service: config_get_max_speed()
    ↓
Driver: motor_set_speed()
    ↓
HAL: hal_pwm_set_duty(), hal_timer_read()
```
</details>

---

## 🎯 Exercise 2: Fix the Violations (Medium - 20 min)

This code violates layering principles. Fix it:

```c
// Application layer
void smart_thermostat_control(void) {
    // VIOLATION: Application touching hardware directly!
    uint16_t adc = ADC_REG;
    float temp = adc * 0.48828125;
    
    if (temp > 25.0) {
        // VIOLATION: Application calling HAL directly!
        hal_gpio_write(PORT_A, 3, true);
    }
}
```

<details>
<summary>Solution</summary>

```c
// HAL Layer
uint16_t hal_adc_read(uint8_t channel) {
    return ADC_REG;  // Hardware access isolated here
}

void hal_gpio_write(uint8_t port, uint8_t pin, bool state) {
    // Hardware access
}

// Driver Layer
float temp_sensor_read(void) {
    uint16_t adc = hal_adc_read(0);
    return adc * 0.48828125;  // Conversion in driver
}

void heater_control(bool on) {
    hal_gpio_write(PORT_A, 3, on);
}

// Application Layer
void smart_thermostat_control(void) {
    float temp = temp_sensor_read();  // Use driver
    
    if (temp > 25.0) {
        heater_control(false);  // Use driver
    } else {
        heater_control(true);
    }
}
```

**Key Fixes:**
1. ✅ Application uses drivers, not HAL
2. ✅ Hardware access isolated in HAL
3. ✅ Clear layer boundaries
4. ✅ Each layer has single responsibility
</details>

---

## 🎯 Exercise 3: Design a Layered System (Medium - 30 min)

**Scenario:** Design a smart door lock system with:
- Keypad input (12 keys)
- RFID reader
- Motor to lock/unlock
- LED indicator (red/green)
- Buzzer for feedback

**Task:** Design the layer structure. List functions for each layer.

<details>
<summary>Solution</summary>

```
┌─────────────────────────────────────┐
│     APPLICATION LAYER               │
│  - door_lock_system_init()          │
│  - door_lock_process_input()        │
│  - door_lock_check_access()         │
└─────────────────────────────────────┘
            ↓
┌─────────────────────────────────────┐
│     SERVICE LAYER                   │
│  - auth_verify_pin()                │
│  - auth_verify_rfid()               │
│  - logger_log_access()              │
│  - config_get_master_pin()          │
└─────────────────────────────────────┘
            ↓
┌─────────────────────────────────────┐
│     DRIVER LAYER                    │
│  - keypad_init()                    │
│  - keypad_get_key()                 │
│  - rfid_init()                      │
│  - rfid_read_card()                 │
│  - motor_init()                     │
│  - motor_lock()                     │
│  - motor_unlock()                   │
│  - led_init()                       │
│  - led_set_color()                  │
│  - buzzer_init()                    │
│  - buzzer_beep()                    │
└─────────────────────────────────────┘
            ↓
┌─────────────────────────────────────┐
│     HAL LAYER                       │
│  - hal_gpio_init()                  │
│  - hal_gpio_read()                  │
│  - hal_gpio_write()                 │
│  - hal_spi_init()                   │
│  - hal_spi_transfer()               │
│  - hal_pwm_init()                   │
│  - hal_pwm_set_duty()               │
└─────────────────────────────────────┘
```

**Example Implementation:**

```c
// Application Layer
void door_lock_process_input(void) {
    char key = keypad_get_key();
    
    if (key != 0) {
        if (auth_verify_pin(key)) {
            motor_unlock();
            led_set_color(LED_GREEN);
            buzzer_beep(100);
            logger_log_access("PIN", true);
        } else {
            led_set_color(LED_RED);
            buzzer_beep(500);
            logger_log_access("PIN", false);
        }
    }
    
    uint32_t card_id = rfid_read_card();
    if (card_id != 0) {
        if (auth_verify_rfid(card_id)) {
            motor_unlock();
            led_set_color(LED_GREEN);
            logger_log_access("RFID", true);
        }
    }
}
```
</details>

---

## 🎯 Exercise 4: Port to Different MCU (Hard - 45 min)

**Scenario:** You have code for ATmega328. Port it to STM32.

**Original (ATmega328):**
```c
// HAL for ATmega328
void hal_gpio_init(uint8_t pin) {
    DDRB |= (1 << pin);
}

void hal_gpio_write(uint8_t pin, bool state) {
    if (state) {
        PORTB |= (1 << pin);
    } else {
        PORTB &= ~(1 << pin);
    }
}

// Driver (unchanged)
void led_init(void) {
    hal_gpio_init(5);
}

void led_on(void) {
    hal_gpio_write(5, true);
}
```

**Task:** Create HAL for STM32 without changing driver/application.

<details>
<summary>Solution</summary>

```c
// HAL for STM32 (NEW - only this changes!)
#include "stm32f4xx.h"

void hal_gpio_init(uint8_t pin) {
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    
    // Configure as output
    GPIOB->MODER &= ~(3 << (pin * 2));
    GPIOB->MODER |= (1 << (pin * 2));
}

void hal_gpio_write(uint8_t pin, bool state) {
    if (state) {
        GPIOB->BSRR = (1 << pin);
    } else {
        GPIOB->BSRR = (1 << (pin + 16));
    }
}

// Driver (UNCHANGED!)
void led_init(void) {
    hal_gpio_init(5);
}

void led_on(void) {
    hal_gpio_write(5, true);
}

// Application (UNCHANGED!)
void main(void) {
    led_init();
    while(1) {
        led_on();
    }
}
```

**Key Points:**
- ✅ Only HAL changed
- ✅ Driver code identical
- ✅ Application code identical
- ✅ This is the power of layering!
</details>

---

## 🎯 Exercise 5: Add New Feature (Hard - 45 min)

**Scenario:** Existing temperature monitor system. Add WiFi alerts.

**Existing Code:**
```c
void temperature_monitor_task(void) {
    float temp = temp_sensor_read();
    lcd_display_temperature(temp);
    
    if (temp > THRESHOLD) {
        alarm_activate();
        uart_send_alert(temp);
    }
}
```

**Task:** Add WiFi without breaking existing code.

<details>
<summary>Solution</summary>

```c
// NEW: WiFi Driver Layer
void wifi_init(const char *ssid, const char *password) {
    // Initialize WiFi hardware
    hal_spi_init();
    // Connect to network
}

void wifi_send_alert(float temperature) {
    char message[64];
    snprintf(message, sizeof(message), 
             "ALERT: Temperature %.1f°C", temperature);
    
    // Send HTTP POST
    wifi_http_post("api.example.com/alert", message);
}

// NEW: Service Layer - Alert Manager
typedef enum {
    ALERT_UART,
    ALERT_WIFI,
    ALERT_BOTH
} alert_method_t;

void alert_manager_send(float temperature, alert_method_t method) {
    switch(method) {
        case ALERT_UART:
            uart_send_alert(temperature);
            break;
            
        case ALERT_WIFI:
            wifi_send_alert(temperature);
            break;
            
        case ALERT_BOTH:
            uart_send_alert(temperature);
            wifi_send_alert(temperature);
            break;
    }
}

// MODIFIED: Application Layer (minimal change)
void temperature_monitor_init(void) {
    temp_sensor_init();
    lcd_init();
    alarm_init();
    uart_init();
    wifi_init("MyNetwork", "password");  // NEW
}

void temperature_monitor_task(void) {
    float temp = temp_sensor_read();
    lcd_display_temperature(temp);
    
    if (temp > THRESHOLD) {
        alarm_activate();
        alert_manager_send(temp, ALERT_BOTH);  // CHANGED
    }
}
```

**Benefits:**
- ✅ Existing code mostly unchanged
- ✅ WiFi isolated in driver layer
- ✅ Alert manager provides flexibility
- ✅ Easy to add more alert methods
- ✅ Testable independently
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Layer Identification**
   - Recognize which code belongs in which layer
   - Understand layer responsibilities

2. **Violation Detection**
   - Spot when layers are violated
   - Know how to fix violations

3. **System Design**
   - Design layered systems from scratch
   - Choose appropriate layer for each function

4. **Portability**
   - Port code to different hardware
   - Minimize changes when porting

5. **Extensibility**
   - Add features without breaking existing code
   - Use layers to isolate changes

## 🚀 Next Steps

1. **Practice:** Refactor your own embedded code using layers
2. **Review:** Study open-source embedded projects (Zephyr, FreeRTOS)
3. **Apply:** Use layered architecture in your next project
4. **Continue:** Move to next design pattern (State Machines)

---

**Congratulations!** You've mastered Layered Architecture - the foundation of professional embedded systems!
