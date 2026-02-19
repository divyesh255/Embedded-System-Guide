# Threads - POSIX Threads (pthreads)

**Study Time:** 30 minutes  
**Difficulty:** Beginner  
**Prerequisites:** C programming basics

## 📖 What Are Threads?

A **thread** is the smallest unit of execution within a process. Think of a process as a program running on your computer, and threads as multiple workers within that program, all working simultaneously.

### Real-World Analogy
Imagine a restaurant:
- **Process** = The entire restaurant
- **Main Thread** = The head chef
- **Additional Threads** = Assistant chefs

All chefs (threads) work in the same kitchen (process memory), sharing the same ingredients (data), but each can work on different dishes (tasks) simultaneously.

## 🤔 Why Use Threads?

### 1. **Parallelism**
- Utilize multiple CPU cores
- Perform multiple tasks simultaneously
- Improve performance on multi-core systems

### 2. **Responsiveness**
- Keep UI responsive while processing
- Handle multiple clients simultaneously
- Don't block on slow operations

### 3. **Resource Sharing**
- Threads share memory space
- Easier communication than processes
- Lower overhead than creating processes

### 4. **Modularity**
- Separate concerns (UI, processing, I/O)
- Cleaner code organization
- Easier to maintain

## 🆚 Process vs Thread

| Aspect | Process | Thread |
|--------|---------|--------|
| Memory | Separate address space | Shared address space |
| Creation | Expensive (~1-2ms) | Cheap (~10-100μs) |
| Communication | IPC (pipes, sockets) | Shared memory |
| Isolation | High (protected) | Low (shared data) |
| Overhead | High | Low |
| Use Case | Independent programs | Concurrent tasks |

## 🔄 Thread Lifecycle

```
   [CREATE]
      ↓
   [READY] ←→ [RUNNING]
      ↓           ↓
   [BLOCKED]  [TERMINATED]
```

1. **Create**: `pthread_create()` - Thread is born
2. **Ready**: Waiting for CPU time
3. **Running**: Executing code
4. **Blocked**: Waiting for resource (I/O, lock, etc.)
5. **Terminated**: `pthread_exit()` or return from function

## 🛠️ POSIX Threads API

### Core Functions

```c
#include <pthread.h>

// Create a new thread
int pthread_create(pthread_t *thread,           // Thread ID (output)
                   const pthread_attr_t *attr,  // Attributes (NULL = default)
                   void *(*start_routine)(void*), // Function to run
                   void *arg);                  // Argument to function

// Wait for thread to finish
int pthread_join(pthread_t thread,    // Thread to wait for
                 void **retval);      // Return value (output)

// Exit from thread
void pthread_exit(void *retval);      // Return value

// Get current thread ID
pthread_t pthread_self(void);
```

### Return Values
- **0** = Success
- **Non-zero** = Error code (EAGAIN, EINVAL, etc.)

## 💡 Key Concepts

### 1. Thread Function Signature
```c
void *thread_function(void *arg) {
    // Your code here
    return NULL;  // or pthread_exit(NULL);
}
```
- Must return `void*`
- Takes `void*` argument
- Can return any pointer type

### 2. Passing Arguments
```c
int data = 42;
pthread_create(&thread, NULL, func, &data);  // Pass address
```
⚠️ **Warning**: Be careful with stack variables!

### 3. Joining Threads
```c
pthread_join(thread, NULL);  // Wait for thread
```
- Blocks until thread finishes
- Cleans up thread resources
- Gets return value

### 4. Detached Threads
```c
pthread_detach(thread);  // Don't need to join
```
- Resources freed automatically
- Can't join later
- Use for "fire and forget" tasks

## ⚠️ Common Pitfalls

### 1. **Forgetting to Join**
```c
// BAD - Thread may not finish before main exits
pthread_create(&thread, NULL, func, NULL);
// main exits immediately!

// GOOD - Wait for thread
pthread_create(&thread, NULL, func, NULL);
pthread_join(thread, NULL);
```

