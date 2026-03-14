# eventfd - Practice Exercises

Test your understanding of eventfd with these hands-on exercises!

---

## 🎯 Exercise 1: Basic Producer-Consumer (Easy - 15 min)

Create a producer-consumer using eventfd for notification.

**Task:** Producer adds items to queue and signals via eventfd. Consumer waits on eventfd.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>

#define QUEUE_SIZE 10

int queue[QUEUE_SIZE];
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int efd;

void* producer(void* arg) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&mutex);
        queue[count++] = i;
        pthread_mutex_unlock(&mutex);
        
        uint64_t val = 1;
        write(efd, &val, sizeof(val));
        printf("[Producer] Added item %d\n", i);
        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < 5; i++) {
        uint64_t val;
        read(efd, &val, sizeof(val));  // Wait for event
        
        pthread_mutex_lock(&mutex);
        int item = queue[--count];
        pthread_mutex_unlock(&mutex);
        
        printf("[Consumer] Got item %d\n", item);
    }
    return NULL;
}

int main(void) {
    efd = eventfd(0, 0);
    
    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    
    close(efd);
    return 0;
}
```

**Key points:**
- eventfd for notification
- Mutex for queue protection
- Clean separation of concerns
</details>

---

## 🎯 Exercise 2: Timeout with eventfd (Medium - 20 min)

Implement a timed wait using poll() with eventfd.

**Task:** Wait for event with 5-second timeout.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <poll.h>
#include <pthread.h>

int efd;

void* delayed_signal(void* arg) {
    int delay = *(int*)arg;
    sleep(delay);
    
    uint64_t val = 1;
    write(efd, &val, sizeof(val));
    printf("[Signal] Event sent after %d seconds\n", delay);
    return NULL;
}

int main(void) {
    efd = eventfd(0, 0);
    
    int delay = 3;  // Try changing to 7 to see timeout
    pthread_t thread;
    pthread_create(&thread, NULL, delayed_signal, &delay);
    
    struct pollfd pfd = {
        .fd = efd,
        .events = POLLIN
    };
    
    printf("Waiting for event (5 second timeout)...\n");
    int ret = poll(&pfd, 1, 5000);  // 5 second timeout
    
    if (ret > 0) {
        uint64_t val;
        read(efd, &val, sizeof(val));
        printf("Event received!\n");
    } else if (ret == 0) {
        printf("Timeout! No event received\n");
    } else {
        perror("poll");
    }
    
    pthread_join(thread, NULL);
    close(efd);
    return 0;
}
```

**Key concepts:**
- poll() with eventfd
- Timeout handling
- Non-blocking event waiting
</details>

---

## 🎯 Exercise 3: Multiple Event Sources (Hard - 30 min)

Use multiple eventfds with epoll for different event types.

**Task:** Monitor 3 different event sources simultaneously.

<details>
<summary>Solution</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENTS 10

int efd_data, efd_control, efd_error;

void* data_producer(void* arg) {
    for (int i = 0; i < 3; i++) {
        sleep(1);
        write(efd_data, &(uint64_t){1}, sizeof(uint64_t));
    }
    return NULL;
}

void* control_producer(void* arg) {
    sleep(2);
    write(efd_control, &(uint64_t){1}, sizeof(uint64_t));
    return NULL;
}

int main(void) {
    efd_data = eventfd(0, EFD_NONBLOCK);
    efd_control = eventfd(0, EFD_NONBLOCK);
    efd_error = eventfd(0, EFD_NONBLOCK);
    
    int epollfd = epoll_create1(0);
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    
    ev.data.fd = efd_data;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, efd_data, &ev);
    
    ev.data.fd = efd_control;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, efd_control, &ev);
    
    ev.data.fd = efd_error;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, efd_error, &ev);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, data_producer, NULL);
    pthread_create(&t2, NULL, control_producer, NULL);
    
    printf("Monitoring events...\n");
    struct epoll_event events[MAX_EVENTS];
    int event_count = 0;
    
    while (event_count < 4) {
        int n = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < n; i++) {
            uint64_t val;
            read(events[i].data.fd, &val, sizeof(val));
            
            if (events[i].data.fd == efd_data) {
                printf("Data event received\n");
            } else if (events[i].data.fd == efd_control) {
                printf("Control event received\n");
            } else if (events[i].data.fd == efd_error) {
                printf("Error event received\n");
            }
            event_count++;
        }
    }
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    close(efd_data);
    close(efd_control);
    close(efd_error);
    close(epollfd);
    
    return 0;
}
```

**Advanced concepts:**
- Multiple eventfds
- epoll for efficient monitoring
- Event type discrimination
</details>

---

## 🎯 Exercise 4: Resource Pool with EFD_SEMAPHORE (Hard - 25 min)

Implement a thread pool using eventfd in semaphore mode.

**Task:** N workers, eventfd tracks available workers.

<details>
<summary>Hint</summary>

- Use EFD_SEMAPHORE flag
- write(N) to initialize pool
- read() to claim worker (blocks if none available)
- write(1) to release worker
</details>

---

## 🎯 Exercise 5: Signal-Safe Notification (Advanced - 30 min)

Use eventfd to safely notify from signal handler.

**Task:** Signal handler writes to eventfd, main loop reads.

<details>
<summary>Hint</summary>

- Use EFD_NONBLOCK in signal handler
- Signal handler only does write()
- Main loop uses poll/epoll to wait
- This is signal-safe!
</details>

---

## 🎓 Key Takeaways

After completing these exercises, you should understand:

1. **eventfd Basics**
   - Create with eventfd()
   - write() to signal
   - read() to wait

2. **Flags**
   - EFD_NONBLOCK - non-blocking mode
   - EFD_SEMAPHORE - semaphore behavior
   - EFD_CLOEXEC - close on exec

3. **Integration**
   - Works with poll/epoll/select
   - Perfect for event loops
   - Signal-safe notification

4. **Use Cases**
   - Thread notification
   - Event counting
   - Resource pools
   - Async I/O frameworks

5. **Advantages**
   - Lightweight (single fd)
   - Accumulates events
   - File descriptor benefits
   - Linux-specific but powerful

---

**Congratulations!** You've mastered eventfd! 🎉
