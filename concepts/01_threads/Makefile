# Makefile for Thread Examples
# Compiles all thread examples in this directory

CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = 01_basic_thread 02_thread_args 03_multiple_threads 04_thread_join

.PHONY: all clean test help

# Build all examples
all: $(TARGETS)
	@echo "✓ All thread examples compiled successfully!"
	@echo "Run './01_basic_thread' to start"

# Individual targets
01_basic_thread: 01_basic_thread.c
	$(CC) $(CFLAGS) -o $@ $<

02_thread_args: 02_thread_args.c
	$(CC) $(CFLAGS) -o $@ $<

03_multiple_threads: 03_multiple_threads.c
	$(CC) $(CFLAGS) -o $@ $<

04_thread_join: 04_thread_join.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "✓ Cleaned all binaries"

# Run all examples (for testing)
test: all
	@echo "=== Running 01_basic_thread ==="
	@./01_basic_thread
	@echo ""
	@echo "=== Running 02_thread_args ==="
	@./02_thread_args
	@echo ""
	@echo "=== Running 03_multiple_threads ==="
	@./03_multiple_threads
	@echo ""
	@echo "=== Running 04_thread_join ==="
	@./04_thread_join

# Show help
help:
	@echo "Thread Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all examples"
	@echo "  make clean    - Remove all binaries"
	@echo "  make test     - Build and run all examples"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Individual examples:"
	@echo "  make 01_basic_thread"
	@echo "  make 02_thread_args"
	@echo "  make 03_multiple_threads"
	@echo "  make 04_thread_join"
