# Signal Handling

**Asynchronous event notification in Unix/Linux**

---

## 🎯 What are Signals?

**Signals** are software interrupts that provide a way to handle asynchronous events. They notify a process that a specific event has occurred.

```c
// Process receives signal
SIGINT (Ctrl+C) → Process → Signal Handler → Action

// Common signals
SIGINT  - Interrupt (Ctrl+C)
SIGTERM - Termination request
SIGSEGV - Segmentation fault
SIGALRM - Timer expired
SIGUSR1 - User-defined signal 1
```

Think of signals as **asynchronous notifications** from the OS or other processes.

---

## 🤔 Why Use Signals?

**Problem:** Need to handle asynchronous events (user interrupts, timers, errors).

**Without signals:**
```c
// Polling - inefficient!
while (running) {
    if (check_interrupt()) break;
    if (check_timer()) handle_timeout();
    do_work();
}
```

**With signals:**
```c
// Efficient - OS notifies you!
signal(SIGINT, handle_interrupt);
signal(SIGALRM, handle_timeout);

while (running) {
    do_work();  // Interrupted automatically when signal arrives
}
```

---

## 📊 Signals vs Other IPC

| Feature | Signals | Pipes | Sockets | Shared Memory |
|---------|---------|-------|---------|---------------|
| **Data** | No data | Stream | Stream | Large data |
| **Async** | Yes | No | No | No |
| **Overhead** | Very low | Low | Medium | Low |
| **Use Case** | Notifications | IPC | Network | High-speed IPC |

---

## 🔧 Signal API

### Basic Signal Handling (signal)

```c
#include <signal.h>

void handler(int signum) {
    // Handle signal
    printf("Caught signal %d\n", signum);
}

int main(void) {
    // Register handler
    signal(SIGINT, handler);
    
    // Or ignore signal
    signal(SIGTERM, SIG_IGN);
    
    // Or use default action
    signal(SIGUSR1, SIG_DFL);
}
```

### Advanced Signal Handling (sigaction)

```c
#include <signal.h>

void handler(int signum, siginfo_t *info, void *context) {
    // More information available
    printf("Signal %d from PID %d\n", signum, info->si_pid);
}

int main(void) {
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;  // Use sa_sigaction
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGINT, &sa, NULL);
}
```

### Sending Signals

```c
#include <signal.h>
#include <unistd.h>

// Send to specific process
kill(pid, SIGTERM);

// Send to self
raise(SIGUSR1);

// Send to process group
kill(-pgid, SIGINT);
```

### Blocking Signals

```c
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);

// Block SIGINT
sigprocmask(SIG_BLOCK, &set, NULL);

// Critical section - SIGINT won't interrupt

// Unblock SIGINT
sigprocmask(SIG_UNBLOCK, &set, NULL);
```

---

## 💡 Common Signals

### Standard Signals

| Signal | Number | Default | Description |
|--------|--------|---------|-------------|
| SIGHUP | 1 | Term | Hangup (terminal closed) |
| SIGINT | 2 | Term | Interrupt (Ctrl+C) |
| SIGQUIT | 3 | Core | Quit (Ctrl+\\) |
| SIGILL | 4 | Core | Illegal instruction |
| SIGABRT | 6 | Core | Abort signal |
| SIGFPE | 8 | Core | Floating point exception |
| SIGKILL | 9 | Term | Kill (cannot be caught!) |
| SIGSEGV | 11 | Core | Segmentation fault |
| SIGPIPE | 13 | Term | Broken pipe |
| SIGALRM | 14 | Term | Alarm clock |
| SIGTERM | 15 | Term | Termination request |
| SIGUSR1 | 10 | Term | User-defined 1 |
| SIGUSR2 | 12 | Term | User-defined 2 |
| SIGCHLD | 17 | Ign | Child stopped/terminated |

### Real-Time Signals

```c
// SIGRTMIN to SIGRTMAX
// Queued, ordered delivery
// Can carry data (sigqueue)

union sigval value;
value.sival_int = 42;
sigqueue(pid, SIGRTMIN, value);
```

---

## ⚠️ Signal Safety

### Async-Signal-Safe Functions

**Only these functions are safe to call in signal handlers:**

```c
// Safe functions (partial list)
write()
_exit()
signal()
sigaction()
// ... see man 7 signal-safety for full list

// UNSAFE functions (DO NOT USE in handlers!)
printf()      // Not async-signal-safe!
malloc()      // Not async-signal-safe!
pthread_*()   // Not async-signal-safe!
```

