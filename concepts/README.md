# Embedded Systems Learning Guide

A comprehensive, hands-on guide to mastering embedded systems programming concepts through practical examples and exercises.

## 🎯 Purpose

This guide provides a structured learning path for embedded systems programming, covering essential concepts with:
- Clear theoretical explanations
- Runnable code examples
- Progressive complexity
- Practice exercises with solutions

## 📚 Current Modules

### ✅ Completed
- **01_threads** - POSIX Threads (pthreads)
- **02_mutex** - Mutual Exclusion
- **03_condition_variables** - Efficient Thread Synchronization
- **04_semaphores** - Resource Management
- **05_atomic_operations** - Lock-Free Programming

### 🚧 Coming Soon
- 06_spinlocks - Fast Locking
- 07_eventfd - Event Notification
- 08_signal_handling - Signal Management

## 🚀 Quick Start

```bash
# Clone or download this repository
cd Embedded-System-Guide

# Build all examples
make

# Or build specific module
make threads
make mutex
make condvar

# Run examples
cd concepts/01_threads
./01_basic_thread
./02_thread_args

cd ../02_mutex
./01_race_condition
./02_mutex_solution

cd ../03_condition_variables
./02_condvar_good
./03_producer_consumer

cd ../04_semaphores
./01_binary_semaphore
./02_counting_semaphore

cd ../05_atomic_operations
./01_atomic_counter
./02_compare_and_swap
```

## 📖 How to Use This Guide

### For Each Module:

1. **Read Theory** (30-45 min)
   - Start with `00_README.md` in each concept directory
   - Understand the "what", "why", and "when"

2. **Study Examples** (1-2 hours)
   - Run each example program
   - Read the code and comments carefully
   - Experiment with modifications

3. **Practice** (45-60 min)
   - Complete exercises in `05_exercises.md`
   - Try to solve before looking at solutions
   - Understand why each solution works

4. **Review** (15-30 min)
   - Revisit theory with new understanding
   - Note key takeaways
   - Move to next module

## 🛠️ Prerequisites

### Required Knowledge
- C programming basics
- Linux/Unix command line
- Basic understanding of processes

### System Requirements
- Linux or WSL2
- GCC compiler
- Make build tool
- POSIX threads library (pthread)

### Installation (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential
```

## 📂 Repository Structure

```
Embedded-System-Guide/
├── README.md              # This file
├── Makefile               # Master build file
└── concepts/
    ├── 01_threads/
    │   ├── 00_README.md           # Theory
    │   ├── 01_basic_thread.c      # Example 1
    │   ├── 02_thread_args.c       # Example 2
    │   ├── 03_multiple_threads.c  # Example 3
    │   ├── 04_thread_join.c       # Example 4
    │   ├── 05_exercises.md        # Practice
    │   └── Makefile               # Build examples
    ├── 02_mutex/
    │   ├── 00_README.md           # Theory
    │   ├── 01_race_condition.c    # Problem demo
    │   ├── 02_mutex_solution.c    # Solution
    │   ├── 03_deadlock.c          # Deadlock demo
    │   ├── 04_trylock.c           # Non-blocking
    │   ├── 05_exercises.md        # Practice
    │   └── Makefile               # Build examples
    ├── 03_condition_variables/
    │   ├── 00_README.md           # Theory
    │   ├── 01_busy_wait_bad.c     # Problem demo
    │   ├── 02_condvar_good.c      # Solution
    │   ├── 03_producer_consumer.c # Classic pattern
    │   ├── 04_spurious_wakeup.c   # Edge cases
    │   ├── 05_exercises.md        # Practice
    │   └── Makefile               # Build examples
    ├── 04_semaphores/
    │   ├── 00_README.md           # Theory
    │   ├── 01_binary_semaphore.c  # Binary semaphore
    │   ├── 02_counting_semaphore.c # Resource pool
    │   ├── 03_producer_consumer.c # Classic pattern
    │   ├── 04_rate_limiter.c      # Rate limiting
    │   ├── 05_exercises.md        # Practice
    │   └── Makefile               # Build examples
    └── 05_atomic_operations/
        ├── 00_README.md           # Theory
        ├── 01_atomic_counter.c    # Lock-free counter
        ├── 02_compare_and_swap.c  # CAS operation
        ├── 03_spinlock.c          # Spinlock
        ├── 04_reference_counting.c # Refcounting
        ├── 05_exercises.md        # Practice
        └── Makefile               # Build examples
```

## 🎓 Learning Objectives

By completing this guide, you will:
- ✅ Understand multi-threaded programming
- ✅ Master synchronization primitives
- ✅ Write thread-safe code
- ✅ Debug concurrent programs
- ✅ Apply best practices
- ✅ Build real-world embedded systems

## 💡 Tips for Success

1. **Don't Rush** - Take time to understand each concept
2. **Experiment** - Modify examples, break things, learn
3. **Practice** - Do all exercises, even if they seem easy
4. **Review** - Revisit concepts as you progress
5. **Build** - Create your own projects using these concepts

## 📊 Module Overview

| Module | Topic | Files | Study Time | Difficulty |
|--------|-------|-------|------------|------------|
| 01 | Threads | 7 | 2.5 hours | Beginner |
| 02 | Mutex | 6 | 2.5 hours | Beginner-Intermediate |
| 03 | Condition Variables | 6 | 3 hours | Intermediate |
| 04 | Semaphores | 6 | 2.5 hours | Intermediate |
| 05 | Atomic Operations | 6 | 2.5 hours | Intermediate-Advanced |

**Total:** 31 files, ~13 hours of study material

## 🤝 Contributing

This is a living guide. Suggestions for improvements are welcome!

## 📝 License

Educational use - Free to learn, share, and build upon.

## 🚀 Let's Begin!

Start with: `concepts/01_threads/00_README.md`

Happy Learning! 🎉