### 2. **Passing Stack Variables**
```c
// BAD - Variable may be destroyed
void create_thread() {
    int data = 42;
    pthread_create(&thread, NULL, func, &data);
    // data destroyed when function returns!
}

// GOOD - Use heap or global
int *data = malloc(sizeof(int));
*data = 42;
pthread_create(&thread, NULL, func, data);
```

### 3. **Race Conditions**
```c
// BAD - Multiple threads accessing shared data
int counter = 0;
void *increment(void *arg) {
    counter++;  // NOT thread-safe!
}

// GOOD - Use mutex (covered in next module)
pthread_mutex_t lock;
void *increment(void *arg) {
    pthread_mutex_lock(&lock);
    counter++;
    pthread_mutex_unlock(&lock);
}
```

### 4. **Not Checking Return Values**
```c
// BAD
pthread_create(&thread, NULL, func, NULL);

// GOOD
if (pthread_create(&thread, NULL, func, NULL) != 0) {
    perror("pthread_create failed");
    exit(1);
}
```

## ✅ Best Practices

1. **Always check return values**
   ```c
   if (pthread_create(...) != 0) { /* handle error */ }
   ```

2. **Join or detach every thread**
   ```c
   pthread_join(thread, NULL);  // or pthread_detach(thread);
   ```

3. **Use proper argument passing**
   ```c
   // Allocate on heap for thread arguments
   int *arg = malloc(sizeof(int));
   ```

4. **Keep thread functions simple**
   ```c
   // One clear purpose per thread
   void *worker_thread(void *arg) {
       // Do one thing well
   }
   ```

5. **Avoid global variables**
   ```c
   // Pass data through arguments instead
   ```

## 🎯 When to Use Threads

### ✅ Good Use Cases
- **I/O-bound tasks**: Network, disk, user input
- **Parallel processing**: Image processing, calculations
- **Background tasks**: Logging, monitoring
- **Server applications**: Handle multiple clients
- **UI applications**: Keep interface responsive

### ❌ Avoid Threads When
- **Simple sequential tasks**: Overhead not worth it
- **Heavy CPU tasks on single core**: No benefit
- **Debugging is critical**: Harder to debug
- **Simple scripts**: Overkill for simple programs

## 📊 Performance Considerations

### Thread Creation Overhead
- **Time**: ~10-100 microseconds
- **Memory**: ~2-8 MB per thread (stack)
- **Limit**: Typically 1000-10000 threads per process

### When Threads Help
```
Single-threaded: Task1 → Task2 → Task3 (30 seconds)
Multi-threaded:  Task1 ↘
                 Task2 → (10 seconds)
                 Task3 ↗
```

### When Threads Don't Help
```
CPU-bound on single core:
Thread1: ████████████ (12 seconds)
Thread2: ████████████ (12 seconds)
Total: 24 seconds (worse due to context switching!)
```

## 🔍 Debugging Tips

1. **Use thread-safe printf**
   ```c
   fprintf(stderr, "[Thread %lu] Message\n", pthread_self());
   ```

2. **Add thread IDs to logs**
   ```c
   printf("Thread %lu: Starting\n", pthread_self());
   ```

3. **Use tools**
   ```bash
   valgrind --tool=helgrind ./program  # Detect race conditions
   gdb --args ./program                # Debug with GDB
   ```

4. **Start simple**
   - Test with one thread first
   - Add threads incrementally
   - Verify each step

## 📚 What's Next?

After mastering threads, you'll need to learn:
1. **Mutex** (02_mutex) - Protect shared data
2. **Condition Variables** (03_condition_variables) - Efficient waiting
3. **Atomic Operations** (04_atomic_operations) - Lock-free programming

## 🚀 Ready to Code?

Now that you understand the theory, let's see threads in action!

**Next Steps:**
1. Run `01_basic_thread.c` - Your first thread
2. Run `02_thread_args.c` - Passing data
3. Run `03_multiple_threads.c` - Multiple workers
4. Run `04_thread_join.c` - Synchronization
5. Complete `05_exercises.md` - Practice!

---

**Time to get hands-on!** → Start with `01_basic_thread.c`