### Safe Signal Handler Pattern

```c
volatile sig_atomic_t flag = 0;

void handler(int sig) {
    // Only set flag - minimal work!
    flag = 1;
}

int main(void) {
    signal(SIGINT, handler);
    
    while (1) {
        if (flag) {
            // Handle in main loop - safe!
            printf("Signal received\n");
            flag = 0;
        }
        // Do work
    }
}
```

---

## 🎨 Common Patterns

### 1. Graceful Shutdown

```c
volatile sig_atomic_t running = 1;

void shutdown_handler(int sig) {
    running = 0;  // Request shutdown
}

int main(void) {
    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    
    while (running) {
        do_work();
    }
    
    cleanup();
    return 0;
}
```

### 2. Timer with SIGALRM

```c
void timer_handler(int sig) {
    // Timer expired
    alarm(5);  // Reset for 5 more seconds
}

int main(void) {
    signal(SIGALRM, timer_handler);
    alarm(5);  // Start 5-second timer
    
    while (1) {
        do_work();
    }
}
```

### 3. Child Process Reaping

```c
void sigchld_handler(int sig) {
    // Reap zombie children
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(void) {
    signal(SIGCHLD, sigchld_handler);
    
    // Fork children...
}
```

### 4. Signal with eventfd (Safe!)

```c
int efd;

void signal_handler(int sig) {
    uint64_t val = 1;
    write(efd, &val, sizeof(val));  // Async-signal-safe!
}

int main(void) {
    efd = eventfd(0, EFD_NONBLOCK);
    signal(SIGINT, signal_handler);
    
    // Use poll/epoll to wait for efd
    // Process signal in main loop - safe!
}
```

---

## ⚠️ Common Pitfalls

### 1. Race Conditions

```c
int flag = 0;  // ❌ Not atomic!

void handler(int sig) {
    flag = 1;  // Race condition!
}

// Solution: Use sig_atomic_t
volatile sig_atomic_t flag = 0;  // ✅ Atomic
```

### 2. Calling Unsafe Functions

```c
void handler(int sig) {
    printf("Signal!\n");  // ❌ NOT async-signal-safe!
    malloc(100);          // ❌ NOT async-signal-safe!
}

// Solution: Only set flags, use write() if needed
void handler(int sig) {
    write(STDOUT_FILENO, "Signal!\n", 8);  // ✅ Safe
}
```

### 3. Signal Masking Issues

```c
// Signals can be lost if not handled properly
signal(SIGINT, handler);
// SIGINT arrives here - might be lost!
signal(SIGINT, SIG_IGN);
```

### 4. Interrupted System Calls

```c
// read() can be interrupted by signal
ssize_t n = read(fd, buf, size);
if (n == -1 && errno == EINTR) {
    // Signal interrupted read - retry!
}

// Or use SA_RESTART flag
sa.sa_flags = SA_RESTART;
```

---

## 📈 Best Practices

1. **Minimal Work in Handlers**
   - Set flags only
   - Use sig_atomic_t
   - Defer work to main loop

2. **Use sigaction over signal**
   - More portable
   - Better control
   - Clearer semantics

3. **Block Signals in Critical Sections**
   - Protect shared data
   - Prevent races
   - Unblock after

4. **Use eventfd for Thread Safety**
   - Signal handler writes to eventfd
   - Main loop reads from eventfd
   - Fully async-signal-safe

5. **Handle EINTR**
   - Retry interrupted system calls
   - Or use SA_RESTART

6. **Reap Zombie Children**
   - Handle SIGCHLD
   - Call waitpid()
   - Prevent resource leaks

---

## 🎓 Key Takeaways

1. **Signals** are asynchronous notifications
2. **Async-signal-safe** functions only in handlers
3. **sig_atomic_t** for shared variables
4. **Minimal work** in signal handlers
5. **sigaction** preferred over signal
6. **Block signals** in critical sections
7. **eventfd** for thread-safe signal handling
8. **Handle EINTR** for interrupted syscalls

---

## 🚀 Next Steps

1. **01_basic_signal.c** - Simple signal handling
2. **02_sigaction.c** - Advanced signal handling
3. **03_signal_eventfd.c** - Thread-safe with eventfd
4. **04_timer_signal.c** - SIGALRM timer
5. **05_exercises.md** - Practice problems

---

**Ready to handle signals?** → `01_basic_signal.c`
