# Makefile for Condition Variable Examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = 01_busy_wait_bad 02_condvar_good 03_producer_consumer 04_spurious_wakeup

.PHONY: all clean test help

# Build all examples
all: $(TARGETS)
	@echo "✓ All condition variable examples compiled successfully!"
	@echo "Run './01_busy_wait_bad' to see the problem"
	@echo "Run './02_condvar_good' to see the solution"

# Individual targets
01_busy_wait_bad: 01_busy_wait_bad.c
	$(CC) $(CFLAGS) -o $@ $<

02_condvar_good: 02_condvar_good.c
	$(CC) $(CFLAGS) -o $@ $<

03_producer_consumer: 03_producer_consumer.c
	$(CC) $(CFLAGS) -o $@ $<

04_spurious_wakeup: 04_spurious_wakeup.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "✓ Cleaned all binaries"

# Run examples
test: all
	@echo "=== Running 01_busy_wait_bad ==="
	@timeout 10 ./01_busy_wait_bad || true
	@echo ""
	@echo "=== Running 02_condvar_good ==="
	@./02_condvar_good
	@echo ""
	@echo "=== Running 03_producer_consumer ==="
	@timeout 10 ./03_producer_consumer || true
	@echo ""
	@echo "=== Running 04_spurious_wakeup ==="
	@./04_spurious_wakeup

# Show help
help:
	@echo "Condition Variable Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all examples"
	@echo "  make clean    - Remove all binaries"
	@echo "  make test     - Build and run examples"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Individual examples:"
	@echo "  make 01_busy_wait_bad"
	@echo "  make 02_condvar_good"
	@echo "  make 03_producer_consumer"
	@echo "  make 04_spurious_wakeup"
