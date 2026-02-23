# Layered Architecture - Professional Code Organization

**Study Time:** 45 minutes  
**Difficulty:** Beginner  
**Industry Use:** Universal - Used in ALL production embedded systems

## 🎯 What You'll Learn

- Why spaghetti code kills embedded projects
- How to structure code like professionals
- Hardware Abstraction Layer (HAL) concept
- Separation of concerns in embedded systems
- Making code portable and maintainable

## 📖 What is Layered Architecture?

**Layered Architecture** is the practice of organizing code into distinct horizontal layers, where each layer has a specific responsibility and only communicates with adjacent layers.

### Real-World Analogy

Think of a restaurant:
- **Kitchen (Hardware)** - The actual cooking equipment
- **Cooks (HAL)** - Know how to operate equipment
- **Head Chef (Driver Layer)** - Coordinates cooking tasks
- **Waiters (Service Layer)** - Interface with customers
- **Customers (Application)** - Order food, don't care about kitchen details

Each layer has clear responsibilities and interfaces.

## 🏭 Industry Standard: The 5-Layer Model

```
┌─────────────────────────────────────┐
│     APPLICATION LAYER               │  ← Business logic
│  (What the product does)            │
├─────────────────────────────────────┤
│     SERVICE LAYER                   │  ← System services
│  (Diagnostics, Logging, Config)    │
├─────────────────────────────────────┤
│     DRIVER LAYER                    │  ← Device drivers
│  (UART, SPI, I2C, CAN drivers)     │
├─────────────────────────────────────┤
│     HAL (Hardware Abstraction)      │  ← Hardware interface
│  (GPIO, Timers, ADC, PWM)          │
├─────────────────────────────────────┤
│     HARDWARE                        │  ← Physical MCU
│  (Registers, Peripherals)          │
└─────────────────────────────────────┘
```

## 🤔 Why Layered Architecture?

### Without Layers (Spaghetti Code)
```c
void read_temperature() {
    // Application logic mixed with hardware
    PORTA |= (1 << 3);           // What does this do?
    _delay_ms(10);
    uint16_t adc = ADC;
    float temp = (adc * 5.0 / 1024.0 - 0.5) * 100;
    
    if (temp > 80) {
        PORTB |= (1 << 5);       // What's this?
        uart_send_byte(0x41);    // Magic number?
    }
}
```

**Problems:**
- ❌ Can't port to different MCU
- ❌ Can't test without hardware
- ❌ Hard to understand
- ❌ Difficult to maintain
- ❌ Can't reuse code

### With Layers (Professional)
```c
// Application Layer
void read_temperature() {
    float temp = temperature_sensor_read();
    
    if (temp > TEMP_THRESHOLD) {
        alarm_activate();
        logger_log("High temperature: %.1f°C", temp);
    }
}

// Driver Layer
float temperature_sensor_read() {
    uint16_t adc_value = adc_read_channel(TEMP_SENSOR_CHANNEL);
    return convert_adc_to_celsius(adc_value);
}

// HAL Layer
uint16_t adc_read_channel(uint8_t channel) {
    // Hardware-specific code isolated here
    return HAL_ADC_Read(channel);
}
```

**Benefits:**
- ✅ Clear responsibilities
- ✅ Easy to port
- ✅ Testable
- ✅ Maintainable
- ✅ Reusable

## 📊 Layer Responsibilities

### 1. Hardware Layer
- **What:** Physical MCU registers and peripherals
- **Who touches it:** Only HAL
- **Example:** `PORTA`, `ADC`, `UART0`

### 2. HAL (Hardware Abstraction Layer)
- **What:** Thin wrapper around hardware
- **Responsibility:** Provide hardware-independent interface
- **Example:** `gpio_set_pin()`, `adc_read()`, `uart_write()`
- **Rules:**
  - No business logic
  - Simple, direct hardware access
  - One function per hardware operation

### 3. Driver Layer
- **What:** Device-specific drivers
- **Responsibility:** Manage hardware devices
- **Example:** `temperature_sensor_init()`, `motor_set_speed()`
- **Rules:**
  - Uses HAL, never touches hardware directly
  - Implements device protocols
  - Handles device state

### 4. Service Layer
- **What:** System-wide services
- **Responsibility:** Cross-cutting concerns
- **Example:** `logger_log()`, `config_get()`, `diagnostics_run()`
- **Rules:**
  - Reusable across applications
  - No hardware knowledge
  - Stateless when possible

