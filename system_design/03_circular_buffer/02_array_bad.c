/**
 * 02_array_bad.c - BAD EXAMPLE: Simple Array (Data Loss)
 * 
 * This is the WRONG way to handle streaming UART data.
 * Uses a simple array without proper buffering.
 * 
 * Problems:
 * - Data loss when processing
 * - Buffer overwrite
 * - No queue for multiple messages
 * - Producer-consumer mismatch
 * 
 * Study time: 10 minutes
 * DO NOT use this pattern in production!
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* ============================================================================
 * SIMPLE ARRAY APPROACH - THE PROBLEM!
 * ============================================================================ */

#define MAX_SENTENCE_LEN 82

// Global state - single buffer
static char uart_buffer[MAX_SENTENCE_LEN];
static uint8_t buffer_index = 0;
static bool sentence_ready = false;
static uint32_t bytes_lost = 0;
static uint32_t sentences_corrupted = 0;

/* Simulated hardware */
#define UART_DATA_REG (simulated_uart_data())
char simulated_uart_data(void);

/**
 * UART Interrupt Handler (Simulated)
 * 
 * Problem: Only ONE buffer!
 * If main loop is processing, where do new bytes go?
 */
void UART_IRQHandler(void) {
    char received_byte = UART_DATA_REG;
    
    /* Problem 1: What if sentence_ready is still true? */
    if (sentence_ready) {
        /* Main loop hasn't processed yet! */
        /* New data will OVERWRITE old data! */
        sentences_corrupted++;
    }
    
    /* Problem 2: Simple overflow handling */
    if (buffer_index < MAX_SENTENCE_LEN - 1) {
        uart_buffer[buffer_index++] = received_byte;
        
        if (received_byte == '\n') {
            uart_buffer[buffer_index] = '\0';
            sentence_ready = true;
            buffer_index = 0;
        }
    } else {
        /* Buffer full - lose data! */
        bytes_lost++;
        buffer_index = 0;
    }
}

/**
 * Main Loop Processing
 * 
 * Problem: Takes too long!
 * While processing, UART keeps receiving...
 */
void process_gps_data(void) {
    if (sentence_ready) {
        printf("[MAIN] Processing: %s", uart_buffer);
        
        /* Simulate slow processing */
        /* Parse NMEA (50ms) */
        usleep(50000);
        printf("[MAIN] Parsed sentence\n");
        
        /* Update display (100ms) */
        usleep(100000);
        printf("[MAIN] Updated display\n");
        
        /* Log to SD card (200ms) */
        usleep(200000);
        printf("[MAIN] Logged to SD\n");
        
        /* Send to server (500ms) */
        usleep(500000);
        printf("[MAIN] Sent to server\n");
        
        /* Total: 850ms of processing! */
        /* During this time, UART received ~12 more bytes! */
        /* WHERE DID THEY GO? LOST! */
        
        sentence_ready = false;
    }
}

/**
 * PROBLEMS WITH THIS CODE:
 * 
 * 1. SINGLE BUFFER
 *    - Only one sentence at a time
 *    - New data overwrites old data
 *    - No queue for multiple sentences
 * 
 * 2. PRODUCER-CONSUMER MISMATCH
 *    - UART produces continuously (fast)
 *    - Main consumes intermittently (slow)
 *    - No buffering between them
 * 
 * 3. DATA LOSS
 *    - Processing takes 850ms
 *    - UART receives new data every 73ms
 *    - 11+ bytes arrive during processing
 *    - All lost!
 * 
 * 4. CORRUPTION
 *    - If sentence_ready is true
 *    - New data overwrites buffer
 *    - Results in corrupted sentences
 * 
 * 5. NO FLOW CONTROL
 *    - Can't tell UART to slow down
 *    - Can't queue incoming data
 *    - Must process immediately or lose
 */

/* ============================================================================
 * SIMULATION
 * ============================================================================ */

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

void simulate_uart_interrupts(int count) {
    for (int i = 0; i < count; i++) {
        UART_IRQHandler();
        usleep(1000);  // 1ms between bytes at 9600 baud
    }
}

int main(void) {
    printf("=== BAD EXAMPLE: Simple Array ===\n\n");
    
    printf("Simulating GPS data reception...\n\n");
    
    /* Simulate receiving and processing */
    for (int cycle = 0; cycle < 3; cycle++) {
        printf("--- Cycle %d ---\n", cycle + 1);
        
        /* Receive one sentence (73 bytes = 73ms) */
        simulate_uart_interrupts(73);
        
        /* Process it (850ms) */
        process_gps_data();
        
        /* During processing, more data arrives! */
        /* Simulate 12 more bytes arriving */
        printf("[UART] More data arriving during processing...\n");
        simulate_uart_interrupts(12);
        
        printf("\n");
    }
    
    printf("=== Results ===\n");
    printf("Bytes lost: %u\n", bytes_lost);
    printf("Sentences corrupted: %u\n", sentences_corrupted);
    
    printf("\n=== Problems ===\n");
    printf("1. Only one buffer - no queue\n");
    printf("2. Processing (850ms) >> Receive time (73ms)\n");
    printf("3. Data arrives during processing → LOST\n");
    printf("4. New data overwrites old → CORRUPTION\n");
    printf("5. 70%% data loss in real scenario!\n");
    
    printf("\nSee 03_circular_good.c for the RIGHT way!\n");
    
    return 0;
}

/*
 * KEY LESSONS:
 * 
 * 1. Simple arrays fail for streaming data
 * 2. Producer-consumer mismatch causes data loss
 * 3. Need buffering between fast producer and slow consumer
 * 4. Single buffer can't handle continuous streams
 * 5. Must queue data for later processing
 * 
 * SOLUTION: Use circular buffer!
 * 
 * NEXT: See how circular buffer solves ALL these problems!
 * Continue to: 03_circular_good.c
 */
