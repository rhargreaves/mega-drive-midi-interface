#include "ring_buf.h"

void ring_buf_init(ring_buf_t* rb, u8* ringBuf, u16 size)
{
    if (ringBuf == NULL || rb == NULL || size < 2) {
        return;
    }
    rb->ringBuf = ringBuf;
    rb->tail = 0;
    rb->head = 0;
    rb->size = size;
}

ring_buf_status_t ring_buf_read(ring_buf_t* rb, u8* data)
{
    if (rb == NULL || rb->ringBuf == NULL || rb->size < 2 || data == NULL) {
        return RING_BUF_ERROR;
    }

    if (rb->tail == rb->head) {
        return RING_BUF_EMPTY;
    }

    *data = rb->ringBuf[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    return RING_BUF_OK;
}

ring_buf_status_t ring_buf_write(ring_buf_t* rb, u8 data)
{
    if (rb == NULL || rb->ringBuf == NULL || rb->size < 2) {
        return RING_BUF_ERROR;
    }

    u16 nextHead = (rb->head + 1) % rb->size;
    if (nextHead == rb->tail) {
        return RING_BUF_FULL;
    }

    rb->ringBuf[rb->head] = data;
    rb->head = nextHead;
    return RING_BUF_OK;
}

bool ring_buf_can_read(ring_buf_t* rb)
{
    if (rb == NULL || rb->ringBuf == NULL || rb->size < 2) {
        return false;
    }
    return rb->tail != rb->head;
}

u16 ring_buf_available(ring_buf_t* rb)
{
    if (rb == NULL || rb->ringBuf == NULL || rb->size < 2) {
        return 0;
    }

    u16 capacity = rb->size - 1;
    if (rb->tail == rb->head) {
        return capacity;
    } else if (rb->tail < rb->head) {
        return capacity - (rb->head - rb->tail);
    } else {
        return rb->tail - rb->head - 1;
    }
}

bool ring_buf_can_write(ring_buf_t* rb)
{
    if (rb == NULL || rb->ringBuf == NULL || rb->size < 2) {
        return false;
    }

    u16 nextHead = (rb->head + 1) % rb->size;
    return nextHead != rb->tail;
}
