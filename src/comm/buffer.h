#pragma once
#include "genesis.h"

/**
 * Lock-free ring buffer implementation for Single Producer Single Consumer (SPSC) scenario.
 * - The producer (interrupt/main thread) only writes to the buffer
 * - The consumer (main thread) only reads from the buffer
 * - One slot is always kept empty to distinguish between full and empty states
 * - Maximum capacity is BUFFER_CAPACITY elements
 */

#define BUFFER_SIZE 4096
#define BUFFER_CAPACITY (BUFFER_SIZE - 1)

typedef enum { BUFFER_OK = 0, BUFFER_EMPTY, BUFFER_FULL, BUFFER_ERROR } buffer_status_t;

void buffer_init(void);

/**
 * Write a byte to the buffer
 * MUST be called ONLY from the producer (interrupt/main thread)
 * @param data The byte to write
 * @return BUFFER_OK if successful, BUFFER_FULL if buffer was full
 */
buffer_status_t buffer_write(u8 data);

/**
 * Read a byte from the buffer
 * MUST be called ONLY from the consumer (main thread)
 * @param data Pointer to store the read byte
 * @return BUFFER_OK if successful, BUFFER_EMPTY if buffer was empty
 */
buffer_status_t buffer_read(u8* data);

/**
 * Check if the buffer has data to read
 * Can be called from either context
 * @return true if buffer has data, false if empty
 */
bool buffer_can_read(void);

/**
 * Check if the buffer has space to write
 * Can be called from either context
 * @return true if buffer has space, false if full
 */
bool buffer_can_write(void);

/**
 * Get the number of free bytes in the buffer
 * Can be called from either context
 * @return Number of bytes that can be written
 */
u16 buffer_available(void);
