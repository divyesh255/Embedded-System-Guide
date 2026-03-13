# Error Handler - Practice Exercises

Test your understanding of error handling with these hands-on exercises!

---

## 🎯 Exercise 1: Design Error Codes (Easy - 10 min)

Design error codes for a temperature monitoring system with 3 sensors.

**Task:** Define status codes and error types.

<details>
<summary>Solution</summary>

```c
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_TIMEOUT = -1,
    STATUS_ERROR_INVALID = -2,
    STATUS_ERROR_DISCONNECTED = -3,
    STATUS_ERROR_CALIBRATION = -4
} status_t;

typedef enum {
    ERROR_SENSOR_1,
    ERROR_SENSOR_2,
    ERROR_SENSOR_3,
    ERROR_COMMUNICATION,
    ERROR_HARDWARE
} error_type_t;

// Usage:
status_t read_sensor(int sensor_id, float *temp) {
    if (!sensor_connected(sensor_id)) {
        return STATUS_ERROR_DISCONNECTED;
    }
    
    if (sensor_timeout(sensor_id)) {
        return STATUS_ERROR_TIMEOUT;
    }
    
    *temp = sensor_read(sensor_id);
    
    if (*temp < -50.0 || *temp > 150.0) {
        return STATUS_ERROR_INVALID;
    }
    
    return STATUS_OK;
}
```

**Benefits:**
- Clear error identification
- Easy to check and handle
- Self-documenting code
</details>

---

## 🎯 Exercise 2: Implement Error Context (Medium - 15 min)

Add file, line, and function information to errors.

**Task:** Create ERROR_REPORT macro with context.

<details>
<summary>Solution</summary>

```c
typedef struct {
    status_t code;
    const char *file;
    int line;
    const char *function;
    const char *message;
} error_context_t;

#define ERROR_REPORT(severity, type, msg) \
    error_report_with_context(severity, type, msg, \
                              __FILE__, __LINE__, __func__)

void error_report_with_context(error_severity_t severity,
                               error_type_t type,
                               const char *message,
                               const char *file,
                               int line,
                               const char *function) {
    printf("[ERROR] %s:%d in %s()\n", file, line, function);
    printf("  Severity: %d, Type: %d\n", severity, type);
    printf("  Message: %s\n", message);
    
    /* Log to buffer */
    log_error(severity, type, message, file, line, function);
    
    /* Take action */
    handle_error(severity, type);
}

// Usage:
if (read_sensor(&temp) != STATUS_OK) {
    ERROR_REPORT(ERROR_RECOVERABLE, ERROR_SENSOR,
                 "Temperature sensor timeout");
}
```

**Output:**
```
[ERROR] main.c:42 in read_temperature()
  Severity: 2, Type: 0
  Message: Temperature sensor timeout
```

**Benefits:**
- Exact error location
- Easy debugging
- Complete context
</details>

---

## 🎯 Exercise 3: Implement Error Recovery (Medium - 20 min)

Create recovery strategies for different error types.

**Task:** Implement recovery with retry logic.

<details>
<summary>Solution</summary>

```c
typedef struct {
    error_type_t type;
    uint32_t retry_count;
    uint32_t retry_delay_ms;
    bool (*recovery_fn)(void);
} recovery_strategy_t;

static bool recover_sensor(void) {
    printf("Recovering sensor: reinitializing...\n");
    return sensor_init() == STATUS_OK;
}

static bool recover_communication(void) {
    printf("Recovering communication: reconnecting...\n");
    return comm_reconnect() == STATUS_OK;
}

static bool recover_hardware(void) {
    printf("Recovering hardware: power cycle...\n");
    hardware_power_cycle();
    delay_ms(100);
    return hardware_init() == STATUS_OK;
}

static recovery_strategy_t strategies[] = {
    { ERROR_SENSOR,         3, 100, recover_sensor },
    { ERROR_COMMUNICATION,  5, 500, recover_communication },
    { ERROR_HARDWARE,       1, 1000, recover_hardware }
};

bool attempt_recovery(error_type_t type) {
    /* Find strategy */
    recovery_strategy_t *strategy = NULL;
    for (int i = 0; i < NUM_STRATEGIES; i++) {
        if (strategies[i].type == type) {
            strategy = &strategies[i];
            break;
        }
    }
    
    if (!strategy) return false;
    
    /* Retry with delay */
    for (uint32_t i = 0; i < strategy->retry_count; i++) {
        printf("Recovery attempt %u/%u\n", i + 1, strategy->retry_count);
        
        if (strategy->recovery_fn()) {
            printf("Recovery successful!\n");
            return true;
        }
        
        if (i < strategy->retry_count - 1) {
            delay_ms(strategy->retry_delay_ms);
        }
    }
    
    printf("Recovery failed after %u attempts\n", strategy->retry_count);
    return false;
}
```

**Benefits:**
- Configurable retry logic
- Per-type strategies
- Automatic recovery
</details>

---

## 🎯 Exercise 4: Implement Error Log (Hard - 25 min)

Create circular buffer error log with persistence.

**Task:** Implement error log with flash storage.

<details>
<summary>Solution</summary>

