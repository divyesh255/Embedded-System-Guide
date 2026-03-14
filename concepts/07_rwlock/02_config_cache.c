/**
 * 02_config_cache.c - Configuration Cache Pattern
 * 
 * Demonstrates using rwlock for a configuration cache.
 * Many readers (get config), rare writers (update config).
 * 
 * Compile: gcc -pthread 02_config_cache.c -o 02_config_cache
 * Run: ./02_config_cache
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 1

/* Configuration structure */
typedef struct {
    char server_url[256];
    int port;
    int timeout_ms;
    int max_connections;
} config_t;

/* Configuration cache with rwlock */
typedef struct {
    pthread_rwlock_t lock;
    config_t data;
    int version;
} config_cache_t;

config_cache_t cache = {
    .lock = PTHREAD_RWLOCK_INITIALIZER,
    .data = {
        .server_url = "http://localhost",
        .port = 8080,
        .timeout_ms = 5000,
        .max_connections = 100
    },
    .version = 1
};

/* Reader: Get configuration (frequent) */
void* reader_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < 5; i++) {
        /* Acquire read lock */
        pthread_rwlock_rdlock(&cache.lock);
        
        /* Read configuration (multiple readers can do this simultaneously!) */
        printf("[Reader %d] Config v%d: %s:%d (timeout=%dms, max_conn=%d)\n",
               thread_id, cache.version,
               cache.data.server_url, cache.data.port,
               cache.data.timeout_ms, cache.data.max_connections);
        
        /* Release lock */
        pthread_rwlock_unlock(&cache.lock);
        
        sleep(1);
    }
    
    printf("[Reader %d] Done reading\n", thread_id);
    return NULL;
}

/* Writer: Update configuration (rare) */
void* writer_thread(void* arg) {
    sleep(2);  /* Let readers start first */
    
    printf("\n[Writer] Updating configuration...\n");
    
    /* Acquire write lock (exclusive access) */
    pthread_rwlock_wrlock(&cache.lock);
    
    /* Update configuration */
    strcpy(cache.data.server_url, "http://production.example.com");
    cache.data.port = 443;
    cache.data.timeout_ms = 10000;
    cache.data.max_connections = 500;
    cache.version++;
    
    printf("[Writer] Configuration updated to v%d\n\n", cache.version);
    
    /* Release lock */
    pthread_rwlock_unlock(&cache.lock);
    
    return NULL;
}

int main(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writer;
    int reader_ids[NUM_READERS];
    
    printf("=== Configuration Cache with Rwlock ===\n\n");
    printf("Starting %d readers and %d writer\n", NUM_READERS, NUM_WRITERS);
    printf("Readers will read config frequently\n");
    printf("Writer will update config once\n\n");
    
    /* Create reader threads */
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }
    
    /* Create writer thread */
    pthread_create(&writer, NULL, writer_thread, NULL);
    
    /* Wait for all threads */
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_join(writer, NULL);
    
    printf("\n=== Final Configuration ===\n");
    printf("URL: %s\n", cache.data.server_url);
    printf("Port: %d\n", cache.data.port);
    printf("Timeout: %dms\n", cache.data.timeout_ms);
    printf("Max Connections: %d\n", cache.data.max_connections);
    printf("Version: %d\n", cache.version);
    
    printf("\n=== Why Rwlock is Perfect Here ===\n");
    printf("✅ Many readers can read config simultaneously\n");
    printf("✅ Writer gets exclusive access when updating\n");
    printf("✅ No reader sees partial/inconsistent config\n");
    printf("✅ Much faster than mutex for read-heavy workload\n");
    
    pthread_rwlock_destroy(&cache.lock);
    return 0;
}

/*
 * TYPICAL USE CASES:
 * 
 * 1. Configuration Cache (this example)
 *    - Frequent reads (get config)
 *    - Rare writes (reload config)
 * 
 * 2. DNS Cache
 *    - Frequent lookups
 *    - Occasional updates
 * 
 * 3. Route Table
 *    - Frequent route lookups
 *    - Rare route updates
 * 
 * 4. User Session Cache
 *    - Frequent session reads
 *    - Occasional session updates
 */
