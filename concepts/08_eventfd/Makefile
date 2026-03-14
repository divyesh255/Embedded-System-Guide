# Makefile for eventfd examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c11
TARGETS = 01_basic_eventfd 02_thread_notification 03_nonblocking 04_semaphore_mode

.PHONY: all clean

all: $(TARGETS)

01_basic_eventfd: 01_basic_eventfd.c
	$(CC) $(CFLAGS) $< -o $@

02_thread_notification: 02_thread_notification.c
	$(CC) $(CFLAGS) $< -o $@

03_nonblocking: 03_nonblocking.c
	$(CC) $(CFLAGS) $< -o $@

04_semaphore_mode: 04_semaphore_mode.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

run: all
	@echo "=== Running eventfd Examples ==="
	@echo
	@echo "--- 01: Basic eventfd ---"
	./01_basic_eventfd
	@echo
	@echo "--- 02: Thread Notification ---"
	./02_thread_notification
	@echo
	@echo "--- 03: Non-blocking Mode ---"
	./03_nonblocking
	@echo
	@echo "--- 04: Semaphore Mode ---"
	./04_semaphore_mode
