#include "buffer.h"

static volatile u16 tail = 0;
static volatile u16 head = 0;
static volatile u8 buffer[BUFFER_SIZE];

void buffer_init(void)
{
    tail = 0;
    head = 0;
}

buffer_status_t buffer_read(u8* data)
{
    if (data == NULL) {
        return BUFFER_ERROR;
    }

    if (tail == head) {
        return BUFFER_EMPTY;
    }

    *data = buffer[tail];
    tail = (tail + 1) % BUFFER_SIZE;
    return BUFFER_OK;
}

buffer_status_t buffer_write(u8 data)
{
    u16 nextHead = (head + 1) % BUFFER_SIZE;
    if (nextHead == tail) {
        return BUFFER_FULL;
    }

    buffer[head] = data;
    head = nextHead;
    return BUFFER_OK;
}

bool buffer_can_read(void)
{
    return tail != head;
}

u16 buffer_available(void)
{
    if (tail == head) {
        return BUFFER_CAPACITY;
    } else if (tail < head) {
        return BUFFER_CAPACITY - (head - tail);
    } else {
        return tail - head - 1;
    }
}

bool buffer_can_write(void)
{
    u16 nextHead = (head + 1) % BUFFER_SIZE;
    return nextHead != tail;
}
