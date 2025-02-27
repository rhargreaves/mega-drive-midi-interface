#include "buffer.h"

static volatile u16 readHead = 0;
static volatile u16 writeHead = 0;
static volatile u8 buffer[BUFFER_SIZE];

void buffer_init(void)
{
    readHead = 0;
    writeHead = 0;
}

buffer_status_t buffer_read(u8* data)
{
    if (data == NULL) {
        return BUFFER_ERROR;
    }

    if (readHead == writeHead) {
        return BUFFER_EMPTY;
    }

    *data = buffer[readHead];
    readHead = (readHead + 1) % BUFFER_SIZE;
    return BUFFER_OK;
}

buffer_status_t buffer_write(u8 data)
{
    u16 nextWriteHead = (writeHead + 1) % BUFFER_SIZE;
    if (nextWriteHead == readHead) {
        return BUFFER_FULL;
    }

    buffer[writeHead] = data;
    writeHead = nextWriteHead;
    return BUFFER_OK;
}

bool buffer_can_read(void)
{
    return readHead != writeHead;
}

u16 buffer_available(void)
{
    if (readHead == writeHead) {
        return BUFFER_CAPACITY;
    } else if (readHead < writeHead) {
        return BUFFER_CAPACITY - (writeHead - readHead);
    } else {
        return readHead - writeHead - 1;
    }
}

bool buffer_can_write(void)
{
    u16 nextWriteHead = (writeHead + 1) % BUFFER_SIZE;
    return nextWriteHead != readHead;
}
