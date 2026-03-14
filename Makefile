# Master Makefile for Embedded Systems Learning Guide
# Builds all concept modules

.PHONY: all clean help threads mutex condvar semaphores atomic spinlocks rwlock eventfd signals

# Default target - build all modules
all: threads mutex condvar semaphores atomic spinlocks rwlock eventfd signals
	@echo ""
	@echo "✓ All modules built successfully!"
	@echo ""
	@echo "To run examples:"
	@echo "  cd concepts/01_threads && ./01_basic_thread"
	@echo "  cd concepts/02_mutex && ./01_race_condition"
	@echo "  cd concepts/03_condition_variables && ./02_condvar_good"
	@echo "  cd concepts/04_semaphores && ./01_binary_semaphore"
	@echo "  cd concepts/05_atomic_operations && ./01_atomic_counter"
	@echo "  cd concepts/06_spinlocks && ./02_atomic_spinlock"
	@echo "  cd concepts/07_rwlock && ./01_mutex_vs_rwlock"
	@echo "  cd concepts/08_eventfd && ./01_basic_eventfd"
	@echo "  cd concepts/09_signals && ./01_basic_signal"

# Build threads module
threads:
	@echo "Building threads module..."
	@$(MAKE) -C concepts/01_threads

# Build mutex module
mutex:
	@echo "Building mutex module..."
	@$(MAKE) -C concepts/02_mutex

# Build condition variables module
condvar:
	@echo "Building condition variables module..."
	@$(MAKE) -C concepts/03_condition_variables

# Build semaphores module
semaphores:
	@echo "Building semaphores module..."
	@$(MAKE) -C concepts/04_semaphores

# Build atomic operations module
atomic:
	@echo "Building atomic operations module..."
	@$(MAKE) -C concepts/05_atomic_operations

# Build spinlocks module
spinlocks:
	@echo "Building spinlocks module..."
	@$(MAKE) -C concepts/06_spinlocks

# Build rwlock module
rwlock:
	@echo "Building rwlock module..."
	@$(MAKE) -C concepts/07_rwlock

# Build eventfd module
eventfd:
	@echo "Building eventfd module..."
	@$(MAKE) -C concepts/08_eventfd

# Build signals module
signals:
	@echo "Building signals module..."
	@$(MAKE) -C concepts/09_signals

# Clean all modules
clean:
	@echo "Cleaning all modules..."
	@$(MAKE) -C concepts/01_threads clean
	@$(MAKE) -C concepts/02_mutex clean
	@$(MAKE) -C concepts/03_condition_variables clean
	@$(MAKE) -C concepts/04_semaphores clean
	@$(MAKE) -C concepts/05_atomic_operations clean
	@$(MAKE) -C concepts/06_spinlocks clean
	@$(MAKE) -C concepts/07_rwlock clean
	@$(MAKE) -C concepts/08_eventfd clean
	@$(MAKE) -C concepts/09_signals clean
	@echo "✓ All modules cleaned"

# Show help
help:
	@echo "Embedded Systems Learning Guide - Master Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make            - Build all modules"
	@echo "  make threads    - Build only threads module"
	@echo "  make mutex      - Build only mutex module"
	@echo "  make condvar    - Build only condition variables module"
	@echo "  make semaphores - Build only semaphores module"
	@echo "  make atomic     - Build only atomic operations module"
	@echo "  make spinlocks  - Build only spinlocks module"
	@echo "  make rwlock     - Build only rwlock module"
	@echo "  make eventfd    - Build only eventfd module"
	@echo "  make signals    - Build only signals module"
	@echo "  make clean      - Clean all build artifacts"
	@echo "  make help       - Show this help"
	@echo ""
	@echo "Current modules:"
	@echo "  ✓ 01_threads - POSIX Threads"
	@echo "  ✓ 02_mutex - Mutual Exclusion"
	@echo "  ✓ 03_condition_variables - Efficient Synchronization"
	@echo "  ✓ 04_semaphores - Resource Management"
	@echo "  ✓ 05_atomic_operations - Lock-Free Programming"
	@echo "  ✓ 06_spinlocks - Busy-Wait Synchronization"
	@echo "  ✓ 07_rwlock - Read-Write Locks"
	@echo "  ✓ 08_eventfd - Event Notification"
	@echo "  ✓ 09_signals - Signal Handling"
	@echo ""
	@echo "To get started:"
	@echo "  1. make"
	@echo "  2. cd concepts/01_threads"
	@echo "  3. ./01_basic_thread"
