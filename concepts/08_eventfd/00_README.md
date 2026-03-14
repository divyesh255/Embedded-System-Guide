# eventfd - Event Notification

**Lightweight event notification mechanism**

---

## 🎯 What is eventfd?

**eventfd** is a Linux-specific file descriptor used for event notification between threads or processes. It's a lightweight alternative to pipes for signaling events.

```c
// Create eventfd
int efd = eventfd(0, 0);

// Thread 1: Signal event
uint64_t value = 1;
write(efd, &value, sizeof(value));

// Thread 2: Wait for event
uint64_t result;
read(efd, &result, sizeof(result));  // Blocks until event
```

Think of it as a **lightweight semaphore** implemented as a file descriptor.

---

## 🤔 Why Use eventfd?

**Problem:** Need lightweight event notification between threads/processes.

**Traditional approaches:**
- **Pipes:** Heavy (2 file descriptors, kernel buffers)
- **Signals:** Complex, unreliable
- **Condition variables:** Thread-only, no file descriptor

**eventfd advantages:**
- ✅ Single file descriptor
- ✅ Works with poll/epoll/select
- ✅ Inter-process capable
- ✅ Lightweight (just a counter)
- ✅ Integrates with event loops

---

## 📊 eventfd vs Other Mechanisms

| Feature | eventfd | Pipe | Semaphore | Condition Variable |
|---------|---------|------|-----------|-------------------|
| **File descriptor** | Yes | Yes | No | No |
| **poll/epoll** | Yes | Yes | No | No |
| **Inter-process** | Yes | Yes | Yes | No |
| **Overhead** | Low | Medium | Low | Low |
| **Counter** | Yes | No | Yes | No |

---

## 🔧 eventfd API

### Create eventfd

```c
#include <sys/eventfd.h>

// Create eventfd
int efd = eventfd(initval, flags);

// initval: Initial counter value (usually 0)
// flags: EFD_CLOEXEC, EFD_NONBLOCK, EFD_SEMAPHORE

// Example
int efd = eventfd(0, EFD_NONBLOCK);
if (efd == -1) {
    perror("eventfd");
}
```

### Write (Signal Event)

```c
uint64_t value = 1;
ssize_t s = write(efd, &value, sizeof(value));

// Adds 'value' to internal counter
// Never blocks (unless counter would overflow)
```

### Read (Wait for Event)

```c
uint64_t result;
ssize_t s = read(efd, &result, sizeof(result));

// Blocks until counter > 0
// Returns counter value and resets to 0
// (or decrements by 1 if EFD_SEMAPHORE)
```

### Close

```c
close(efd);
```

---

## 🎨 eventfd Flags

### EFD_NONBLOCK

```c
int efd = eventfd(0, EFD_NONBLOCK);

// read() returns immediately with EAGAIN if counter is 0
uint64_t value;
if (read(efd, &value, sizeof(value)) == -1) {
    if (errno == EAGAIN) {
        // No events available
    }
}
```

### EFD_SEMAPHORE

```c
int efd = eventfd(0, EFD_SEMAPHORE);

// read() returns 1 and decrements counter by 1
// (instead of returning counter and resetting to 0)

write(efd, &(uint64_t){5}, sizeof(uint64_t));  // counter = 5

uint64_t val;
read(efd, &val, sizeof(val));  // val = 1, counter = 4
read(efd, &val, sizeof(val));  // val = 1, counter = 3
```

### EFD_CLOEXEC

```c
int efd = eventfd(0, EFD_CLOEXEC);

// Close on exec() - prevents fd leak to child processes
```

---

## 💡 Common Use Cases

### 1. Thread Notification

```c
// Main thread
int efd = eventfd(0, 0);

// Worker thread signals completion
void* worker(void* arg) {
    do_work();
    
    uint64_t done = 1;
    write(efd, &done, sizeof(done));
    return NULL;
}

// Main thread waits
uint64_t result;
read(efd, &result, sizeof(result));  // Blocks until worker done
```

### 2. Event Loop Integration

```c
// Add eventfd to epoll
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = efd;
epoll_ctl(epollfd, EPOLL_CTL_ADD, efd, &ev);

// Worker signals event
write(efd, &(uint64_t){1}, sizeof(uint64_t));

// Event loop receives notification
struct epoll_event events[10];
int n = epoll_wait(epollfd, events, 10, -1);
```

### 3. Timer Expiration

```c
// Signal timer expiration
void timer_handler(int sig) {
    uint64_t exp = 1;
    write(timer_efd, &exp, sizeof(exp));
}

// Main loop waits for timer
uint64_t expirations;
read(timer_efd, &expirations, sizeof(expirations));
```

### 4. Resource Availability

```c
// Producer signals resource available
void produce() {
    add_to_queue(item);
    write(efd, &(uint64_t){1}, sizeof(uint64_t));
}

// Consumer waits for resource
void consume() {
    uint64_t count;
    read(efd, &count, sizeof(count));
    process_items(count);
}
```

---

## ⚠️ Common Pitfalls

### 1. Wrong Size in read/write

```c
uint64_t value = 1;
write(efd, &value, 4);  // ❌ Must be sizeof(uint64_t) = 8!
```

**Solution:** Always use `sizeof(uint64_t)`.

### 2. Overflow

```c
// Counter can overflow (max = UINT64_MAX - 1)
for (int i = 0; i < 1000000; i++) {
    write(efd, &(uint64_t){1}, sizeof(uint64_t));
}
// May block or fail if counter overflows!
```

**Solution:** Use EFD_SEMAPHORE or drain periodically.

### 3. Forgetting to Check Return Values

```c
write(efd, &value, sizeof(value));  // ❌ Didn't check return!
```

**Solution:** Always check for errors.

### 4. Blocking in Signal Handler

```c
void signal_handler(int sig) {
    uint64_t val;
    read(efd, &val, sizeof(val));  // ❌ May block!
}
```

**Solution:** Use EFD_NONBLOCK in signal handlers.

---

## 🔬 How eventfd Works

Internally, eventfd maintains:
- **64-bit counter**
- **Wait queue** for blocked readers

```
write(efd, 5):  counter += 5
read(efd):      returns counter, sets counter = 0
                (or returns 1, counter -= 1 if EFD_SEMAPHORE)
```

**Blocking behavior:**
- `read()` blocks if counter == 0
- `write()` blocks if counter would overflow (rare)

---

## 📈 Performance Characteristics

**Advantages:**
- Very lightweight (single counter)
- No kernel buffer allocation
- Fast (just atomic counter operations)
- Integrates with epoll/poll

**Overhead:**
- System call overhead (read/write)
- Context switch if blocking

**Best for:**
- ✅ Event notification
- ✅ Waking event loops
- ✅ Simple signaling
- ✅ Integration with poll/epoll

---

## 🎓 Key Takeaways

1. **eventfd** is a lightweight event notification mechanism
2. **File descriptor** - works with poll/epoll/select
3. **Counter-based** - accumulates events
4. **read()** blocks until counter > 0
5. **write()** increments counter
6. **EFD_SEMAPHORE** - semaphore-like behavior
7. **EFD_NONBLOCK** - non-blocking mode
8. **Linux-specific** - not portable to other Unix systems

---

## 🚀 Next Steps

1. **01_basic_eventfd.c** - Simple event notification
2. **02_thread_notification.c** - Thread signaling
3. **03_epoll_integration.c** - Event loop with eventfd
4. **04_semaphore_mode.c** - EFD_SEMAPHORE usage
5. **05_exercises.md** - Practice problems

---

**Ready to use eventfd?** → `01_basic_eventfd.c`