### 5. Application Layer
- **What:** Product-specific logic
- **Responsibility:** Implement product features
- **Example:** `washing_machine_run()`, `thermostat_control()`
- **Rules:**
  - Uses services and drivers
  - Contains business logic
  - Hardware-agnostic

## 🎯 Communication Rules

### The Golden Rules

1. **Downward Calls Only**
   - Application → Service → Driver → HAL → Hardware
   - Never call upward (HAL can't call Application)

2. **Adjacent Layers Only**
   - Application calls Service or Driver
   - Application NEVER calls HAL directly
   - Keeps coupling low

3. **Callbacks for Upward Communication**
   - Use callbacks/events for hardware → application
   - Example: Interrupt → HAL → Driver callback → Application

### Example: Proper Layer Communication

```c
// ✅ CORRECT
void application_task() {
    // Application calls Driver
    float temp = temperature_sensor_read();
    
    // Application calls Service
    logger_log("Temperature: %.1f", temp);
}

// ❌ WRONG
void application_task() {
    // Application calling HAL directly - BAD!
    uint16_t adc = adc_read_channel(3);
    
    // Application touching hardware - TERRIBLE!
    PORTA |= (1 << 5);
}
```

## 🏗️ Real Example: LED Control

### Bad Design (No Layers)
```c
void blink_led() {
    while(1) {
        PORTB |= (1 << 5);   // Turn on
        _delay_ms(500);
        PORTB &= ~(1 << 5);  // Turn off
        _delay_ms(500);
    }
}
```

### Good Design (Layered)

```c
// HAL Layer (hal_gpio.h)
void gpio_set_pin(uint8_t port, uint8_t pin, bool state);

// Driver Layer (led_driver.h)
void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

// Application Layer
void blink_led() {
    while(1) {
        led_on();
        delay_ms(500);
        led_off();
        delay_ms(500);
    }
}
```

**Why better?**
- Change LED pin? Modify only driver
- Port to different MCU? Modify only HAL
- Test without hardware? Mock the driver
- Reuse LED driver? Copy driver file

## 💡 Industry Examples

### Automotive (AUTOSAR)
```
Application Layer    → Engine Control Logic
RTE (Runtime Env)    → Service Layer
BSW (Basic Software) → Drivers + HAL
MCAL                 → Hardware Abstraction
Hardware             → ECU
```

### Medical Devices
```
Application → Patient Monitoring
Service     → Data Logging, Alarms
Driver      → Sensor Drivers
HAL         → MCU Peripherals
Hardware    → Medical-grade MCU
```

### IoT Devices
```
Application → Smart Home Logic
Service     → Cloud Communication
Driver      → WiFi, Sensors
HAL         → ESP32 HAL
Hardware    → ESP32
```

## 📏 Design Guidelines

### HAL Guidelines
- ✅ One function per hardware operation
- ✅ Return error codes
- ✅ No delays or blocking
- ✅ Minimal logic
- ❌ No business logic
- ❌ No device knowledge

### Driver Guidelines
- ✅ Manage device state
- ✅ Implement protocols
- ✅ Use HAL only
- ✅ Provide clean API
- ❌ No hardware access
- ❌ No application logic

### Application Guidelines
- ✅ Implement features
- ✅ Use drivers/services
- ✅ Handle user interaction
- ❌ No hardware knowledge
- ❌ No HAL calls

## 🎓 Benefits Summary

| Benefit | Description |
|---------|-------------|
| **Portability** | Change MCU by replacing HAL only |
| **Testability** | Mock layers for unit testing |
| **Maintainability** | Clear structure, easy to navigate |
| **Reusability** | Drivers work across projects |
| **Team Work** | Different teams own different layers |
| **Debugging** | Isolate issues to specific layer |

## 🚀 Next Steps

Now that you understand the concept, let's see it in action:

1. **01_problem.md** - See the real problem this solves
2. **02_monolithic.c** - Bad example (no layers)
3. **03_layered.c** - Good example (with layers)
4. **04_production.c** - Industrial implementation
5. **05_exercises.md** - Practice problems

---

**Remember:** Layered architecture is the foundation of ALL professional embedded systems. Master this, and everything else becomes easier!
