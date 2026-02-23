# Master Makefile for Embedded Systems Learning Guide
# Builds all concept modules

.PHONY: all clean help threads mutex condvar

# Default target - build all modules
all: threads mutex condvar
	@echo ""
	@echo "✓ All modules built successfully!"
	@echo ""
	@echo "To run examples:"
	@echo "  cd concepts/01_threads && ./01_basic_thread"
	@echo "  cd concepts/02_mutex && ./01_race_condition"
	@echo "  cd concepts/03_condition_variables && ./02_condvar_good"

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

# Clean all modules
clean:
	@echo "Cleaning all modules..."
	@$(MAKE) -C concepts/01_threads clean
	@$(MAKE) -C concepts/02_mutex clean
	@$(MAKE) -C concepts/03_condition_variables clean
	@echo "✓ All modules cleaned"

# Show help
help:
	@echo "Embedded Systems Learning Guide - Master Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build all modules"
	@echo "  make threads  - Build only threads module"
	@echo "  make mutex    - Build only mutex module"
	@echo "  make condvar  - Build only condition variables module"
	@echo "  make clean    - Clean all build artifacts"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Current modules:"
	@echo "  ✓ 01_threads - POSIX Threads"
	@echo "  ✓ 02_mutex - Mutual Exclusion"
	@echo "  ✓ 03_condition_variables - Efficient Synchronization"
	@echo ""
	@echo "To get started:"
	@echo "  1. make"
	@echo "  2. cd concepts/01_threads"
	@echo "  3. ./01_basic_thread"
