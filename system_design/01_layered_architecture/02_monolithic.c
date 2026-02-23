/**
 * 02_monolithic.c - BAD EXAMPLE: Monolithic Code (No Layers)
 * 
 * This is the WRONG way to structure embedded code.
 * Everything is mixed together in one giant function.
 * 
 * Problems:
 * - Can't test without hardware
 * - Can't port to different MCU
 * - Can't reuse components
 * - Hard to maintain
 * - Team can't work in parallel
 * 
 * Study time: 15 minutes
 * DO NOT use this pattern in production!
 */

#include <stdint.h>
#include <stdbool.h>

/* Simulated hardware registers (in real code, these would be actual registers) */
volatile uint8_t *PORTA = (uint8_t*)0x1000;
volatile uint8_t *PORTB = (uint8_t*)0x1001;
volatile uint16_t *ADC_REG = (uint16_t*)0x1002;
volatile uint8_t *UART_DATA = (uint8_t*)0x1003;
volatile uint8_t *UART_STATUS = (uint8_t*)0x1004;
volatile uint8_t *EEPROM_ADDR = (uint8_t*)0x1005;
volatile uint8_t *EEPROM_DATA = (uint8_t*)0x1006;

#define TEMP_THRESHOLD 38.5
#define ALARM_PIN 5
#define LCD_RS_PIN 2
#define LCD_EN_PIN 3

/**
 * THE MONOLITHIC MONSTER
 * 
 * This function does EVERYTHING:
 * - Reads ADC
 * - Converts to temperature
 * - Controls LCD
 * - Manages alarm
 * - Sends UART
 * - Writes EEPROM
 * - Handles timing
 * 
 * Result: Unmaintainable mess!
 */
void temperature_monitor_monolithic(void) {
    uint16_t log_counter = 0;
    
    /* Initialize hardware - mixed with application logic */
    *PORTA |= (1 << ALARM_PIN);  /* What pin is this? */
    *PORTB |= (1 << LCD_RS_PIN) | (1 << LCD_EN_PIN);
    
    while(1) {
        /* ========== ADC Reading ========== */
        /* Problem: Hardware-specific code mixed with logic */
        *ADC_REG = 0x01;  /* Start conversion - magic number! */
        
        /* Busy wait - blocking! */
        while(*ADC_REG & 0x80);  /* What does 0x80 mean? */
        
        uint16_t adc_value = *ADC_REG & 0x3FF;
        
        /* ========== Temperature Conversion ========== */
        /* Problem: Algorithm buried in main loop */
        float voltage = adc_value * 5.0 / 1024.0;
        float temperature = (voltage - 0.5) * 100.0;
        
        /* ========== LCD Display ========== */
        /* Problem: LCD protocol mixed with application */
        *PORTB &= ~(1 << LCD_RS_PIN);  /* Command mode */
        *PORTB |= (1 << LCD_EN_PIN);
        *PORTB = 0x01;  /* Clear display - magic number! */
        
        /* Delay - but how long? Why? */
        for(volatile int i = 0; i < 1000; i++);
        
        *PORTB &= ~(1 << LCD_EN_PIN);
        
        /* More LCD bit-banging... */
        *PORTB |= (1 << LCD_RS_PIN);  /* Data mode */
        
        /* Convert temperature to string - inline! */
        int temp_int = (int)temperature;
        int temp_frac = (int)((temperature - temp_int) * 10);
        
        /* Display digits - hardcoded LCD commands */
        *PORTB = '0' + (temp_int / 10);
        for(volatile int i = 0; i < 100; i++);
        *PORTB = '0' + (temp_int % 10);
        for(volatile int i = 0; i < 100; i++);
        *PORTB = '.';
        for(volatile int i = 0; i < 100; i++);
        *PORTB = '0' + temp_frac;
        
        /* ========== Alarm Logic ========== */
        /* Problem: Business logic mixed with hardware control */
        if(temperature > TEMP_THRESHOLD) {
            /* Turn on alarm - but which pin? */
            *PORTA |= (1 << ALARM_PIN);
            
            /* ========== UART Alert ========== */
            /* Problem: UART protocol inline */
            while(!(*UART_STATUS & 0x20));  /* Wait for ready - magic number! */
            *UART_DATA = 'A';  /* 'A' for alarm */
            
            while(!(*UART_STATUS & 0x20));
            *UART_DATA = (uint8_t)temperature;
            
            while(!(*UART_STATUS & 0x20));
            *UART_DATA = '\n';
        } else {
            *PORTA &= ~(1 << ALARM_PIN);
        }
        
        /* ========== EEPROM Logging ========== */
        /* Problem: Storage logic mixed with everything else */
        log_counter++;
        if(log_counter >= 300) {  /* 300 seconds = 5 minutes */
            /* EEPROM write - inline */
            uint8_t log_address = (log_counter / 300) % 256;
            
            *EEPROM_ADDR = log_address;
            *EEPROM_DATA = (uint8_t)temperature;
            
            /* Trigger write - magic sequence */
            *EEPROM_ADDR |= 0x04;  /* What does this do? */
            *EEPROM_ADDR |= 0x02;
            
            /* Wait for write - how long? */
            for(volatile int i = 0; i < 10000; i++);
            
            log_counter = 0;
        }
        
        /* ========== Timing ========== */
        /* Problem: Crude delay, blocks everything */
        for(volatile int i = 0; i < 100000; i++);  /* ~1 second? */
    }
}

