#pragma once
#include "genesis.h"

/**
 * Lock-free ring buffer implementation for Single Producer Single Consumer (SPSC) scenario.
 * - The producer (interrupt/main thread) only writes to the buffer
 * - The consumer (main thread) only reads from the buffer
 * - One slot is always kept empty to distinguish between full and empty states
 * - Maximum capacity is RING_BUF_CAPACITY elements
 */

typedef enum { RING_BUF_OK = 0, RING_BUF_EMPTY, RING_BUF_FULL, RING_BUF_ERROR } ring_buf_status_t;

typedef struct {
    volatile u8* ringBuf;
    volatile u16 tail;
    volatile u16 head;
    u16 size;
} ring_buf_t;

void ring_buf_init(ring_buf_t* rb, u8* ringBuf, u16 size);

/**
 * Write a byte to the buffer
 * MUST be called ONLY from the producer (interrupt/main thread)
 * @param data The byte to write
 * @return RING_BUF_OK if successful, RING_BUF_FULL if buffer was full
 */
ring_buf_status_t ring_buf_write(ring_buf_t* rb, u8 data);

/**
 * Read a byte from the buffer
 * MUST be called ONLY from the consumer (main thread)
 * @param data Pointer to store the read byte
 * @return RING_BUF_OK if successful, RING_BUF_EMPTY if buffer was empty
 */
ring_buf_status_t ring_buf_read(ring_buf_t* rb, u8* data);

/**
 * Check if the buffer has data to read
 * Can be called from either context
 * @return true if buffer has data, false if empty
 */
bool ring_buf_can_read(ring_buf_t* rb);

/**
 * Check if the buffer has space to write
 * Can be called from either context
 * @return true if buffer has space, false if full
 */
bool ring_buf_can_write(ring_buf_t* rb);

/**
 * Get the number of free bytes in the buffer
 * Can be called from either context
 * @return Number of bytes that can be written
 */
u16 ring_buf_available(ring_buf_t* rb);
