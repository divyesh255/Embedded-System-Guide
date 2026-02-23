# Makefile for Atomic Operations Examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = 01_atomic_counter 02_compare_and_swap 03_spinlock 04_reference_counting

.PHONY: all clean test help

# Build all examples
all: $(TARGETS)
	@echo "✓ All atomic operations examples compiled successfully!"
	@echo "Run './01_atomic_counter' to start"

# Individual targets
01_atomic_counter: 01_atomic_counter.c
	$(CC) $(CFLAGS) -o $@ $<

02_compare_and_swap: 02_compare_and_swap.c
	$(CC) $(CFLAGS) -o $@ $<

03_spinlock: 03_spinlock.c
	$(CC) $(CFLAGS) -o $@ $<

04_reference_counting: 04_reference_counting.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "✓ Cleaned all binaries"

# Run examples
test: all
	@echo "=== Running 01_atomic_counter ==="
	@./01_atomic_counter
	@echo ""
	@echo "=== Running 02_compare_and_swap ==="
	@./02_compare_and_swap
	@echo ""
	@echo "=== Running 03_spinlock ==="
	@./03_spinlock
	@echo ""
	@echo "=== Running 04_reference_counting ==="
	@./04_reference_counting

# Show help
help:
	@echo "Atomic Operations Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all examples"
	@echo "  make clean    - Remove all binaries"
	@echo "  make test     - Build and run examples"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Individual examples:"
	@echo "  make 01_atomic_counter"
	@echo "  make 02_compare_and_swap"
	@echo "  make 03_spinlock"
	@echo "  make 04_reference_counting"
	@echo ""
	@echo "Note: Requires C11 support (gcc 4.9+)"
