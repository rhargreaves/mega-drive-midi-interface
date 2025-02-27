#include "ring_buf.h"

static volatile u16 tail = 0;
static volatile u16 head = 0;
static volatile u8 ring_buf[RING_BUF_SIZE];

void ring_buf_init(void)
{
    tail = 0;
    head = 0;
}

ring_buf_status_t ring_buf_read(u8* data)
{
    if (data == NULL) {
        return RING_BUF_ERROR;
    }

    if (tail == head) {
        return RING_BUF_EMPTY;
    }

    *data = ring_buf[tail];
    tail = (tail + 1) % RING_BUF_SIZE;
    return RING_BUF_OK;
}

ring_buf_status_t ring_buf_write(u8 data)
{
    u16 nextHead = (head + 1) % RING_BUF_SIZE;
    if (nextHead == tail) {
        return RING_BUF_FULL;
    }

    ring_buf[head] = data;
    head = nextHead;
    return RING_BUF_OK;
}

bool ring_buf_can_read(void)
{
    return tail != head;
}

u16 ring_buf_available(void)
{
    if (tail == head) {
        return RING_BUF_CAPACITY;
    } else if (tail < head) {
        return RING_BUF_CAPACITY - (head - tail);
    } else {
        return tail - head - 1;
    }
}

bool ring_buf_can_write(void)
{
    u16 nextHead = (head + 1) % RING_BUF_SIZE;
    return nextHead != tail;
}
