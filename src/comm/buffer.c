#include "buffer.h"

static u16 readHead = 0;
static volatile u16 writeHead = 0;
static volatile char buffer[BUFFER_SIZE];
static u16 length = 0;

void buffer_init(void)
{
    readHead = 0;
    writeHead = 0;
    length = 0;
}

buffer_status_t buffer_read(u8* data)
{
    if (data == NULL) {
        return BUFFER_ERROR;
    }

    if (length == 0) {
        return BUFFER_EMPTY;
    }

    *data = buffer[readHead];
    length--;

    readHead++;
    if (readHead == BUFFER_SIZE) {
        readHead = 0;
    }
    return BUFFER_OK;
}

buffer_status_t buffer_write(u8 data)
{
    if (length == BUFFER_SIZE) {
        return BUFFER_FULL;
    }

    buffer[writeHead] = data;
    length++;

    writeHead++;
    if (writeHead == BUFFER_SIZE) {
        writeHead = 0;
    }
    return BUFFER_OK;
}

bool buffer_can_read(void)
{
    return length != 0;
}

u16 buffer_available(void)
{
    return BUFFER_SIZE - length;
}

bool buffer_can_write(void)
{
    return length != BUFFER_SIZE;
}
