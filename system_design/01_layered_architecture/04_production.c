/**
 * 04_production.c - PRODUCTION EXAMPLE: Industrial-Grade Layered Architecture
 * 
 * This shows PRODUCTION-READY embedded code with:
 * - Error handling
 * - Return codes
 * - Defensive programming
 * - Documentation
 * - MISRA-C compliance considerations
 * 
 * Study time: 25 minutes
 * This is how professionals write embedded code!
 */

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_INIT,
    STATUS_ERROR_TIMEOUT,
    STATUS_ERROR_INVALID_PARAM,
    STATUS_ERROR_HARDWARE
} status_t;

/* ============================================================================
 * LAYER 1: HAL WITH ERROR HANDLING
 * ============================================================================ */

/* HAL: GPIO with error checking */
status_t hal_gpio_init(uint8_t port, uint8_t pin, bool output) {
    if (port > 7 || pin > 7) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    /* Hardware initialization */
    return STATUS_OK;
}

status_t hal_gpio_write(uint8_t port, uint8_t pin, bool state) {
    if (port > 7 || pin > 7) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    /* Hardware write */
    (void)state;
    return STATUS_OK;
}

/* HAL: ADC with timeout */
status_t hal_adc_init(void) {
    /* Initialize ADC */
    return STATUS_OK;
}

status_t hal_adc_read(uint8_t channel, uint16_t *value) {
    if (value == NULL || channel > 15) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    
    /* Simulated read with timeout */
    *value = 512;
    return STATUS_OK;
}

/* ============================================================================
 * LAYER 2: DRIVERS WITH ERROR HANDLING
 * ============================================================================ */

/* Temperature Sensor Driver */
typedef struct {
    bool initialized;
    uint8_t channel;
    float last_reading;
} temp_sensor_t;

static temp_sensor_t temp_sensor = {0};

status_t temp_sensor_init(uint8_t channel) {
    status_t status;
    
    if (channel > 15) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    
    status = hal_adc_init();
    if (status != STATUS_OK) {
        return status;
    }
    
    temp_sensor.channel = channel;
    temp_sensor.initialized = true;
    temp_sensor.last_reading = 0.0f;
    
    return STATUS_OK;
}

status_t temp_sensor_read(float *temperature) {
    uint16_t adc_value;
    status_t status;
    
    if (!temp_sensor.initialized) {
        return STATUS_ERROR_INIT;
    }
    
    if (temperature == NULL) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    
    status = hal_adc_read(temp_sensor.channel, &adc_value);
    if (status != STATUS_OK) {
        return status;
    }
    
    /* Convert with bounds checking */
    float voltage = (float)adc_value * 5.0f / 1024.0f;
    *temperature = (voltage - 0.5f) * 100.0f;
    
    /* Sanity check */
    if (*temperature < -40.0f || *temperature > 125.0f) {
        return STATUS_ERROR_HARDWARE;
    }
    
    temp_sensor.last_reading = *temperature;
    return STATUS_OK;
}

/* Alarm Driver */
typedef struct {
    bool initialized;
    uint8_t port;
    uint8_t pin;
    bool active;
} alarm_t;

static alarm_t alarm = {0};

status_t alarm_init(uint8_t port, uint8_t pin) {
    status_t status;
    
    status = hal_gpio_init(port, pin, true);
    if (status != STATUS_OK) {
        return status;
    }
    
    alarm.port = port;
    alarm.pin = pin;
    alarm.active = false;
    alarm.initialized = true;
    
    return STATUS_OK;
}

status_t alarm_set_state(bool active) {
    status_t status;
    
    if (!alarm.initialized) {
        return STATUS_ERROR_INIT;
    }
    
    status = hal_gpio_write(alarm.port, alarm.pin, active);
    if (status != STATUS_OK) {
        return status;
    }
    
    alarm.active = active;
    return STATUS_OK;
}

bool alarm_is_active(void) {
    return alarm.active;
}

/* ============================================================================
 * LAYER 3: SERVICES
 * ============================================================================ */

/* Configuration Service */
typedef struct {
    float temp_threshold;
    uint32_t log_interval;
    bool initialized;
} config_t;

static config_t config = {
    .temp_threshold = 38.5f,
    .log_interval = 300,
    .initialized = false
};

