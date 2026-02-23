# Makefile for Semaphore Examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGETS = 01_binary_semaphore 02_counting_semaphore 03_producer_consumer 04_rate_limiter

.PHONY: all clean test help

# Build all examples
all: $(TARGETS)
	@echo "✓ All semaphore examples compiled successfully!"
	@echo "Run './01_binary_semaphore' to start"

# Individual targets
01_binary_semaphore: 01_binary_semaphore.c
	$(CC) $(CFLAGS) -o $@ $<

02_counting_semaphore: 02_counting_semaphore.c
	$(CC) $(CFLAGS) -o $@ $<

03_producer_consumer: 03_producer_consumer.c
	$(CC) $(CFLAGS) -o $@ $<

04_rate_limiter: 04_rate_limiter.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "✓ Cleaned all binaries"

# Run examples
test: all
	@echo "=== Running 01_binary_semaphore ==="
	@./01_binary_semaphore
	@echo ""
	@echo "=== Running 02_counting_semaphore ==="
	@timeout 30 ./02_counting_semaphore || true
	@echo ""
	@echo "=== Running 03_producer_consumer ==="
	@timeout 15 ./03_producer_consumer || true
	@echo ""
	@echo "=== Running 04_rate_limiter ==="
	@timeout 15 ./04_rate_limiter || true

# Show help
help:
	@echo "Semaphore Examples Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all examples"
	@echo "  make clean    - Remove all binaries"
	@echo "  make test     - Build and run examples"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Individual examples:"
	@echo "  make 01_binary_semaphore"
	@echo "  make 02_counting_semaphore"
	@echo "  make 03_producer_consumer"
	@echo "  make 04_rate_limiter"
