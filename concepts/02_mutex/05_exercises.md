# Mutex Exercises

Practice problems with proper error checking and best practices.

## Easy Exercises (15 minutes each)

### Exercise 1: Protected Counter
**Task:** Create 3 threads that each increment a shared counter 10000 times using a mutex.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

int counter = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *increment(void *arg) {
    (void)arg;
    for (int i = 0; i < 10000; i++) {
        if (pthread_mutex_lock(&lock) != 0) {
            perror("lock failed");
            return NULL;
        }
        counter++;
        if (pthread_mutex_unlock(&lock) != 0) {
            perror("unlock failed");
            return NULL;
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[3];
    
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, increment, NULL) != 0) {
            perror("create failed");
            return 1;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Counter: %d (expected 30000)\n", counter);
    pthread_mutex_destroy(&lock);
    return 0;
}
```
</details>

### Exercise 2: Bank Account
**Task:** Create a bank account with deposit/withdraw functions protected by mutex.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

typedef struct {
    int balance;
    pthread_mutex_t lock;
} BankAccount;

void account_init(BankAccount *acc) {
    acc->balance = 0;
    pthread_mutex_init(&acc->lock, NULL);
}

int deposit(BankAccount *acc, int amount) {
    pthread_mutex_lock(&acc->lock);
    acc->balance += amount;
    pthread_mutex_unlock(&acc->lock);
    return acc->balance;
}

int withdraw(BankAccount *acc, int amount) {
    pthread_mutex_lock(&acc->lock);
    if (acc->balance >= amount) {
        acc->balance -= amount;
        pthread_mutex_unlock(&acc->lock);
        return 1;
    }
    pthread_mutex_unlock(&acc->lock);
    return 0;
}

int main() {
    BankAccount acc;
    account_init(&acc);
    
    deposit(&acc, 1000);
    printf("Balance: %d\n", acc.balance);
    
    if (withdraw(&acc, 500)) {
        printf("Withdrew 500, balance: %d\n", acc.balance);
    }
    
    pthread_mutex_destroy(&acc.lock);
    return 0;
}
```
</details>

## Medium Exercises (30 minutes each)

### Exercise 3: Thread-Safe Queue
**Task:** Implement a simple thread-safe queue with enqueue/dequeue using mutex.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define QUEUE_SIZE 10

typedef struct {
    int data[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
} Queue;

void queue_init(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
}

int enqueue(Queue *q, int value) {
    pthread_mutex_lock(&q->lock);
    
    if (q->count >= QUEUE_SIZE) {
        pthread_mutex_unlock(&q->lock);
        return 0;  /* Queue full */
    }
    
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
    
    pthread_mutex_unlock(&q->lock);
    return 1;
}

int dequeue(Queue *q, int *value) {
    pthread_mutex_lock(&q->lock);
    
    if (q->count == 0) {
        pthread_mutex_unlock(&q->lock);
        return 0;  /* Queue empty */
    }
    
    *value = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    
    pthread_mutex_unlock(&q->lock);
    return 1;
}

int main() {
    Queue q;
    queue_init(&q);
    
    enqueue(&q, 10);
    enqueue(&q, 20);
    enqueue(&q, 30);
    
    int val;
    while (dequeue(&q, &val)) {
        printf("Dequeued: %d\n", val);
    }
    
    pthread_mutex_destroy(&q.lock);
    return 0;
}
```
</details>

### Exercise 4: Reader Count
**Task:** Track how many threads are currently reading a shared resource.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int reader_count = 0;
pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;

void *reader(void *arg) {
    int id = *(int *)arg;
    
    /* Enter */
    pthread_mutex_lock(&count_lock);
    reader_count++;
    printf("[Reader %d] Entered, total readers: %d\n", id, reader_count);
    pthread_mutex_unlock(&count_lock);
    
    /* Read */
    sleep(2);
    
    /* Exit */
    pthread_mutex_lock(&count_lock);
    reader_count--;
    printf("[Reader %d] Exited, total readers: %d\n", id, reader_count);
    pthread_mutex_unlock(&count_lock);
    
    return NULL;
}

int main() {
    pthread_t threads[5];
    int ids[5] = {1, 2, 3, 4, 5};
    
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, reader, &ids[i]);
        usleep(100000);  /* Stagger starts */
    }
    
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&count_lock);
    return 0;
}
```
</details>

## Hard Exercise (45 minutes)

### Exercise 5: Deadlock-Free Transfer
**Task:** Implement money transfer between accounts without deadlock.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <pthread.h>

typedef struct {
    int id;
    int balance;
    pthread_mutex_t lock;
} Account;

void account_init(Account *acc, int id, int balance) {
    acc->id = id;
    acc->balance = balance;
    pthread_mutex_init(&acc->lock, NULL);
}

/* Lock in consistent order to avoid deadlock */
void transfer(Account *from, Account *to, int amount) {
    Account *first, *second;
    
    /* Always lock lower ID first */
    if (from->id < to->id) {
        first = from;
        second = to;
    } else {
        first = to;
        second = from;
    }
    
    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);
    
    if (from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
        printf("Transferred %d from Account %d to Account %d\n",
               amount, from->id, to->id);
    }
    
    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

int main() {
    Account acc1, acc2;
    account_init(&acc1, 1, 1000);
    account_init(&acc2, 2, 500);
    
    transfer(&acc1, &acc2, 200);
    transfer(&acc2, &acc1, 100);
    
    printf("Account 1: %d\n", acc1.balance);
    printf("Account 2: %d\n", acc2.balance);
    
    pthread_mutex_destroy(&acc1.lock);
    pthread_mutex_destroy(&acc2.lock);
    return 0;
}
```
</details>

---

**Key Takeaways:**
- ✅ Always protect shared data with mutex
- ✅ Keep critical sections small
- ✅ Lock in consistent order
- ✅ Always unlock (even on errors)
- ✅ Check all return values

**Next Module:** 03_condition_variables
