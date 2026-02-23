# Problem: Temperature Monitoring System

## 📋 The Scenario

You're hired by a medical device company to develop a **patient temperature monitoring system**. The device must:

### Requirements
1. Read temperature from an analog sensor every second
2. Display temperature on an LCD screen
3. Sound an alarm if temperature > 38.5°C (fever)
4. Log temperature data to EEPROM every 5 minutes
5. Send alerts via UART to a central monitoring station
6. Must work 24/7 for years without failure

### Business Constraints
- **Time to Market:** 6 months to production
- **Cost:** Must use low-cost MCU (ATmega328 or similar)
- **Future:** May need to port to ARM Cortex-M later
- **Compliance:** Must pass FDA medical device certification
- **Team:** 3 developers working simultaneously

## 🤔 Your First Attempt

You start coding and create this:

```c
// temperature_monitor.c - Your first attempt

#include <avr/io.h>
#include <util/delay.h>

#define TEMP_THRESHOLD 38.5
#define ALARM_PIN 5
#define LCD_RS 2
#define LCD_EN 3

void main() {
    // Initialize everything
    DDRB |= (1 << ALARM_PIN);  // Alarm output
    DDRD |= (1 << LCD_RS) | (1 << LCD_EN);  // LCD pins
    
    // ADC setup
    ADMUX = (1 << REFS0);  // AVcc reference
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  // Enable ADC
    
    // UART setup
    UBRR0H = 0;
    UBRR0L = 103;  // 9600 baud at 16MHz
    UCSR0B = (1 << TXEN0);
    
    uint16_t log_counter = 0;
    
    while(1) {
        // Read temperature
        ADMUX = (ADMUX & 0xF0) | 0;  // Select ADC0
        ADCSRA |= (1 << ADSC);  // Start conversion
        while(ADCSRA & (1 << ADSC));  // Wait
        uint16_t adc = ADC;
        
        // Convert to temperature
        float voltage = adc * 5.0 / 1024.0;
        float temp = (voltage - 0.5) * 100.0;
        
        // Display on LCD
        PORTD &= ~(1 << LCD_RS);  // Command mode
        PORTD |= (1 << LCD_EN);
        PORTD = 0x01;  // Clear display
        _delay_us(100);
        PORTD &= ~(1 << LCD_EN);
        _delay_ms(2);
        
        // Show temperature (simplified)
        PORTD |= (1 << LCD_RS);  // Data mode
        // ... 50 more lines of LCD bit-banging ...
        
        // Check alarm
        if(temp > TEMP_THRESHOLD) {
            PORTB |= (1 << ALARM_PIN);  // Alarm ON
            
            // Send UART alert
            while(!(UCSR0A & (1 << UDRE0)));
            UDR0 = 'A';  // 'A' for alarm
            while(!(UCSR0A & (1 << UDRE0)));
            UDR0 = (uint8_t)temp;
        } else {
            PORTB &= ~(1 << ALARM_PIN);  // Alarm OFF
        }
        
        // Log to EEPROM every 5 minutes
        log_counter++;
        if(log_counter >= 300) {  // 300 seconds = 5 minutes
            // EEPROM write
            while(EECR & (1 << EEPE));
            EEAR = log_counter / 300;
            EEDR = (uint8_t)temp;
            EECR |= (1 << EEMPE);
            EECR |= (1 << EEPE);
            
            log_counter = 0;
        }
        
        _delay_ms(1000);  // Wait 1 second
    }
}
```

## 😱 The Problems Start

### Week 2: Bug Reports
**Issue #1:** "LCD sometimes shows garbage"
- You spend 3 days debugging
- Problem: Timing issues in LCD bit-banging
- Fix requires changing 20+ lines scattered throughout code

**Issue #2:** "Temperature readings are noisy"
- Need to add averaging
- But where? Code is already 300 lines in one function

### Week 4: Change Request
**Manager:** "We need to support a different temperature sensor (I2C instead of analog)"
- You realize: ADC code is mixed with display code
- Changing sensor requires rewriting 40% of the code
- Risk breaking LCD and UART code

### Week 6: Testing Nightmare
**QA Team:** "How do we test this without hardware?"
- You can't - everything touches hardware registers
- Can't write unit tests
- Must test on actual device every time
- Each test cycle takes 30 minutes