status_t config_init(void) {
    /* Load from EEPROM or use defaults */
    config.initialized = true;
    return STATUS_OK;
}

status_t config_get_temp_threshold(float *threshold) {
    if (!config.initialized || threshold == NULL) {
        return STATUS_ERROR_INVALID_PARAM;
    }
    *threshold = config.temp_threshold;
    return STATUS_OK;
}

/* ============================================================================
 * LAYER 4: APPLICATION WITH FULL ERROR HANDLING
 * ============================================================================ */

typedef enum {
    APP_STATE_INIT,
    APP_STATE_RUNNING,
    APP_STATE_ERROR
} app_state_t;

typedef struct {
    app_state_t state;
    uint32_t error_count;
    uint32_t tick_count;
} app_context_t;

static app_context_t app = {
    .state = APP_STATE_INIT,
    .error_count = 0,
    .tick_count = 0
};

status_t app_init(void) {
    status_t status;
    
    /* Initialize configuration */
    status = config_init();
    if (status != STATUS_OK) {
        return status;
    }
    
    /* Initialize temperature sensor */
    status = temp_sensor_init(0);
    if (status != STATUS_OK) {
        return status;
    }
    
    /* Initialize alarm */
    status = alarm_init(0, 5);
    if (status != STATUS_OK) {
        return status;
    }
    
    app.state = APP_STATE_RUNNING;
    return STATUS_OK;
}

status_t app_task(void) {
    status_t status;
    float temperature;
    float threshold;
    
    if (app.state != APP_STATE_RUNNING) {
        return STATUS_ERROR_INIT;
    }
    
    /* Read temperature with error handling */
    status = temp_sensor_read(&temperature);
    if (status != STATUS_OK) {
        app.error_count++;
        if (app.error_count > 10) {
            app.state = APP_STATE_ERROR;
            return STATUS_ERROR_HARDWARE;
        }
        return status;
    }
    
    /* Reset error count on success */
    app.error_count = 0;
    
    /* Get threshold */
    status = config_get_temp_threshold(&threshold);
    if (status != STATUS_OK) {
        return status;
    }
    
    /* Control alarm */
    if (temperature > threshold) {
        status = alarm_set_state(true);
    } else {
        status = alarm_set_state(false);
    }
    
    if (status != STATUS_OK) {
        return status;
    }
    
    app.tick_count++;
    return STATUS_OK;
}

/* ============================================================================
 * MAIN WITH ERROR HANDLING
 * ============================================================================ */

int main(void) {
    status_t status;
    
    /* Initialize application */
    status = app_init();
    if (status != STATUS_OK) {
        /* Handle initialization error */
        /* In production: Log error, enter safe state */
        while(1) {
            /* Error state */
        }
    }
    
    /* Main loop */
    while(1) {
        status = app_task();
        if (status != STATUS_OK) {
            /* Handle runtime error */
            /* In production: Log, attempt recovery */
        }
        
        /* Delay 1 second */
    }
    
    return 0;
}

/* ============================================================================
 * PRODUCTION-GRADE FEATURES:
 * 
 * 1. ERROR HANDLING
 *    - Every function returns status
 *    - Errors propagate up
 *    - Graceful degradation
 * 
 * 2. DEFENSIVE PROGRAMMING
 *    - NULL pointer checks
 *    - Parameter validation
 *    - Bounds checking
 *    - Initialization checks
 * 
 * 3. STATE MANAGEMENT
 *    - Clear state machines
 *    - State validation
 *    - Error recovery
 * 
 * 4. DOCUMENTATION
 *    - Clear comments
 *    - Function contracts
 *    - Layer boundaries
 * 
 * 5. TESTABILITY
 *    - Each function testable
 *    - Clear interfaces
 *    - Mockable layers
 * 
 * 6. MAINTAINABILITY
 *    - Consistent style
 *    - Clear structure
 *    - Easy to modify
 * 
 * 7. SAFETY
 *    - No undefined behavior
 *    - Fail-safe defaults
 *    - Error counting
 * 
 * 8. PORTABILITY
 *    - Hardware isolated in HAL
 *    - Standard C types
 *    - No platform assumptions
 * ============================================================================ */
