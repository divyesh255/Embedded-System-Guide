# Signal Handling - Practice Exercises

Test your understanding of signal handling with these hands-on exercises!

---

## 🎯 Exercise 1: Multi-Signal Handler (Easy - 15 min)

Handle multiple signals with a single handler.

**Task:** Create a program that handles SIGINT, SIGTERM, and SIGUSR1 with one handler that prints which signal was received.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t running = 1;

void signal_handler(int signum) {
    const char *msg;
    
    switch (signum) {
        case SIGINT:
            msg = "SIGINT received\n";
            break;
        case SIGTERM:
            msg = "SIGTERM received\n";
            running = 0;
            break;
        case SIGUSR1:
            msg = "SIGUSR1 received\n";
            break;
        default:
            msg = "Unknown signal\n";
    }
    
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    
    printf("PID: %d\n", getpid());
    printf("Send signals: kill -INT/-TERM/-USR1 %d\n", getpid());
    
    while (running) {
        pause();  // Wait for signal
    }
    
    return 0;
}
```

**Key points:**
- One handler for multiple signals
- Use switch to differentiate
- Only async-signal-safe functions
</details>

---

## 🎯 Exercise 2: Signal Blocking (Medium - 20 min)

Implement critical section protection using signal blocking.

**Task:** Block SIGINT during critical section, unblock after.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t signal_count = 0;

void handler(int sig) {
    signal_count++;
    write(STDOUT_FILENO, "Signal!\n", 8);
}

int main(void) {
    signal(SIGINT, handler);
    
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    
    printf("Press Ctrl+C during critical section\n");
    
    for (int i = 0; i < 5; i++) {
        printf("\n=== Iteration %d ===\n", i);
        
        // Block SIGINT
        sigprocmask(SIG_BLOCK, &set, &oldset);
        printf("Critical section START (signals blocked)\n");
        sleep(3);
        printf("Critical section END\n");
        
        // Unblock SIGINT
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        printf("Signals unblocked\n");
        
        sleep(1);
    }
    
    printf("\nTotal signals: %d\n", signal_count);
    return 0;
}
```

**Key concepts:**
- sigprocmask() blocks/unblocks signals
- Signals queued while blocked
- Delivered when unblocked
</details>

---

## 🎯 Exercise 3: Timeout Implementation (Medium - 25 min)

Implement operation timeout using SIGALRM.

**Task:** Read from stdin with 5-second timeout.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

static jmp_buf jmpbuf;
static volatile sig_atomic_t timeout_occurred = 0;

void alarm_handler(int sig) {
    timeout_occurred = 1;
    longjmp(jmpbuf, 1);
}

int main(void) {
    char buffer[100];
    
    signal(SIGALRM, alarm_handler);
    
    printf("Enter text (5 second timeout): ");
    fflush(stdout);
    
    if (setjmp(jmpbuf) == 0) {
        alarm(5);  // 5 second timeout
        
        if (fgets(buffer, sizeof(buffer), stdin)) {
            alarm(0);  // Cancel alarm
            printf("You entered: %s", buffer);
        }
    } else {
        printf("\nTimeout! No input received\n");
    }
    
    return 0;
}
```

**Advanced concepts:**
- setjmp/longjmp for timeout
- alarm() for timer
- Cancel alarm if operation completes
</details>

---

## 🎯 Exercise 4: Child Process Reaping (Hard - 30 min)

Handle SIGCHLD to reap zombie children.

**Task:** Fork multiple children, reap them properly with SIGCHLD handler.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

volatile sig_atomic_t children_done = 0;

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    
    // Reap all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        children_done++;
        
        char msg[50];
        snprintf(msg, sizeof(msg), "Child %d exited\n", pid);
        write(STDOUT_FILENO, msg, strlen(msg));
    }
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);
    
    printf("Forking 5 children...\n");
    
    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            printf("Child %d starting\n", getpid());
            sleep(i + 1);  // Variable work time
            printf("Child %d done\n", getpid());
            exit(0);
        }
    }
    
    // Parent waits for all children
    while (children_done < 5) {
        printf("Waiting... (%d/5 done)\n", children_done);
        sleep(1);
    }
    
    printf("All children reaped!\n");
    return 0;
}
```

**Key points:**
- SIGCHLD sent when child terminates
- waitpid() with WNOHANG in handler
- Loop to reap all children
- Prevents zombie processes
</details>

---

## 🎯 Exercise 5: Production Signal Handler (Advanced - 35 min)

Implement production-grade signal handling with eventfd.

**Task:** Multi-threaded server with graceful shutdown via signals and eventfd.

<details>
<summary>Hint</summary>

Combine concepts:
- eventfd for signal notification
- Multiple worker threads
- Graceful shutdown on SIGINT/SIGTERM
- Clean resource cleanup
- Use poll() for event loop
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **Signal Basics**
   - signal() vs sigaction()
   - Async-signal-safe functions
   - sig_atomic_t and volatile

2. **Signal Blocking**
   - sigprocmask() for blocking
   - Critical section protection
   - Signal queuing

3. **Timers**
   - alarm() for timeouts
   - SIGALRM handling
   - Timeout patterns

4. **Process Management**
   - SIGCHLD for child reaping
   - waitpid() in handler
   - Zombie prevention

5. **Production Patterns**
   - eventfd for thread safety
   - Graceful shutdown
   - Event loop integration

6. **Best Practices**
   - Minimal work in handlers
   - Use eventfd for complex handling
   - Always check async-signal-safety
   - Block signals in critical sections

---

**Congratulations!** You've mastered signal handling! 🎉

**Next:** You've completed the entire Concurrent Programming track! 🎊