### Week 8: Team Conflict
**Developer 2:** "I need to add WiFi communication"
- But your code is one giant function
- Can't work in parallel
- Merge conflicts everywhere
- Team productivity drops 50%

### Week 10: Port to ARM
**Manager:** "We're switching to STM32 for next version"
- You look at the code: `PORTA`, `ADMUX`, `UCSR0B` everywhere
- Realize: Need to rewrite EVERYTHING
- 6 months of work down the drain

### Week 12: FDA Audit
**Auditor:** "Show me your temperature reading algorithm"
- You: "Uh... it's mixed with LCD code and UART code..."
- Auditor: "I need to verify ONLY the temperature calculation"
- You: "That's... not possible with this code structure"
- **Result:** Failed audit, 3-month delay

## 💭 Think About It (5 minutes)

Before looking at the solution, ask yourself:

1. **What went wrong?**
   - Why is this code so hard to maintain?
   - Why can't we test it?
   - Why can't we port it?

2. **How would YOU fix it?**
   - How would you separate concerns?
   - How would you make it testable?
   - How would you make it portable?

3. **What if...**
   - You need to support 5 different sensors?
   - You need to add Bluetooth?
   - You need to run on 3 different MCUs?
   - You have 10 developers on the team?

## 🎯 The Core Problems

Let's identify what's wrong:

### Problem 1: Mixed Responsibilities
```c
// One function does EVERYTHING:
- Read ADC
- Convert to temperature
- Control LCD
- Check alarm
- Send UART
- Write EEPROM
- Timing control
```

### Problem 2: Hardware Coupling
```c
// Application logic mixed with hardware:
if(temp > THRESHOLD) {
    PORTB |= (1 << 5);  // What is this?
}
```

### Problem 3: No Abstraction
```c
// Direct register access everywhere:
ADMUX = (ADMUX & 0xF0) | 0;
ADCSRA |= (1 << ADSC);
```

### Problem 4: Untestable
```c
// Can't test without hardware:
void main() {
    // Everything in main()
    // No way to mock hardware
}
```

### Problem 5: Not Reusable
```c
// LCD code can't be reused:
// It's tangled with temperature logic
```

## 📊 Impact Analysis

| Issue | Time Lost | Cost Impact |
|-------|-----------|-------------|
| Bug fixing | 2 weeks | $20,000 |
| Change requests | 3 weeks | $30,000 |
| Testing delays | 4 weeks | $40,000 |
| Port to ARM | 6 months | $150,000 |
| Failed FDA audit | 3 months | $100,000 |
| **TOTAL** | **~10 months** | **$340,000** |

## 🤯 The Realization

You realize you need:
- **Separation of Concerns** - Each piece does ONE thing
- **Hardware Abstraction** - Hide hardware details
- **Testability** - Test without hardware
- **Portability** - Easy to change MCU
- **Maintainability** - Easy to modify
- **Team Scalability** - Multiple developers can work together

## 💡 The Solution Preview

What if the code looked like this instead?

```c
// Application Layer - Clean and readable
void temperature_monitor_task() {
    float temp = temp_sensor_read();
    
    lcd_display_temperature(temp);
    
    if(temp > TEMP_THRESHOLD) {
        alarm_activate();
        uart_send_alert(temp);
    } else {
        alarm_deactivate();
    }
    
    if(should_log_data()) {
        eeprom_log_temperature(temp);
    }
}
```

**Benefits:**
- ✅ Easy to understand
- ✅ Easy to test (mock each function)
- ✅ Easy to port (change implementations, not logic)
- ✅ Easy to maintain (clear structure)
- ✅ Easy to extend (add new features)

## 🚀 Next Steps

Now that you understand the problem, let's see the solution:

1. **02_monolithic.c** - The bad code (what we just saw)
2. **03_layered.c** - Refactored with layers
3. **04_production.c** - Industrial-grade implementation

---

**Key Takeaway:** Without proper architecture, even simple projects become unmaintainable nightmares. Layered architecture solves this by separating concerns and abstracting hardware.

**Ready to see the solution?** Continue to the next file!
