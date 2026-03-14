/**
 * 04_lookup_table.c - Lookup Table with Rwlock
 * 
 * Simple hash table protected by rwlock.
 * Frequent lookups, rare inserts/deletes.
 * 
 * Compile: gcc -pthread 04_lookup_table.c -o 04_lookup_table
 * Run: ./04_lookup_table
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define TABLE_SIZE 10
#define NUM_READERS 4
#define NUM_WRITERS 1

/* Simple hash table entry */
typedef struct entry {
    char *key;
    int value;
    struct entry *next;
} entry_t;

/* Hash table with rwlock */
typedef struct {
    pthread_rwlock_t lock;
    entry_t *buckets[TABLE_SIZE];
} hash_table_t;

hash_table_t table = {
    .lock = PTHREAD_RWLOCK_INITIALIZER,
    .buckets = {NULL}
};

/* Simple hash function */
unsigned int hash(const char *key) {
    unsigned int h = 0;
    while (*key) {
        h = h * 31 + *key++;
    }
    return h % TABLE_SIZE;
}

/* Lookup (read operation) */
int lookup(const char *key) {
    pthread_rwlock_rdlock(&table.lock);
    
    unsigned int idx = hash(key);
    entry_t *e = table.buckets[idx];
    
    while (e) {
        if (strcmp(e->key, key) == 0) {
            int value = e->value;
            pthread_rwlock_unlock(&table.lock);
            return value;
        }
        e = e->next;
    }
    
    pthread_rwlock_unlock(&table.lock);
    return -1;  /* Not found */
}

/* Insert (write operation) */
void insert(const char *key, int value) {
    pthread_rwlock_wrlock(&table.lock);
    
    unsigned int idx = hash(key);
    
    /* Check if key exists */
    entry_t *e = table.buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;  /* Update */
            pthread_rwlock_unlock(&table.lock);
            return;
        }
        e = e->next;
    }
    
    /* Insert new entry */
    entry_t *new_entry = malloc(sizeof(entry_t));
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = table.buckets[idx];
    table.buckets[idx] = new_entry;
    
    pthread_rwlock_unlock(&table.lock);
}

void* reader_thread(void* arg) {
    int id = *(int*)arg;
    const char *keys[] = {"user1", "user2", "user3", "user4", "user5"};
    
    for (int i = 0; i < 10; i++) {
        const char *key = keys[i % 5];
        int value = lookup(key);
        printf("[Reader %d] lookup('%s') = %d\n", id, key, value);
        usleep(100000);  /* 100ms */
    }
    
    return NULL;
}

void* writer_thread(void* arg) {
    const char *keys[] = {"user1", "user2", "user3", "user4", "user5"};
    
    for (int i = 0; i < 5; i++) {
        insert(keys[i], i * 100);
        printf("[Writer] insert('%s', %d)\n", keys[i], i * 100);
        sleep(1);
    }
    
    return NULL;
}

int main(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writer;
    int reader_ids[NUM_READERS];
    
    printf("=== Hash Table with Rwlock ===\n\n");
    printf("Starting %d readers and %d writer\n", NUM_READERS, NUM_WRITERS);
    printf("Readers: frequent lookups\n");
    printf("Writer: occasional inserts\n\n");
    
    /* Create writer first to populate table */
    pthread_create(&writer, NULL, writer_thread, NULL);
    sleep(1);  /* Let writer insert some data */
    
    /* Create readers */
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader_thread, &reader_ids[i]);
    }
    
    /* Wait for all */
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    
    printf("\n=== Final Table Contents ===\n");
    pthread_rwlock_rdlock(&table.lock);
    for (int i = 0; i < TABLE_SIZE; i++) {
        entry_t *e = table.buckets[i];
        while (e) {
            printf("'%s' => %d\n", e->key, e->value);
            e = e->next;
        }
    }
    pthread_rwlock_unlock(&table.lock);
    
    printf("\n=== Why Rwlock Works Well ===\n");
    printf("✅ Multiple readers can lookup simultaneously\n");
    printf("✅ Writer gets exclusive access for inserts\n");
    printf("✅ No reader sees inconsistent table state\n");
    printf("✅ Perfect for read-heavy workloads\n");
    
    /* Cleanup */
    pthread_rwlock_destroy(&table.lock);
    for (int i = 0; i < TABLE_SIZE; i++) {
        entry_t *e = table.buckets[i];
        while (e) {
            entry_t *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    
    return 0;
}

/*
 * REAL-WORLD EXAMPLES:
 * 
 * 1. DNS Cache
 *    - Frequent: DNS lookups
 *    - Rare: Cache updates
 * 
 * 2. User Session Store
 *    - Frequent: Session validation
 *    - Rare: Session creation/deletion
 * 
 * 3. Configuration Registry
 *    - Frequent: Config reads
 *    - Rare: Config updates
 * 
 * 4. Symbol Table (compiler)
 *    - Frequent: Symbol lookups
 *    - Rare: Symbol definitions
 */
