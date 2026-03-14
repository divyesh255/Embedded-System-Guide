# Makefile for Signal Handling examples

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
PTHREAD_FLAGS = -pthread
TARGETS = 01_basic_signal 02_sigaction 03_signal_eventfd 04_timer_signal

.PHONY: all clean

all: $(TARGETS)

01_basic_signal: 01_basic_signal.c
	$(CC) $(CFLAGS) $< -o $@

02_sigaction: 02_sigaction.c
	$(CC) $(CFLAGS) $< -o $@

03_signal_eventfd: 03_signal_eventfd.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) $< -o $@

04_timer_signal: 04_timer_signal.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

run: all
	@echo "=== Running Signal Handling Examples ==="
	@echo
	@echo "--- 01: Basic Signal Handling ---"
	@echo "Press Ctrl+C 3 times to exit"
	./01_basic_signal
	@echo
	@echo "--- 02: sigaction() ---"
	@echo "Press Ctrl+C to exit"
	./02_sigaction
	@echo
	@echo "--- 03: Signal with eventfd ---"
	@echo "Press Ctrl+C for graceful shutdown"
	./03_signal_eventfd
	@echo
	@echo "--- 04: Timer with SIGALRM ---"
	./04_timer_signal
