/**
 * 03_layered.c - GOOD EXAMPLE: Layered Architecture
 * 
 * This shows the RIGHT way to structure embedded code.
 * Code is organized into clear layers with defined responsibilities.
 * 
 * Benefits:
 * - Easy to test (can mock each layer)
 * - Easy to port (change HAL only)
 * - Easy to reuse (drivers are independent)
 * - Easy to maintain (clear structure)
 * - Team can work in parallel
 * 
 * Study time: 20 minutes
 * This is production-ready architecture!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
 * LAYER 1: HARDWARE ABSTRACTION LAYER (HAL)
 * 
 * Responsibility: Provide hardware-independent interface
 * Rules: 
 * - Thin wrapper around hardware
 * - No business logic
 * - Simple, direct operations
 * ============================================================================ */

/* HAL: GPIO */
typedef enum {
    GPIO_PORT_A,
    GPIO_PORT_B
} gpio_port_t;

void hal_gpio_init(gpio_port_t port, uint8_t pin, bool output) {
    /* Hardware-specific initialization */
    (void)port; (void)pin; (void)output;
    /* In real code: Configure registers */
}

void hal_gpio_write(gpio_port_t port, uint8_t pin, bool state) {
    /* Hardware-specific write */
    (void)port; (void)pin; (void)state;
    /* In real code: Set/clear register bit */
}

/* HAL: ADC */
void hal_adc_init(void) {
    /* Configure ADC hardware */
}

uint16_t hal_adc_read(uint8_t channel) {
    /* Read ADC value */
    (void)channel;
    return 512;  /* Simulated value */
}

/* HAL: UART */
void hal_uart_init(uint32_t baudrate) {
    /* Configure UART hardware */
    (void)baudrate;
}

void hal_uart_write_byte(uint8_t data) {
    /* Send one byte */
    (void)data;
}

bool hal_uart_is_ready(void) {
    /* Check if UART is ready */
    return true;
}

/* HAL: EEPROM */
void hal_eeprom_write(uint16_t address, uint8_t data) {
    /* Write to EEPROM */
    (void)address; (void)data;
}

uint8_t hal_eeprom_read(uint16_t address) {
    /* Read from EEPROM */
    (void)address;
    return 0;
}

/* ============================================================================
 * LAYER 2: DRIVER LAYER
 * 
 * Responsibility: Manage hardware devices
 * Rules:
 * - Uses HAL only (never touches hardware directly)
 * - Implements device protocols
 * - Manages device state
 * ============================================================================ */

/* Driver: Temperature Sensor */
#define TEMP_SENSOR_CHANNEL 0

void temp_sensor_init(void) {
    hal_adc_init();
}

float temp_sensor_read(void) {
    uint16_t adc_value = hal_adc_read(TEMP_SENSOR_CHANNEL);
    
    /* Convert ADC to temperature */
    float voltage = adc_value * 5.0f / 1024.0f;
    float temperature = (voltage - 0.5f) * 100.0f;
    
    return temperature;
}

/* Driver: LCD Display */
void lcd_init(void) {
    /* Initialize LCD pins */
    hal_gpio_init(GPIO_PORT_B, 2, true);  /* RS */
    hal_gpio_init(GPIO_PORT_B, 3, true);  /* EN */
    /* LCD initialization sequence */
}

void lcd_clear(void) {
    /* Clear LCD display */
}

void lcd_print(const char *text) {
    /* Print text to LCD */
    (void)text;
}

void lcd_display_temperature(float temp) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Temp: %.1f C", temp);
    lcd_clear();
    lcd_print(buffer);
}

/* Driver: Alarm */
#define ALARM_PIN 5

void alarm_init(void) {
    hal_gpio_init(GPIO_PORT_A, ALARM_PIN, true);
}

void alarm_activate(void) {
    hal_gpio_write(GPIO_PORT_A, ALARM_PIN, true);
}

void alarm_deactivate(void) {
    hal_gpio_write(GPIO_PORT_A, ALARM_PIN, false);
}

/* Driver: UART Communication */
void uart_init(void) {
    hal_uart_init(9600);
}

void uart_send_alert(float temperature) {
    if (hal_uart_is_ready()) {
        hal_uart_write_byte('A');  /* Alert marker */
        hal_uart_write_byte((uint8_t)temperature);
        hal_uart_write_byte('\n');
    }
}

/* Driver: Data Logger */
static uint16_t log_address = 0;

void logger_init(void) {
    log_address = 0;
}

void logger_write(uint8_t data) {
    hal_eeprom_write(log_address, data);
    log_address = (log_address + 1) % 256;
}

