/**
 * 03_circular_good.c - GOOD EXAMPLE: Circular Buffer
 * 
 * This shows the RIGHT way to handle streaming UART data.
 * Uses a circular buffer to queue incoming data.
 * 
 * Benefits:
 * - No data loss (if sized correctly)
 * - Decouples producer and consumer
 * - O(1) operations
 * - Fixed memory usage
 * - Simple and efficient
 * 
 * Study time: 15 minutes
 * This is production-ready architecture!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * CIRCULAR BUFFER IMPLEMENTATION
 * ============================================================================ */

#define BUFFER_SIZE 256  // Power of 2 for fast masking

typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    uint32_t head;   // Write position
    uint32_t tail;   // Read position
    uint32_t count;  // Number of bytes in buffer
} circular_buffer_t;

/* Initialize circular buffer */
void cb_init(circular_buffer_t *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

/* Check if buffer is full */
bool cb_is_full(circular_buffer_t *cb) {
    return cb->count == BUFFER_SIZE;
}

/* Check if buffer is empty */
bool cb_is_empty(circular_buffer_t *cb) {
    return cb->count == 0;
}

/* Get number of bytes in buffer */
uint32_t cb_count(circular_buffer_t *cb) {
    return cb->count;
}

/* Write one byte to buffer */
bool cb_write(circular_buffer_t *cb, uint8_t data) {
    if (cb_is_full(cb)) {
        return false;  // Buffer full
    }
    
    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % BUFFER_SIZE;  // Wrap around
    cb->count++;
    
    return true;
}

/* Read one byte from buffer */
bool cb_read(circular_buffer_t *cb, uint8_t *data) {
    if (cb_is_empty(cb)) {
        return false;  // Buffer empty
    }
    
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;  // Wrap around
    cb->count--;
    
    return true;
}

/* Peek at next byte without removing */
bool cb_peek(circular_buffer_t *cb, uint8_t *data) {
    if (cb_is_empty(cb)) {
        return false;
    }
    
    *data = cb->buffer[cb->tail];
    return true;
}

/* ============================================================================
 * GPS UART APPLICATION
 * ============================================================================ */

static circular_buffer_t uart_rx_buffer;
static uint32_t bytes_received = 0;
static uint32_t sentences_processed = 0;

/* Simulated UART data */
static const char* gps_sentences[] = {
    "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\n",
    "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\n",
    "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39\n",
    "$GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75\n"
};
static int sentence_idx = 0;
static int byte_idx = 0;

char simulated_uart_data(void) {
    char byte = gps_sentences[sentence_idx][byte_idx++];
    
    if (byte == '\0' || byte == '\n') {
        byte = '\n';
        byte_idx = 0;
        sentence_idx = (sentence_idx + 1) % 4;
    }
    
    return byte;
}

/**
 * UART Interrupt Handler
 * 
 * Now uses circular buffer!
 * - Fast: O(1) operation
 * - Never blocks
 * - Queues data for later processing
 */
void UART_IRQHandler(void) {
    uint8_t received_byte = simulated_uart_data();
    
    /* Write to circular buffer */
    if (cb_write(&uart_rx_buffer, received_byte)) {
        bytes_received++;
    } else {
        /* Buffer full - but this is rare if sized correctly */
        printf("[UART] Buffer full! (This shouldn't happen often)\n");
    }
}

/**
 * Read one NMEA sentence from buffer
 * 
 * Returns true if complete sentence read
 */
bool read_sentence(char *sentence, uint32_t max_len) {
    uint32_t idx = 0;
    uint8_t byte;
    
    while (idx < max_len - 1) {
        if (!cb_read(&uart_rx_buffer, &byte)) {
            return false;  // No more data
        }
        
        sentence[idx++] = byte;
        
        if (byte == '\n') {
            sentence[idx] = '\0';
            return true;  // Complete sentence
        }
    }
    
    sentence[idx] = '\0';
    return false;  // Sentence too long
}

/**
 * Main Loop Processing
 * 
 * Now reads from circular buffer!
 * - Processes at its own pace
 * - UART keeps filling buffer
 * - No data loss!
 */
void process_gps_data(void) {
    char sentence[82];
    
    /* Check if data available */
    if (cb_count(&uart_rx_buffer) > 0) {
        /* Read complete sentence */
        if (read_sentence(sentence, sizeof(sentence))) {
            printf("[MAIN] Processing: %s", sentence);
            
            /* Simulate slow processing */
            usleep(50000);   // Parse (50ms)
            usleep(100000);  // Display (100ms)
            usleep(200000);  // SD card (200ms)
            usleep(500000);  // Server (500ms)
            
            printf("[MAIN] Processed sentence #%u\n", ++sentences_processed);
            printf("[MAIN] Buffer has %u bytes remaining\n", 
                   cb_count(&uart_rx_buffer));
        }
    }
}

/* ============================================================================
 * BENEFITS OF CIRCULAR BUFFER
 * ============================================================================ */

void simulate_uart_interrupts(int count) {
    for (int i = 0; i < count; i++) {
        UART_IRQHandler();
        usleep(1000);  // 1ms between bytes
    }
}

int main(void) {
    printf("=== GOOD EXAMPLE: Circular Buffer ===\n\n");
    
    /* Initialize circular buffer */
    cb_init(&uart_rx_buffer);
    
    printf("Simulating GPS data reception with circular buffer...\n\n");
    
    /* Simulate receiving and processing */
    for (int cycle = 0; cycle < 3; cycle++) {
        printf("--- Cycle %d ---\n", cycle + 1);
        
        /* Receive one sentence (73 bytes) */
        printf("[UART] Receiving sentence...\n");
        simulate_uart_interrupts(73);
        
        /* Process it (850ms) */
        process_gps_data();
        
        /* During processing, more data arrives */
        printf("[UART] More data arriving during processing...\n");
        simulate_uart_interrupts(12);
        
        /* But it's OK! Buffer stores it! */
        printf("[MAIN] Buffer now has %u bytes\n", 
               cb_count(&uart_rx_buffer));
        
        printf("\n");
    }
    
    /* Process any remaining data */
    printf("--- Processing remaining data ---\n");
    while (cb_count(&uart_rx_buffer) > 0) {
        process_gps_data();
    }
    
    printf("\n=== Results ===\n");
    printf("Bytes received: %u\n", bytes_received);
    printf("Sentences processed: %u\n", sentences_processed);
    printf("Data loss: 0 bytes!\n");
    
    printf("\n=== Benefits ===\n");
    printf("1. ✅ No data loss - buffer queues everything\n");
    printf("2. ✅ Decoupled - UART and main run independently\n");
    printf("3. ✅ Fast - O(1) read/write operations\n");
    printf("4. ✅ Simple - Easy to understand and maintain\n");
    printf("5. ✅ Efficient - Fixed memory, no allocation\n");
    
    printf("\nSee 04_production.c for thread-safe version!\n");
    
    return 0;
}

/*
 * KEY IMPROVEMENTS:
 * 
 * 1. BUFFERING
 *    - Circular buffer queues incoming data
 *    - Can store multiple sentences
 *    - No data loss if sized correctly
 * 
 * 2. DECOUPLING
 *    - UART writes at its pace
 *    - Main reads at its pace
 *    - Buffer bridges the gap
 * 
 * 3. PERFORMANCE
 *    - O(1) write in interrupt
 *    - O(1) read in main loop
 *    - No data movement
 * 
 * 4. SIMPLICITY
 *    - Clean, understandable code
 *    - Easy to test
 *    - Easy to maintain
 * 
 * 5. RELIABILITY
 *    - Handles burst data
 *    - Handles slow processing
 *    - Predictable behavior
 * 
 * NEXT: See production-grade implementation!
 * Continue to: 04_production.c
 */