```c
#define MAX_ERROR_LOG 64

typedef struct {
    uint32_t timestamp;
    error_severity_t severity;
    error_type_t type;
    char message[64];
    char file[32];
    uint16_t line;
} error_log_entry_t;

typedef struct {
    error_log_entry_t entries[MAX_ERROR_LOG];
    uint32_t write_index;
    uint32_t count;
    uint32_t total_errors;
} error_log_t;

static error_log_t error_log = {0};

void error_log_add(error_severity_t severity,
                   error_type_t type,
                   const char *message,
                   const char *file,
                   uint16_t line) {
    error_log_entry_t *entry = &error_log.entries[error_log.write_index];
    
    entry->timestamp = get_timestamp();
    entry->severity = severity;
    entry->type = type;
    strncpy(entry->message, message, sizeof(entry->message) - 1);
    strncpy(entry->file, file, sizeof(entry->file) - 1);
    entry->line = line;
    
    error_log.write_index = (error_log.write_index + 1) % MAX_ERROR_LOG;
    if (error_log.count < MAX_ERROR_LOG) {
        error_log.count++;
    }
    error_log.total_errors++;
    
    /* Persist to flash every 10 errors */
    if (error_log.total_errors % 10 == 0) {
        error_log_save_to_flash();
    }
}

void error_log_save_to_flash(void) {
    flash_write(ERROR_LOG_ADDR, &error_log, sizeof(error_log));
}

void error_log_load_from_flash(void) {
    flash_read(ERROR_LOG_ADDR, &error_log, sizeof(error_log));
}

void error_log_print(void) {
    printf("\n=== Error Log (%u/%u entries, %u total) ===\n",
           error_log.count, MAX_ERROR_LOG, error_log.total_errors);
    
    for (uint32_t i = 0; i < error_log.count; i++) {
        error_log_entry_t *entry = &error_log.entries[i];
        printf("[%u] %s:%u - %s\n",
               entry->timestamp,
               entry->file,
               entry->line,
               entry->message);
    }
}

// Usage:
void main(void) {
    /* Load previous errors from flash */
    error_log_load_from_flash();
    
    /* Run system */
    while (1) {
        if (error_occurred()) {
            error_log_add(ERROR_RECOVERABLE, ERROR_SENSOR,
                         "Sensor timeout", __FILE__, __LINE__);
        }
    }
}
```

**Benefits:**
- Survives power loss
- Circular buffer (no overflow)
- Complete error history
- Remote diagnostics
</details>

---

## 🎯 Exercise 5: Implement Error Callbacks (Hard - 30 min)

Create callback system for error notifications.

**Task:** Implement error callback registration and notification.

<details>
<summary>Solution</summary>

```c
typedef void (*error_callback_t)(error_severity_t severity,
                                 error_type_t type,
                                 const char *message);

#define MAX_CALLBACKS 8
static error_callback_t callbacks[MAX_CALLBACKS];
static uint32_t callback_count = 0;

bool error_register_callback(error_callback_t callback) {
    if (callback_count >= MAX_CALLBACKS) {
        return false;
    }
    
    callbacks[callback_count++] = callback;
    return true;
}

void error_notify_callbacks(error_severity_t severity,
                            error_type_t type,
                            const char *message) {
    for (uint32_t i = 0; i < callback_count; i++) {
        callbacks[i](severity, type, message);
    }
}

void error_report(error_severity_t severity,
                  error_type_t type,
                  const char *message) {
    /* Log error */
    error_log_add(severity, type, message);
    
    /* Notify callbacks */
    error_notify_callbacks(severity, type, message);
    
    /* Take action */
    handle_error(severity, type);
}

// Example callbacks:
void led_error_callback(error_severity_t severity,
                       error_type_t type,
                       const char *message) {
    if (severity >= ERROR_RECOVERABLE) {
        led_set(LED_ERROR, true);
    }
}

void cloud_error_callback(error_severity_t severity,
                         error_type_t type,
                         const char *message) {
    if (cloud_connected()) {
        cloud_send_error(severity, type, message);
    }
}

void buzzer_error_callback(error_severity_t severity,
                          error_type_t type,
                          const char *message) {
    if (severity == ERROR_FATAL) {
        buzzer_beep(3);  /* 3 beeps for fatal error */
    }
}

// Usage:
void main(void) {
    /* Register callbacks */
    error_register_callback(led_error_callback);
    error_register_callback(cloud_error_callback);
    error_register_callback(buzzer_error_callback);
    
    /* Errors will trigger all callbacks */
    error_report(ERROR_FATAL, ERROR_HARDWARE, "Motor failure");
    /* → LED turns on, cloud notified, buzzer beeps */
}
```

**Benefits:**
- Decoupled error handling
- Multiple notification channels
- Easy to add new handlers
- Flexible error response
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Error Codes**
   - Clear status codes
   - Error type classification
   - Self-documenting

2. **Error Context**
   - File, line, function
   - Complete debugging info
   - Easy troubleshooting

3. **Error Recovery**
   - Retry strategies
   - Per-type recovery
   - Automatic handling

4. **Error Logging**
   - Circular buffer
   - Flash persistence
   - Remote diagnostics

5. **Error Callbacks**
   - Decoupled handling
   - Multiple channels
   - Flexible response

---

**Congratulations!** You've mastered Error Handlers — essential for robust embedded systems!
