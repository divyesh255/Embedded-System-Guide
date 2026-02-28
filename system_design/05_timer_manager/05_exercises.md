# Timer Manager - Practice Exercises

Test your understanding of software timer managers with these hands-on exercises!

---

## 🎯 Exercise 1: Calculate Timer Pool Size (Easy - 10 min)

A motor controller needs the following timed tasks:

| Task | Period | Type |
|------|--------|------|
| PWM update | 1ms | Periodic |
| Speed PID | 10ms | Periodic |
| Temperature check | 500ms | Periodic |
| Fault LED blink | 250ms | Periodic |
| Watchdog kick | 50ms | Periodic |
| Overcurrent timeout | 100ms | One-shot |
| Startup delay | 2000ms | One-shot |

**Task:** How many timer slots do you need? What tick resolution?

<details>
<summary>Solution</summary>

**Timer count:** 7 tasks → need at least **8 slots** (next power of 2, plus 1 spare)

**Tick resolution:**
```
Shortest period = 1ms (PWM update)
Tick must be ≤ shortest period
→ Use 1ms tick (SysTick at 1kHz)
```

**Why 1ms tick?**
- PWM update fires every 1ms → tick must be ≤ 1ms
- Finer tick (e.g., 100µs) wastes CPU on ISR overhead
- Coarser tick (e.g., 10ms) can't fire PWM every 1ms

**Pool definition:**
```c
#define MAX_TIMERS   8   // 7 tasks + 1 spare
#define TICK_RATE_HZ 1000  // 1ms tick
```

**Important:** The PWM update callback must be extremely fast (< 1ms) since it fires every tick. Consider whether it should be in the tick ISR directly rather than via flag+main loop.
</details>

---

## 🎯 Exercise 2: Fix the Volatile Bug (Easy - 15 min)

This timer callback code has a bug. Find and fix it.

```c
bool sensor_ready = false;
uint32_t sample_count = 0;

void on_sensor_timer(void) {
    sensor_ready = true;
    sample_count++;
}

void main_loop(void) {
    while (!sensor_ready) { /* wait */ }
    process_sample(sample_count);
    sensor_ready = false;
}
```

<details>
<summary>Solution</summary>

**Bug:** Both shared variables are missing `volatile`.

**Problem 1: `sensor_ready`**
```c
// Compiler sees sensor_ready never changes in main_loop
// Optimizes while (!sensor_ready) → while (true) → infinite loop!
```

**Problem 2: `sample_count`**
```c
// Compiler may cache sample_count in a register
// main_loop reads stale value from register, not memory
```

**Fixed:**
```c
volatile bool     sensor_ready  = false;  // MUST be volatile
volatile uint32_t sample_count  = 0;      // MUST be volatile

void on_sensor_timer(void) {
    sensor_ready = true;
    sample_count++;
}

void main_loop(void) {
    while (!sensor_ready) { /* wait — now checks memory */ }
    process_sample(sample_count);  // Reads from memory
    sensor_ready = false;
}
```

**Rule:** Every variable written by a timer callback (ISR context) and read by main loop MUST be `volatile`.
</details>

---

## 🎯 Exercise 3: Implement a Timeout Pattern (Medium - 20 min)

Implement a communication timeout: if no response arrives within 500ms of sending a request, retry (up to 3 times).

**Task:** Use a one-shot timer to implement the timeout.

```c
// Given:
int  timeout_timer_id;
void send_request(void);
void process_response(void);
bool response_received(void);
```

<details>
<summary>Solution</summary>

```c
#define MAX_RETRIES     3
#define TIMEOUT_MS    500

static volatile bool timeout_fired = false;
static uint32_t      retry_count   = 0;

/* Callback — ISR context */
void on_timeout(void) {
    timeout_fired = true;  /* Just set flag */
}

/* Initialize */
void comm_init(void) {
    timeout_timer_id = timer_create(TIMEOUT_MS, TIMER_ONE_SHOT,
                                    on_timeout, "Comm-timeout");
    retry_count = 0;
}

/* Send and start timeout */
void comm_send(void) {
    send_request();
    timer_start(timeout_timer_id);  /* Start 500ms countdown */
    timeout_fired = false;
    printf("Request sent, timeout started\n");
}

/* Main loop */
void comm_loop(void) {
    if (response_received()) {
        timer_stop(timeout_timer_id);  /* Cancel timeout */
        timeout_fired = false;
        retry_count   = 0;
        process_response();
        printf("Response received OK\n");
        return;
    }

    if (timeout_fired) {
        timeout_fired = false;
        retry_count++;

        if (retry_count <= MAX_RETRIES) {
            printf("Timeout! Retry %u/%u\n", retry_count, MAX_RETRIES);
            comm_send();  /* Retry — restarts timer */
        } else {
            printf("Max retries reached — giving up\n");
            retry_count = 0;
            /* handle_comm_failure(); */
        }
    }
}
```

**Key points:**
- `TIMER_ONE_SHOT` — fires once, stops automatically
- `timer_stop()` on success — cancels pending timeout
- `timer_start()` on retry — restarts the countdown
- Flag cleared before checking response (avoids race)
</details>

---

## 🎯 Exercise 4: Design a Debounce System (Medium - 25 min)

Design a complete button debounce system for 3 buttons. Each button:
- Has its own 50ms debounce timer
- Detects press AND release
- Counts consecutive presses

**Task:** Design the data structure and callback logic.

<details>
<summary>Solution</summary>

