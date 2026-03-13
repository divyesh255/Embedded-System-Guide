# Makefile for Spinlocks examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c11
TARGETS = 01_naive_spinlock 02_atomic_spinlock 03_test_and_test_and_set 04_ticket_spinlock

.PHONY: all clean

all: $(TARGETS)

01_naive_spinlock: 01_naive_spinlock.c
	$(CC) $(CFLAGS) $< -o $@

02_atomic_spinlock: 02_atomic_spinlock.c
	$(CC) $(CFLAGS) $< -o $@

03_test_and_test_and_set: 03_test_and_test_and_set.c
	$(CC) $(CFLAGS) $< -o $@

04_ticket_spinlock: 04_ticket_spinlock.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

run: all
	@echo "=== Running Spinlock Examples ==="
	@echo
	@echo "--- 01: Naive Spinlock (BROKEN!) ---"
	./01_naive_spinlock
	@echo
	@echo "--- 02: Atomic Spinlock (CORRECT!) ---"
	./02_atomic_spinlock
	@echo
	@echo "--- 03: Test-and-Test-and-Set Optimization ---"
	./03_test_and_test_and_set
	@echo
	@echo "--- 04: Ticket Spinlock (Fair) ---"
	./04_ticket_spinlock
