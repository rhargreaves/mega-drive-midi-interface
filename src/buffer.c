#include <genesis.h>

#define BUFFER_SIZE 256

static u16 readHead = 0;
static volatile u16 writeHead = 0;
static volatile char buffer[BUFFER_SIZE];

u8 buffer_read(void)
{
    u8 data = buffer[readHead++];
    if (readHead == BUFFER_SIZE) {
        readHead = 0;
    }
    return data;
}

void buffer_write(u8 data)
{
    buffer[writeHead++] = data;
    if (writeHead == BUFFER_SIZE) {
        writeHead = 0;
    }
}

u8 buffer_canRead(void)
{
    return writeHead != readHead;
}

u16 buffer_available(void)
{
    /*
    ----R--------W-----
    xxxxx        xxxxxx

    ----W--------R-----
        xxxxxxxxx
    */
    if (writeHead >= readHead) {
        return BUFFER_SIZE - (writeHead - readHead);
    } else {
        return readHead - writeHead;
    }
}
