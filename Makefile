# Master Makefile for Embedded Systems Learning Guide
# Builds all concept modules

.PHONY: all clean help threads

# Default target - build all modules
all: threads
	@echo ""
	@echo "✓ All modules built successfully!"
	@echo ""
	@echo "To run examples:"
	@echo "  cd concepts/01_threads && ./01_basic_thread"

# Build threads module
threads:
	@echo "Building threads module..."
	@$(MAKE) -C concepts/01_threads

# Clean all modules
clean:
	@echo "Cleaning all modules..."
	@$(MAKE) -C concepts/01_threads clean
	@echo "✓ All modules cleaned"

# Show help
help:
	@echo "Embedded Systems Learning Guide - Master Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build all modules"
	@echo "  make threads  - Build only threads module"
	@echo "  make clean    - Clean all build artifacts"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Current modules:"
	@echo "  ✓ 01_threads - POSIX Threads"
	@echo ""
	@echo "To get started:"
	@echo "  1. make"
	@echo "  2. cd concepts/01_threads"
	@echo "  3. ./01_basic_thread"