/* ============================================================================
 * LAYER 3: SERVICE LAYER
 * 
 * Responsibility: System-wide services
 * Rules:
 * - Reusable across applications
 * - No hardware knowledge
 * - Stateless when possible
 * ============================================================================ */

/* Service: Timer */
static uint32_t tick_count = 0;

void timer_tick(void) {
    tick_count++;
}

bool timer_elapsed(uint32_t *last_time, uint32_t interval) {
    if ((tick_count - *last_time) >= interval) {
        *last_time = tick_count;
        return true;
    }
    return false;
}

/* Service: Configuration */
#define TEMP_THRESHOLD 38.5f
#define LOG_INTERVAL 300  /* 5 minutes in seconds */

float config_get_temp_threshold(void) {
    return TEMP_THRESHOLD;
}

uint32_t config_get_log_interval(void) {
    return LOG_INTERVAL;
}

/* ============================================================================
 * LAYER 4: APPLICATION LAYER
 * 
 * Responsibility: Product-specific logic
 * Rules:
 * - Uses services and drivers
 * - Contains business logic
 * - Hardware-agnostic
 * ============================================================================ */

/* Application: Temperature Monitor */
static uint32_t last_log_time = 0;

void temperature_monitor_init(void) {
    /* Initialize all components */
    temp_sensor_init();
    lcd_init();
    alarm_init();
    uart_init();
    logger_init();
}

void temperature_monitor_task(void) {
    /* Read temperature */
    float temperature = temp_sensor_read();
    
    /* Display on LCD */
    lcd_display_temperature(temperature);
    
    /* Check alarm condition */
    if (temperature > config_get_temp_threshold()) {
        alarm_activate();
        uart_send_alert(temperature);
    } else {
        alarm_deactivate();
    }
    
    /* Log data periodically */
    if (timer_elapsed(&last_log_time, config_get_log_interval())) {
        logger_write((uint8_t)temperature);
    }
}

/* ============================================================================
 * MAIN APPLICATION
 * ============================================================================ */

int main(void) {
    /* Initialize system */
    temperature_monitor_init();
    
    /* Main loop */
    while(1) {
        temperature_monitor_task();
        timer_tick();
        
        /* Simulate 1 second delay */
        /* In real code: Use hardware timer or RTOS delay */
    }
    
    return 0;
}

/* ============================================================================
 * BENEFITS OF LAYERED ARCHITECTURE:
 * 
 * 1. PORTABILITY: Excellent
 *    - Change MCU? Replace HAL only
 *    - Application code unchanged
 *    - Drivers unchanged
 * 
 * 2. TESTABILITY: Easy
 *    - Mock HAL for unit tests
 *    - Test drivers independently
 *    - Test application logic without hardware
 * 
 * 3. MAINTAINABILITY: Clear
 *    - Each layer has clear responsibility
 *    - Easy to find and fix bugs
 *    - Changes isolated to one layer
 * 
 * 4. REUSABILITY: High
 *    - LCD driver works in any project
 *    - UART driver reusable
 *    - Services reusable
 * 
 * 5. TEAM WORK: Parallel
 *    - One person works on HAL
 *    - Another on drivers
 *    - Another on application
 *    - No conflicts!
 * 
 * 6. DEBUGGING: Isolated
 *    - Bug in temperature? Check sensor driver
 *    - Bug in display? Check LCD driver
 *    - Clear boundaries
 * 
 * 7. REQUIREMENTS CHANGES: Easy
 *    - Add WiFi? Create new driver
 *    - Change sensor? Replace sensor driver
 *    - Application logic unchanged
 * 
 * 8. CODE REVIEW: Simple
 *    - Review each layer independently
 *    - Clear interfaces
 *    - Easy to verify
 * 
 * 9. CERTIFICATION: Possible
 *    - Isolate safety-critical code
 *    - Prove correctness per layer
 *    - Auditor can verify
 * 
 * 10. PERFORMANCE: Optimizable
 *     - Optimize each layer independently
 *     - No side effects
 *     - Clear bottlenecks
 * ============================================================================ */

/*
 * COMPARISON WITH MONOLITHIC CODE:
 * 
 * Monolithic (02_monolithic.c):
 * - 150+ lines in one function
 * - Everything mixed together
 * - Impossible to test
 * - Impossible to port
 * - $340,000 in delays
 * 
 * Layered (this file):
 * - Clear structure
 * - Easy to test
 * - Easy to port
 * - Easy to maintain
 * - Production-ready
 * 
 * NEXT: See industrial-grade implementation with error handling!
 * Continue to: 04_production.c
 */