```c
#define NUM_BUTTONS    3
#define DEBOUNCE_MS   50

typedef struct {
    int      timer_id;
    bool     last_state;      /* Last confirmed state */
    bool     raw_state;       /* Raw GPIO reading */
    uint32_t press_count;     /* Consecutive presses */
    volatile bool pending;    /* Debounce timer fired */
} button_t;

static button_t buttons[NUM_BUTTONS];

/* One callback per button — sets the right pending flag */
static void on_btn0_debounce(void) { buttons[0].pending = true; }
static void on_btn1_debounce(void) { buttons[1].pending = true; }
static void on_btn2_debounce(void) { buttons[2].pending = true; }

static timer_callback_t btn_callbacks[NUM_BUTTONS] = {
    on_btn0_debounce,
    on_btn1_debounce,
    on_btn2_debounce,
};

/* Initialize all button timers */
void buttons_init(void) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].timer_id = timer_create(DEBOUNCE_MS, TIMER_ONE_SHOT,
                                           btn_callbacks[i], "Btn-debounce");
        buttons[i].last_state  = false;
        buttons[i].press_count = 0;
        buttons[i].pending     = false;
    }
}

/* Called from GPIO ISR when any button changes */
void on_gpio_change(int btn_idx, bool new_raw_state) {
    if (btn_idx >= NUM_BUTTONS) return;
    buttons[btn_idx].raw_state = new_raw_state;
    timer_reset(buttons[btn_idx].timer_id);  /* Retrigger debounce */
}

/* Main loop — process confirmed button states */
void buttons_process(void) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (!buttons[i].pending) continue;
        buttons[i].pending = false;

        bool confirmed = buttons[i].raw_state;

        if (confirmed && !buttons[i].last_state) {
            /* Press confirmed */
            buttons[i].press_count++;
            printf("Button %d PRESSED (count: %u)\n",
                   i, buttons[i].press_count);
        } else if (!confirmed && buttons[i].last_state) {
            /* Release confirmed */
            printf("Button %d RELEASED\n", i);
        }

        buttons[i].last_state = confirmed;
    }
}
```

**Key design decisions:**
- One timer per button — independent debounce periods
- `timer_reset()` on each GPIO change — retriggerable
- Detects both press and release
- `raw_state` captured at GPIO change, confirmed after debounce
</details>

---

## 🎯 Exercise 5: Identify the Timer Bug (Hard - 30 min)

This code has a subtle bug. What happens and why?

```c
volatile bool data_ready = false;
uint32_t      process_timer_id;

void on_process_timer(void) {
    data_ready = true;
}

void main_loop(void) {
    while (1) {
        if (data_ready) {
            data_ready = false;

            // Processing takes 150ms
            process_data();   // 100ms
            send_to_cloud();  // 50ms

            // Restart timer for next cycle
            timer_start(process_timer_id);  // Period: 200ms
        }
    }
}

// Setup:
// process_timer_id = timer_create(200, TIMER_ONE_SHOT, on_process_timer, "proc");
// timer_start(process_timer_id);
```

<details>
<summary>Solution</summary>

**The Bug:** Using `TIMER_ONE_SHOT` + manual `timer_start()` after processing creates **timing drift**.

**What happens:**
```
t=0ms:    timer_start() — starts 200ms countdown
t=200ms:  timer fires → data_ready = true
t=200ms:  main loop detects data_ready
t=350ms:  process_data() + send_to_cloud() complete (150ms)
t=350ms:  timer_start() — starts ANOTHER 200ms countdown
t=550ms:  timer fires again

Actual period: 200ms + 150ms processing = 350ms
Expected:      200ms
Drift:         75% longer than expected!
```

**Fix 1: Use PERIODIC timer (if processing < period)**
```c
// Timer fires every 200ms regardless of processing time
process_timer_id = timer_create(200, TIMER_PERIODIC,
                                on_process_timer, "proc");
timer_start(process_timer_id);

// Remove timer_start() from main loop — not needed!
void main_loop(void) {
    while (1) {
        if (data_ready) {
            data_ready = false;
            process_data();    // 100ms
            send_to_cloud();   // 50ms
            // Timer already running — no restart needed
        }
    }
}
```

**Fix 2: If processing can exceed period, use flag to skip**
```c
volatile bool data_ready    = false;
volatile bool processing    = false;

void on_process_timer(void) {
    if (!processing) {
        data_ready = true;  // Only set if not already processing
    }
    // If processing: skip this cycle (don't overrun)
}

void main_loop(void) {
    while (1) {
        if (data_ready) {
            data_ready  = false;
            processing  = true;
            process_data();
            send_to_cloud();
            processing  = false;
        }
    }
}
```

**Rule:** Use `TIMER_PERIODIC` for regular tasks. Only use `TIMER_ONE_SHOT` + manual restart for tasks where the period should start AFTER processing completes.
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Timer Pool Sizing**
   - Count all tasks needing timers
   - Tick resolution = shortest period
   - Add 1-2 spare slots

2. **Volatile**
   - All variables shared between callback (ISR) and main loop MUST be `volatile`
   - Prevents compiler optimization bugs

3. **One-Shot vs Periodic**
   - `PERIODIC`: regular tasks (LED, sensor, watchdog)
   - `ONE_SHOT`: debounce, timeout, retry delay
   - `ONE_SHOT` + manual restart = drift (usually wrong)

4. **Deferred Processing**
   - Callbacks set flags only (fast, ISR-safe)
   - Main loop does the actual work (slow OK)
   - Process flags in priority order (watchdog first)

5. **Retriggerable Timers**
   - `timer_reset()` restarts countdown from now
   - Used for debounce, inactivity timeout, watchdog

---

**Congratulations!** You've mastered Software Timer Managers — the foundation of cooperative multitasking in bare-metal embedded systems!
