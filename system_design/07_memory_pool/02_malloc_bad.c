/**
 * 02_malloc_bad.c - BAD: Using malloc/free
 *
 * Simulates network gateway using malloc/free.
 * Demonstrates:
 *   - Memory fragmentation
 *   - Non-deterministic timing
 *   - Memory leaks
 *   - Allocation failures
 *
 * Study time: 10 minutes
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Simulated memory tracking */
static uint32_t total_allocated = 0;
static uint32_t total_freed = 0;
static uint32_t alloc_count = 0;
static uint32_t free_count = 0;
static uint32_t failed_allocs = 0;

/* Simulate malloc with tracking */
static void* tracked_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        total_allocated += size;
        alloc_count++;
    } else {
        failed_allocs++;
    }
    return ptr;
}

/* Simulate free with tracking */
static void tracked_free(void *ptr, size_t size) {
    if (ptr) {
        free(ptr);
        total_freed += size;
        free_count++;
    }
}

/* Packet structure */
typedef struct {
    uint8_t *data;
    uint16_t size;
} packet_t;

/* TCP connection structure */
typedef struct {
    int socket_fd;
    uint8_t *rx_buffer;
    uint8_t *tx_buffer;
} tcp_conn_t;

/* Allocate packet */
static packet_t* packet_alloc(uint16_t size) {
    packet_t *pkt = tracked_malloc(sizeof(packet_t));
    if (!pkt) return NULL;
    
    pkt->data = tracked_malloc(size);
    if (!pkt->data) {
        tracked_free(pkt, sizeof(packet_t));
        return NULL;
    }
    
    pkt->size = size;
    return pkt;
}

/* Free packet */
static void packet_free(packet_t *pkt) {
    if (pkt) {
        tracked_free(pkt->data, pkt->size);
        tracked_free(pkt, sizeof(packet_t));
    }
}

/* Allocate TCP connection */
static tcp_conn_t* tcp_conn_alloc(void) {
    tcp_conn_t *conn = tracked_malloc(sizeof(tcp_conn_t));
    if (!conn) return NULL;
    
    conn->rx_buffer = tracked_malloc(1024);
    conn->tx_buffer = tracked_malloc(1024);
    
    if (!conn->rx_buffer || !conn->tx_buffer) {
        tracked_free(conn->rx_buffer, 1024);
        tracked_free(conn->tx_buffer, 1024);
        tracked_free(conn, sizeof(tcp_conn_t));
        return NULL;
    }
    
    return conn;
}

/* Free TCP connection */
static void tcp_conn_free(tcp_conn_t *conn) {
    if (conn) {
        tracked_free(conn->rx_buffer, 1024);
        tracked_free(conn->tx_buffer, 1024);
        tracked_free(conn, sizeof(tcp_conn_t));
    }
}

int main(void) {
    printf("=== BAD: Using malloc/free ===\n\n");
    printf("Simulating network gateway with malloc/free\n\n");

    /* Simulate workload */
    packet_t *packets[100] = {0};
    tcp_conn_t *conns[50] = {0};

    printf("--- Allocating resources ---\n");

    /* Allocate packets (variable sizes) */
    for (int i = 0; i < 100; i++) {
        uint16_t size = 64 + (i * 14);  /* Variable: 64-1450 bytes */
        packets[i] = packet_alloc(size);
        if (!packets[i]) {
            printf("Packet alloc %d FAILED\n", i);
        }
    }

    /* Allocate connections */
    for (int i = 0; i < 50; i++) {
        conns[i] = tcp_conn_alloc();
        if (!conns[i]) {
            printf("Connection alloc %d FAILED\n", i);
        }
    }

    printf("\n--- Simulating fragmentation ---\n");
    
    /* Free every other packet (creates fragmentation) */
    for (int i = 0; i < 100; i += 2) {
        packet_free(packets[i]);
        packets[i] = NULL;
    }

    printf("Freed 50 packets (every other one)\n");
    printf("Memory now fragmented!\n\n");

    /* Try to allocate large packet (will likely fail due to fragmentation) */
    printf("--- Trying to allocate 1400-byte packet ---\n");
    packet_t *large_pkt = packet_alloc(1400);
    if (!large_pkt) {
        printf("❌ FAILED! Fragmentation prevents large allocation\n");
        printf("   Even though %u bytes were freed\n", 
               (total_freed - total_allocated + total_freed));
    }

    /* Simulate memory leak (forget to free) */
    printf("\n--- Simulating memory leak ---\n");
    packet_t *leaked = packet_alloc(512);
    printf("Allocated packet but forgot to free it!\n");

    /* Cleanup */
    for (int i = 0; i < 100; i++) {
        if (packets[i]) packet_free(packets[i]);
    }
    for (int i = 0; i < 50; i++) {
        if (conns[i]) tcp_conn_free(conns[i]);
    }

    printf("\n=== Results ===\n");
    printf("Total allocated: %u bytes\n", total_allocated);
    printf("Total freed:     %u bytes\n", total_freed);
    printf("Leaked:          %u bytes\n", total_allocated - total_freed);
    printf("Alloc count:     %u\n", alloc_count);
    printf("Free count:      %u\n", free_count);
    printf("Failed allocs:   %u\n", failed_allocs);

    printf("\n=== Problems ===\n");
    printf("❌ Fragmentation causes allocation failures\n");
    printf("❌ Non-deterministic timing (malloc searches heap)\n");
    printf("❌ Memory leaks hard to detect\n");
    printf("❌ No way to know limits at startup\n");

    printf("\n=== The Fix ===\n");
    printf("See 03_pool_good.c — memory pool solution\n");

    return 0;
}

/*
 * PROBLEMS WITH MALLOC/FREE:
 *
 * 1. ❌ Fragmentation
 *    Variable-size allocations create gaps
 *    Large allocations fail even with free memory
 *
 * 2. ❌ Non-deterministic
 *    malloc() time: 5µs to 2ms (depends on heap state)
 *    Not suitable for real-time systems
 *
 * 3. ❌ Memory leaks
 *    Easy to forget free() in error paths
 *    Hard to detect until crash
 *
 * 4. ❌ Unknown limits
 *    Don't know max allocations until runtime
 *    Fails unpredictably
 */
