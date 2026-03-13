/**
 * 03_error_handler_good.c - GOOD: Error Handler
 *
 * Solves smart home hub problem with error handler:
 *   - Detects all errors
 *   - Logs with context
 *   - Attempts recovery
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Error types */
typedef enum {
    ERROR_NONE = 0,
    ERROR_WARNING,
    ERROR_RECOVERABLE,
    ERROR_FATAL
} error_severity_t;

typedef enum {
    ERROR_SENSOR,
    ERROR_COMMUNICATION,
    ERROR_HARDWARE
} error_type_t;

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR_TIMEOUT,
    STATUS_ERROR_INVALID,
    STATUS_ERROR_DISCONNECTED
} status_t;

static uint32_t sys_ms = 0;
static int last_valid_temp = 22;
static bool simulate_sensor_fail = false;
static bool simulate_cloud_fail = false;

/* Error handler */
void error_report(error_severity_t severity, error_type_t type, const char *msg) {
    const char *sev_str[] = {"NONE", "WARNING", "RECOVERABLE", "FATAL"};
    const char *type_str[] = {"SENSOR", "COMM", "HARDWARE"};
    
    printf("[%ums] ERROR [%s/%s]: %s\n", 
           sys_ms, sev_str[severity], type_str[type], msg);
    
    switch (severity) {
        case ERROR_WARNING:
            /* Log only, continue */
            break;
            
        case ERROR_RECOVERABLE:
            printf("[%ums] Attempting recovery...\n", sys_ms);
            break;
            
        case ERROR_FATAL:
            printf("[%ums] FATAL ERROR - System reset required\n", sys_ms);
            break;
            
        default:
            break;
    }
}

/* Sensor with error handling */
status_t read_temperature(int *temp) {
    if (simulate_sensor_fail) {
        return STATUS_ERROR_TIMEOUT;
    }
    *temp = 22;
    return STATUS_OK;
}

/* Cloud with error handling */
status_t update_cloud(int temp) {
    if (simulate_cloud_fail) {
        return STATUS_ERROR_DISCONNECTED;
    }
    printf("[%ums] Cloud updated: temp=%d\n", sys_ms, temp);
    return STATUS_OK;
}

void set_light(bool on) {
    printf("[%ums] Light: %s\n", sys_ms, on ? "ON" : "OFF");
}

int main(void) {
    printf("=== GOOD: Error Handler ===\n\n");
    
    printf("--- Normal operation ---\n");
    for (int i = 0; i < 3; i++) {
        int temp;
        status_t status = read_temperature(&temp);
        if (status == STATUS_OK) {
            printf("[%ums] Temperature: %d°C\n", sys_ms, temp);
            last_valid_temp = temp;
            set_light(temp > 25);
            update_cloud(temp);
        }
        sys_ms += 1000;
    }

    printf("\n--- Simulating sensor failure ---\n");
    simulate_sensor_fail = true;
    for (int i = 0; i < 3; i++) {
        int temp;
        status_t status = read_temperature(&temp);
        if (status != STATUS_OK) {
            error_report(ERROR_RECOVERABLE, ERROR_SENSOR, 
                        "Temperature sensor timeout");
            /* Recovery: Use last valid value */
            temp = last_valid_temp;
            printf("[%ums] Using cached temperature: %d°C\n", sys_ms, temp);
        }
        set_light(temp > 25);
        update_cloud(temp);
        sys_ms += 1000;
    }

    printf("\n--- Simulating cloud failure ---\n");
    simulate_sensor_fail = false;
    simulate_cloud_fail = true;
    for (int i = 0; i < 3; i++) {
        int temp;
        status_t status = read_temperature(&temp);
        if (status == STATUS_OK) {
            printf("[%ums] Temperature: %d°C\n", sys_ms, temp);
            set_light(temp > 25);
            
            status = update_cloud(temp);
            if (status != STATUS_OK) {
                error_report(ERROR_WARNING, ERROR_COMMUNICATION,
                            "Cloud connection failed");
                printf("[%ums] Continuing with local operation\n", sys_ms);
            }
        }
        sys_ms += 1000;
    }

    printf("\n=== Results ===\n");
    printf("✅ All errors detected and reported\n");
    printf("✅ Sensor failure: Used cached value\n");
    printf("✅ Cloud failure: Continued locally\n");
    printf("✅ System remained operational\n");

    printf("\n=== Improvements Over No Error Handling ===\n");
    printf("✅ Detects all errors\n");
    printf("✅ Logs with context (time, type, severity)\n");
    printf("✅ Attempts automatic recovery\n");
    printf("✅ System continues operation\n");

    return 0;
}

/*
 * HOW ERROR HANDLER WORKS:
 *
 * 1. Error Detection:
 *    - Functions return status codes
 *    - Caller checks status
 *    - Reports errors to handler
 *
 * 2. Error Classification:
 *    - Severity: WARNING, RECOVERABLE, FATAL
 *    - Type: SENSOR, COMM, HARDWARE
 *
 * 3. Error Recovery:
 *    - WARNING: Log only
 *    - RECOVERABLE: Attempt recovery
 *    - FATAL: Reset system
 *
 * 4. Error Logging:
 *    - Timestamp
 *    - Severity
 *    - Type
 *    - Message
 */
