/**
 * 03_writer_starvation.c - Writer Starvation Demo
 * 
 * Demonstrates writer starvation when readers keep coming.
 * 
 * Compile: gcc -pthread 03_writer_starvation.c -o 03_writer_starvation
 * Run: ./03_writer_starvation
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
int shared_data = 0;
int writer_waiting = 0;

void* continuous_reader(void* arg) {
    int id = *(int*)arg;
    
    for (int i = 0; i < 20; i++) {
        pthread_rwlock_rdlock(&rwlock);
        printf("[Reader %d] Read: %d\n", id, shared_data);
        usleep(100000);  /* 100ms */
        pthread_rwlock_unlock(&rwlock);
        
        usleep(50000);  /* Short gap */
    }
    
    return NULL;
}

void* starving_writer(void* arg) {
    sleep(1);  /* Let readers start */
    
    printf("\n[Writer] Waiting for write lock...\n");
    writer_waiting = 1;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    pthread_rwlock_wrlock(&rwlock);  /* May wait a long time! */
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double wait_time = (end.tv_sec - start.tv_sec) + 
                      (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("[Writer] Got write lock after %.2f seconds!\n", wait_time);
    shared_data++;
    printf("[Writer] Updated data to %d\n", shared_data);
    
    pthread_rwlock_unlock(&rwlock);
    writer_waiting = 0;
    
    return NULL;
}

int main(void) {
    pthread_t readers[5];
    pthread_t writer;
    int reader_ids[5];
    
    printf("=== Writer Starvation Demo ===\n\n");
    printf("5 readers continuously reading\n");
    printf("1 writer trying to write\n");
    printf("Watch how long writer waits!\n\n");
    
    /* Create continuous readers */
    for (int i = 0; i < 5; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, continuous_reader, &reader_ids[i]);
    }
    
    /* Create writer */
    pthread_create(&writer, NULL, starving_writer, NULL);
    
    /* Wait for all */
    pthread_join(writer, NULL);
    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }
    
    printf("\n=== Writer Starvation Explained ===\n");
    printf("Problem: Readers keep coming, writer never gets lock!\n");
    printf("- Reader 1 holds lock\n");
    printf("- Reader 2 joins (allowed, multiple readers OK)\n");
    printf("- Reader 1 releases, but Reader 2 still holds\n");
    printf("- Reader 3 joins before writer can get lock\n");
    printf("- Writer keeps waiting...\n\n");
    
    printf("=== Solutions ===\n");
    printf("1. Writer-preferred rwlock (some implementations)\n");
    printf("2. Limit reader hold time\n");
    printf("3. Use timeouts (pthread_rwlock_timedwrlock)\n");
    printf("4. Fairness policies\n");
    
    pthread_rwlock_destroy(&rwlock);
    return 0;
}

/*
 * WRITER STARVATION:
 * 
 * Time | Reader 1 | Reader 2 | Reader 3 | Writer | Lock State
 * -----|----------|----------|----------|--------|------------
 * t0   | rdlock() |          |          |        | R=1
 * t1   | reading  | rdlock() |          |        | R=2
 * t2   | unlock() | reading  |          | wrlock | R=1, W waiting
 * t3   |          | reading  | rdlock() | waiting| R=2, W waiting
 * t4   |          | unlock() | reading  | waiting| R=1, W waiting
 * t5   | rdlock() |          | reading  | waiting| R=2, W waiting
 * ... (writer keeps waiting!)
 * 
 * Writer may wait indefinitely if readers keep coming!
 */
