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

u8 buffer_read(void)
{
    u8 data = buffer[readHead];
    length--;

    readHead++;
    if (readHead == BUFFER_SIZE) {
        readHead = 0;
    }
    return data;
}

void buffer_write(u8 data)
{
    buffer[writeHead] = data;
    length++;

    writeHead++;
    if (writeHead == BUFFER_SIZE) {
        writeHead = 0;
    }
}

u8 buffer_canRead(void)
{
    return length != 0;
}

u16 buffer_available(void)
{
    return BUFFER_SIZE - length;
}
