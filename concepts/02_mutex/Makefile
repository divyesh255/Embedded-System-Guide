# Makefile for Mutex Examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = 01_race_condition 02_mutex_solution 03_deadlock 04_trylock

.PHONY: all clean test help

# Build all examples
all: $(TARGETS)
	@echo "✓ All mutex examples compiled successfully!"
	@echo "Run './01_race_condition' to see the problem"
	@echo "Run './02_mutex_solution' to see the fix"

# Individual targets
01_race_condition: 01_race_condition.c
	$(CC) $(CFLAGS) -o $@ $<

02_mutex_solution: 02_mutex_solution.c
	$(CC) $(CFLAGS) -o $@ $<

03_deadlock: 03_deadlock.c
	$(CC) $(CFLAGS) -o $@ $<

04_trylock: 04_trylock.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "✓ Cleaned all binaries"

# Run examples (skip deadlock as it hangs)
test: all
	@echo "=== Running 01_race_condition ==="
	@./01_race_condition
	@echo ""
	@echo "=== Running 02_mutex_solution ==="
	@./02_mutex_solution
	@echo ""
	@echo "=== Skipping 03_deadlock (would hang) ==="
	@echo ""
	@echo "=== Running 04_trylock ==="
	@./04_trylock

# Show help
help:
	@echo "Mutex Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all examples"
	@echo "  make clean    - Remove all binaries"
	@echo "  make test     - Build and run examples (skip deadlock)"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Individual examples:"
	@echo "  make 01_race_condition"
	@echo "  make 02_mutex_solution"
	@echo "  make 03_deadlock"
	@echo "  make 04_trylock"
	@echo ""
	@echo "WARNING: 03_deadlock will hang - press Ctrl+C to exit"
