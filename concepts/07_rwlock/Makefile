# Makefile for Read-Write Lock examples

CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c11 -D_XOPEN_SOURCE=700
TARGETS = 01_mutex_vs_rwlock 02_config_cache 03_writer_starvation 04_lookup_table

.PHONY: all clean

all: $(TARGETS)

01_mutex_vs_rwlock: 01_mutex_vs_rwlock.c
	$(CC) $(CFLAGS) $< -o $@

02_config_cache: 02_config_cache.c
	$(CC) $(CFLAGS) $< -o $@

03_writer_starvation: 03_writer_starvation.c
	$(CC) $(CFLAGS) $< -o $@

04_lookup_table: 04_lookup_table.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

run: all
	@echo "=== Running Read-Write Lock Examples ==="
	@echo
	@echo "--- 01: Mutex vs Rwlock Performance ---"
	./01_mutex_vs_rwlock
	@echo
	@echo "--- 02: Configuration Cache Pattern ---"
	./02_config_cache
	@echo
	@echo "--- 03: Writer Starvation Demo ---"
	./03_writer_starvation
	@echo
	@echo "--- 04: Lookup Table with Rwlock ---"
	./04_lookup_table
