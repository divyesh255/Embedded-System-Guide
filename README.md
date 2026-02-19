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

### 🚧 Coming Soon
- 02_mutex - Mutual Exclusion
- 03_condition_variables - Thread Synchronization
- 04_atomic_operations - Lock-Free Programming
- 05_semaphores - Resource Management
- 06_spinlocks - Fast Locking
- 07_eventfd - Event Notification
- 08_signal_handling - Signal Management

## 🚀 Quick Start

```bash
# Clone or download this repository
cd Embedded-System-Guide

# Build all examples
make all

# Or build specific module
cd concepts/01_threads
make all

# Run examples
./01_basic_thread
./02_thread_args
./03_multiple_threads
./04_thread_join
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
    └── 01_threads/
        ├── 00_README.md           # Theory
        ├── 01_basic_thread.c      # Example 1
        ├── 02_thread_args.c       # Example 2
        ├── 03_multiple_threads.c  # Example 3
        ├── 04_thread_join.c       # Example 4
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

## 🤝 Contributing

This is a living guide. Suggestions for improvements are welcome!

## 📝 License

Educational use - Free to learn, share, and build upon.

## 🚀 Let's Begin!

Start with: `concepts/01_threads/00_README.md`

Happy Learning! 🎉