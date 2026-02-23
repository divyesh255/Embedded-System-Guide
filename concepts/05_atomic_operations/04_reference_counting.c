/**
 * 04_reference_counting.c - Atomic Reference Counting
 * 
 * Practical example: thread-safe reference counting.
 * 
 * Compile: gcc -std=c11 -pthread -o 04_reference_counting 04_reference_counting.c
 * Run: ./04_reference_counting
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

typedef struct {
    int data;
    atomic_int refcount;
} Resource;

Resource *resource_create(int data) {
    Resource *r = malloc(sizeof(Resource));
    r->data = data;
    atomic_init(&r->refcount, 1);
    printf("Resource created (data=%d, refcount=1)\n", data);
    return r;
}

void resource_acquire(Resource *r) {
    atomic_fetch_add(&r->refcount, 1);
    int count = atomic_load(&r->refcount);
    printf("Acquired resource (refcount=%d)\n", count);
}

void resource_release(Resource *r) {
    int old = atomic_fetch_sub(&r->refcount, 1);
    printf("Released resource (refcount=%d)\n", old - 1);
    
    if (old == 1) {
        printf("Last reference! Freeing resource (data=%d)\n", r->data);
        free(r);
    }
}

void *worker(void *arg) {
    Resource *r = (Resource *)arg;
    
    resource_acquire(r);
    printf("[Worker] Using resource (data=%d)\n", r->data);
    sleep(1);
    resource_release(r);
    
    return NULL;
}

int main(void) {
    printf("=== Reference Counting Demo ===\n\n");
    
    Resource *r = resource_create(42);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, r);
    pthread_create(&t2, NULL, worker, r);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    resource_release(r);  /* Release initial reference */
    
    printf("\nResource freed automatically!\n");
    return 0;
}

/* Atomic refcounting prevents use-after-free bugs */