/**
 * PROBLEMS WITH THIS CODE:
 * 
 * 1. PORTABILITY: Zero
 *    - Change MCU? Rewrite everything
 *    - All hardware registers hardcoded
 * 
 * 2. TESTABILITY: Impossible
 *    - Can't test without actual hardware
 *    - Can't mock components
 *    - Can't unit test algorithms
 * 
 * 3. MAINTAINABILITY: Nightmare
 *    - 100+ lines in one function
 *    - Magic numbers everywhere
 *    - No clear structure
 *    - Fix one thing, break another
 * 
 * 4. REUSABILITY: None
 *    - LCD code can't be reused
 *    - UART code can't be reused
 *    - Everything is tangled
 * 
 * 5. TEAM WORK: Impossible
 *    - Only one person can work on this
 *    - Merge conflicts guaranteed
 *    - No clear ownership
 * 
 * 6. DEBUGGING: Painful
 *    - Where is the bug?
 *    - In ADC? LCD? UART? EEPROM?
 *    - Everything affects everything
 * 
 * 7. REQUIREMENTS CHANGES: Disaster
 *    - Add WiFi? Rewrite everything
 *    - Change sensor? Rewrite everything
 *    - Add feature? Risk breaking everything
 * 
 * 8. CODE REVIEW: Impossible
 *    - Reviewer must understand ALL hardware
 *    - Can't review in isolation
 *    - High risk of missing bugs
 * 
 * 9. CERTIFICATION: Failed
 *    - Can't isolate safety-critical code
 *    - Can't prove correctness
 *    - Auditor can't verify
 * 
 * 10. PERFORMANCE: Poor
 *     - Busy waiting wastes CPU
 *     - No optimization possible
 *     - Everything blocks everything
 */

/**
 * REAL-WORLD CONSEQUENCES:
 * 
 * Week 2:  Bug in LCD breaks temperature reading
 * Week 4:  Can't add new sensor without rewriting
 * Week 6:  QA can't test, delays release
 * Week 8:  Team conflicts, productivity drops
 * Week 10: Port to ARM requires complete rewrite
 * Week 12: FDA audit fails, 3-month delay
 * 
 * TOTAL COST: $340,000 in delays and rework
 * 
 * This is why we need LAYERED ARCHITECTURE!
 */

int main(void) {
    temperature_monitor_monolithic();
    return 0;
}

/*
 * KEY LESSONS:
 * 
 * 1. Don't mix hardware and application logic
 * 2. Don't put everything in one function
 * 3. Don't use magic numbers
 * 4. Don't make code untestable
 * 5. Don't ignore maintainability
 * 
 * NEXT: See how layered architecture fixes ALL these problems!
 * Continue to: 03_layered.c
 */
