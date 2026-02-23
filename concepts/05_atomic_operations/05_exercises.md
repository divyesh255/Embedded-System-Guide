# Atomic Operations Exercises

Practice lock-free programming!

## Easy Exercises (20 minutes each)

### Exercise 1: Atomic Flag
**Task:** Use atomic_bool as a ready flag between threads.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

atomic_bool ready = ATOMIC_VAR_INIT(false);

void *waiter(void *arg) {
    (void)arg;
    
    printf("Waiting...\n");
    while (!atomic_load(&ready)) {
        /* Spin */
    }
    printf("Ready!\n");
    
    return NULL;
}

void *signaler(void *arg) {
    (void)arg;
    
    sleep(2);
    printf("Signaling...\n");
    atomic_store(&ready, true);
    
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    pthread_create(&t1, NULL, waiter, NULL);
    pthread_create(&t2, NULL, signaler, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    return 0;
}
```
</details>

### Exercise 2: Atomic Max
**Task:** Find maximum value atomically across threads.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int max_value = ATOMIC_VAR_INIT(0);

void atomic_max(atomic_int *var, int value) {
    int old = atomic_load(var);
    while (value > old) {
        if (atomic_compare_exchange_weak(var, &old, value)) {
            break;
        }
    }
}

void *worker(void *arg) {
    int value = *(int *)arg;
    atomic_max(&max_value, value);
    return NULL;
}

int main() {
    pthread_t threads[5];
    int values[5] = {10, 50, 30, 70, 20};
    
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, worker, &values[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Max: %d\n", atomic_load(&max_value));
    return 0;
}
```
</details>

## Medium Exercises (30 minutes each)

### Exercise 3: Lock-Free Stack (Push Only)
**Task:** Implement lock-free stack push using CAS.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct {
    _Atomic(Node *) head;
} Stack;

void stack_init(Stack *s) {
    atomic_init(&s->head, NULL);
}

void stack_push(Stack *s, int data) {
    Node *node = malloc(sizeof(Node));
    node->data = data;
    
    Node *old_head = atomic_load(&s->head);
    do {
        node->next = old_head;
    } while (!atomic_compare_exchange_weak(&s->head, &old_head, node));
}

void stack_print(Stack *s) {
    Node *curr = atomic_load(&s->head);
    while (curr) {
        printf("%d ", curr->data);
        curr = curr->next;
    }
    printf("\n");
}

void *pusher(void *arg) {
    Stack *s = (Stack *)arg;
    for (int i = 0; i < 5; i++) {
        stack_push(s, i);
    }
    return NULL;
}

int main() {
    Stack s;
    stack_init(&s);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, pusher, &s);
    pthread_create(&t2, NULL, pusher, &s);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("Stack: ");
    stack_print(&s);
    
    return 0;
}
```
</details>

### Exercise 4: Atomic Statistics
**Task:** Track min, max, sum atomically.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <limits.h>

typedef struct {
    atomic_int min;
    atomic_int max;
    atomic_long sum;
    atomic_int count;
} Stats;

void stats_init(Stats *s) {
    atomic_init(&s->min, INT_MAX);
    atomic_init(&s->max, INT_MIN);
    atomic_init(&s->sum, 0);
    atomic_init(&s->count, 0);
}

void stats_add(Stats *s, int value) {
    /* Update min */
    int old_min = atomic_load(&s->min);
    while (value < old_min) {
        if (atomic_compare_exchange_weak(&s->min, &old_min, value)) {
            break;
        }
    }
    
    /* Update max */
    int old_max = atomic_load(&s->max);
    while (value > old_max) {
        if (atomic_compare_exchange_weak(&s->max, &old_max, value)) {
            break;
        }
    }
    
    /* Update sum and count */
    atomic_fetch_add(&s->sum, value);
    atomic_fetch_add(&s->count, 1);
}

void *worker(void *arg) {
    Stats *s = (Stats *)arg;
    for (int i = 1; i <= 100; i++) {
        stats_add(s, i);
    }
    return NULL;
}

int main() {
    Stats s;
    stats_init(&s);
    
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, worker, &s);
    }
    
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Min: %d\n", atomic_load(&s.min));
    printf("Max: %d\n", atomic_load(&s.max));
    printf("Sum: %ld\n", atomic_load(&s.sum));
    printf("Count: %d\n", atomic_load(&s.count));
    printf("Avg: %.2f\n", (double)atomic_load(&s.sum) / atomic_load(&s.count));
    
    return 0;
}
```
</details>

## Hard Exercise (45 minutes)

### Exercise 5: Lock-Free Queue (MPSC)
**Task:** Multi-producer, single-consumer queue.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

typedef struct Node {
    int data;
    _Atomic(struct Node *) next;
} Node;

typedef struct {
    _Atomic(Node *) head;
    Node *tail;
} Queue;

void queue_init(Queue *q) {
    Node *dummy = malloc(sizeof(Node));
    atomic_init(&dummy->next, NULL);
    atomic_init(&q->head, dummy);
    q->tail = dummy;
}

void enqueue(Queue *q, int data) {
    Node *node = malloc(sizeof(Node));
    node->data = data;
    atomic_init(&node->next, NULL);
    
    Node *old_head = atomic_load(&q->head);
    do {
        atomic_store(&node->next, old_head);
    } while (!atomic_compare_exchange_weak(&q->head, &old_head, node));
}

int dequeue(Queue *q, int *data) {
    if (q->tail == atomic_load(&q->head)) {
        return 0;  /* Empty */
    }
    
    Node *next = atomic_load(&q->tail->next);
    if (next) {
        *data = next->data;
        q->tail = next;
        return 1;
    }
    return 0;
}

void *producer(void *arg) {
    Queue *q = (Queue *)arg;
    for (int i = 0; i < 10; i++) {
        enqueue(q, i);
        printf("[Producer] Enqueued %d\n", i);
        usleep(100000);
    }
    return NULL;
}

int main() {
    Queue q;
    queue_init(&q);
    
    pthread_t p1, p2;
    pthread_create(&p1, NULL, producer, &q);
    pthread_create(&p2, NULL, producer, &q);
    
    sleep(1);
    
    int data;
    while (dequeue(&q, &data)) {
        printf("[Consumer] Dequeued %d\n", data);
    }
    
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    
    return 0;
}
```
</details>

---

**Key Takeaways:**
- ✅ Use atomic operations for simple variables
- ✅ CAS is foundation of lock-free algorithms
- ✅ Test with ThreadSanitizer
- ✅ Start with seq_cst memory order
- ✅ Keep lock-free code simple

**Completed!** You've mastered 5 synchronization modules!
