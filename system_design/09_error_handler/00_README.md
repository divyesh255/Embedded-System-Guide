# Error Handler

**The pattern for robust error management and recovery**

---

## 🎯 What Problem Does This Solve?

Embedded systems encounter errors constantly:
- Hardware failures
- Communication timeouts
- Invalid sensor data
- Memory allocation failures
- Configuration errors

```c
// WRONG: Ignoring errors
int result = read_sensor();
process_data(result);  // What if read failed?
```

**The naive solution — ignoring errors — leads to:**
- Silent failures and data corruption
- Difficult debugging (no error context)
- System crashes in production
- Safety hazards in critical systems

**The solution: Error Handler**

Centralized error management with logging, recovery, and escalation.

---

## 🔧 How It Works

### Error Classification

```c
typedef enum {
    ERROR_NONE = 0,
    ERROR_WARNING,      /* Non-critical, log only */
    ERROR_RECOVERABLE,  /* Try recovery */
    ERROR_FATAL         /* System reset required */
} error_severity_t;

typedef enum {
    ERROR_HARDWARE,     /* Hardware fault */
    ERROR_COMMUNICATION,/* Comm timeout */
    ERROR_DATA,         /* Invalid data */
    ERROR_MEMORY,       /* Allocation failed */
    ERROR_CONFIG        /* Configuration error */
} error_type_t;
```

### Error Reporting

```c
void error_report(error_severity_t severity, 
                  error_type_t type,
                  const char *message) {
    /* Log error */
    log_error(severity, type, message);
    
    /* Take action based on severity */
    switch (severity) {
        case ERROR_WARNING:
            /* Continue operation */
            break;
            
        case ERROR_RECOVERABLE:
            /* Attempt recovery */
            attempt_recovery(type);
            break;
            
        case ERROR_FATAL:
            /* Reset system */
            system_reset();
            break;
    }
}
```

### Error Recovery

```c
bool attempt_recovery(error_type_t type) {
    switch (type) {
        case ERROR_HARDWARE:
            return reinit_hardware();
            
        case ERROR_COMMUNICATION:
            return reconnect();
            
        case ERROR_DATA:
            return use_default_data();
            
        case ERROR_MEMORY:
            return free_unused_memory();
            
        default:
            return false;
    }
}
```

---

## 📐 Error Handling Strategies

### 1. Return Codes
```c
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_TIMEOUT,
    STATUS_ERROR_INVALID,
    STATUS_ERROR_BUSY
} status_t;

status_t read_sensor(int *value) {
    if (!sensor_ready()) {
        return STATUS_ERROR_BUSY;
    }
    
    *value = sensor_read();
    
    if (*value < MIN || *value > MAX) {
        return STATUS_ERROR_INVALID;
    }
    
    return STATUS_OK;
}

// Usage:
int value;
status_t status = read_sensor(&value);
if (status != STATUS_OK) {
    error_report(ERROR_RECOVERABLE, ERROR_DATA, "Sensor read failed");
}
```

### 2. Error Codes with Context
```c
typedef struct {
    status_t code;
    const char *file;
    int line;
    const char *function;
} error_context_t;

#define ERROR_CONTEXT(code) \
    ((error_context_t){ code, __FILE__, __LINE__, __func__ })

// Usage:
error_context_t err = read_sensor_with_context(&value);
if (err.code != STATUS_OK) {
    printf("Error in %s:%d (%s): %d\n", 
           err.file, err.line, err.function, err.code);
}
```

### 3. Error Callbacks
```c
typedef void (*error_callback_t)(error_severity_t, error_type_t, const char*);

static error_callback_t error_callbacks[MAX_CALLBACKS];

void error_register_callback(error_callback_t callback) {
    /* Add to callback list */
}

void error_report(error_severity_t severity, 
                  error_type_t type,
                  const char *message) {
    /* Call all registered callbacks */
    for (int i = 0; i < num_callbacks; i++) {
        error_callbacks[i](severity, type, message);
    }
}
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────┐
│           Application Code                  │
│  Sensors  Actuators  Communication          │
└──────────────┬──────────────────────────────┘
               │ error_report()
┌──────────────▼──────────────────────────────┐
│          Error Handler                      │
│  - Classify error                           │
│  - Log with context                         │
│  - Attempt recovery                         │
│  - Escalate if needed                       │
└──────────────┬──────────────────────────────┘
               │
       ┌───────┴────────┐
       ▼                ▼
┌─────────────┐  ┌─────────────┐
│ Error Log   │  │  Recovery   │
│ (Flash/RAM) │  │  Actions    │
└─────────────┘  └─────────────┘
```

---

## 📊 Comparison

| Approach | Error Detection | Recovery | Debug Info | Complexity |
|----------|----------------|----------|------------|------------|
| Ignore errors | ❌ None | ❌ None | ❌ None | Simple |
| Return codes | ✅ Basic | ⚠️ Manual | ⚠️ Limited | Simple |
| **Error Handler** | ✅ Complete | ✅ Auto | ✅ Full | Medium |
| Exceptions | ✅ Complete | ✅ Auto | ✅ Full | High |

---

## 🔑 Key Takeaways

1. **Classification** — categorize errors by severity and type
2. **Context** — capture file, line, function for debugging
3. **Recovery** — attempt automatic recovery when possible
4. **Escalation** — escalate to higher severity if recovery fails
5. **Logging** — maintain error history for analysis

---

## 🎯 Use Cases

**Medical Devices:**
- Sensor failures
- Communication errors
- Safety violations
- Calibration errors

**Automotive:**
- CAN bus errors
- Sensor faults
- Actuator failures
- Diagnostic trouble codes (DTCs)

**Industrial:**
- Motor faults
- Network timeouts
- Configuration errors
- Safety system failures

**IoT:**
- Cloud connectivity
- Battery low
- Sensor calibration
- Firmware updates

---

**Ready to see the problem?** → `01_problem.md`
