/**
 * 03_pool_good.c - GOOD: Memory Pool
 *
 * Solves network gateway problem with memory pool:
 *   - No fragmentation (fixed-size blocks)
 *   - Deterministic O(1) allocation
 *   - Leak detection (track used count)
 *   - Known limits at startup
 *
 * Study time: 15 minutes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Memory pool structure */
typedef struct block {
    struct block *next;
} block_t;

typedef struct {
    void     *buffer;
    block_t  *free_list;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t used_count;
} pool_t;

/* Initialize pool */
static void pool_init(pool_t *pool, void *buffer, uint32_t block_size, uint32_t count) {
    pool->buffer = buffer;
    pool->block_size = block_size;
    pool->block_count = count;
    pool->free_list = NULL;
    pool->used_count = 0;
    
    /* Link all blocks into free list */
    for (uint32_t i = 0; i < count; i++) {
        block_t *block = (block_t*)((uint8_t*)buffer + (i * block_size));
        block->next = pool->free_list;
        pool->free_list = block;
    }
}

/* Allocate from pool */
static void* pool_alloc(pool_t *pool) {
    if (pool->free_list == NULL) {
        return NULL;  /* Pool exhausted */
    }
    
    block_t *block = pool->free_list;
    pool->free_list = block->next;
    pool->used_count++;
    
    return (void*)block;
}

/* Free to pool */
static void pool_free(pool_t *pool, void *ptr) {
    if (!ptr) return;
    
    block_t *block = (block_t*)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
    pool->used_count--;
}

/* Packet pool */
#define PACKET_SIZE  512
#define PACKET_COUNT 100
static uint8_t packet_buffer[PACKET_SIZE * PACKET_COUNT];
static pool_t packet_pool;

/* Connection pool */
#define CONN_SIZE  2048
#define CONN_COUNT 50
static uint8_t conn_buffer[CONN_SIZE * CONN_COUNT];
static pool_t conn_pool;

int main(void) {
    printf("=== GOOD: Memory Pool ===\n\n");
    
    /* Initialize pools at startup */
    pool_init(&packet_pool, packet_buffer, PACKET_SIZE, PACKET_COUNT);
    pool_init(&conn_pool, conn_buffer, CONN_SIZE, CONN_COUNT);
    
    printf("Packet pool: %u blocks × %u bytes = %u KB\n",
           PACKET_COUNT, PACKET_SIZE, (PACKET_COUNT * PACKET_SIZE) / 1024);
    printf("Conn pool:   %u blocks × %u bytes = %u KB\n",
           CONN_COUNT, CONN_SIZE, (CONN_COUNT * CONN_SIZE) / 1024);
    printf("Total:       %u KB\n\n",
           ((PACKET_COUNT * PACKET_SIZE) + (CONN_COUNT * CONN_SIZE)) / 1024);
    
    /* Allocate packets */
    void *packets[100];
    printf("--- Allocating 100 packets ---\n");
    for (int i = 0; i < 100; i++) {
        packets[i] = pool_alloc(&packet_pool);
        if (!packets[i]) {
            printf("Packet %d: FAILED (pool exhausted)\n", i);
        }
    }
    printf("Used: %u/%u\n\n", packet_pool.used_count, packet_pool.block_count);
    
    /* Allocate connections */
    void *conns[50];
    printf("--- Allocating 50 connections ---\n");
    for (int i = 0; i < 50; i++) {
        conns[i] = pool_alloc(&conn_pool);
        if (!conns[i]) {
            printf("Connection %d: FAILED (pool exhausted)\n", i);
        }
    }
    printf("Used: %u/%u\n\n", conn_pool.used_count, conn_pool.block_count);
    
    /* Free every other packet */
    printf("--- Freeing 50 packets ---\n");
    for (int i = 0; i < 100; i += 2) {
        pool_free(&packet_pool, packets[i]);
    }
    printf("Used: %u/%u (50 freed)\n\n", packet_pool.used_count, packet_pool.block_count);
    
    /* Allocate again (no fragmentation!) */
    printf("--- Allocating 50 more packets ---\n");
    for (int i = 0; i < 50; i++) {
        void *pkt = pool_alloc(&packet_pool);
        if (!pkt) {
            printf("FAILED at %d\n", i);
            break;
        }
    }
    printf("✅ SUCCESS! No fragmentation\n");
    printf("Used: %u/%u\n\n", packet_pool.used_count, packet_pool.block_count);
    
    /* Cleanup */
    for (int i = 0; i < 100; i++) {
        if (packets[i]) pool_free(&packet_pool, packets[i]);
    }
    for (int i = 0; i < 50; i++) {
        if (conns[i]) pool_free(&conn_pool, conns[i]);
    }
    
    printf("=== Results ===\n");
    printf("Packet pool used: %u/%u\n", packet_pool.used_count, packet_pool.block_count);
    printf("Conn pool used:   %u/%u\n", conn_pool.used_count, conn_pool.block_count);
    
    if (packet_pool.used_count > 0 || conn_pool.used_count > 0) {
        printf("\n❌ MEMORY LEAK DETECTED!\n");
        printf("   Packet leaks: %u\n", packet_pool.used_count);
        printf("   Conn leaks:   %u\n", conn_pool.used_count);
    } else {
        printf("\n✅ No memory leaks!\n");
    }
    
    printf("\n=== Improvements Over malloc() ===\n");
    printf("✅ No fragmentation (fixed-size blocks)\n");
    printf("✅ Deterministic O(1) allocation\n");
    printf("✅ Leak detection (track used_count)\n");
    printf("✅ Known limits at startup\n");
    printf("✅ Real-time safe\n");
    
    return 0;
}

/*
 * HOW MEMORY POOL WORKS:
 *
 * 1. Initialization:
 *    - Pre-allocate buffer at startup
 *    - Link all blocks into free list
 *    - Know exact memory usage
 *
 * 2. Allocation:
 *    - Remove block from free list head
 *    - O(1) time, deterministic
 *    - Fails immediately if pool exhausted
 *
 * 3. Deallocation:
 *    - Add block back to free list head
 *    - O(1) time, deterministic
 *    - No fragmentation
 *
 * 4. Leak Detection:
 *    - Track used_count
 *    - At shutdown: used_count should be 0
 *    - Easy to find leaks
 */
