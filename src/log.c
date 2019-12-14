#include "log.h"

#include <stdbool.h>

#include <memory.h>
#include <string.h>

#define MAX_LOG_ENTRIES 10

static Log logs[MAX_LOG_ENTRIES];
static u8 readHead;
static u8 writeHead;
static u16 count;

void log_init(void)
{
    for (u8 i = 0; i < MAX_LOG_ENTRIES; i++) {
        Log* log = &logs[i];
        log->level = Info;
        memset(&log->msg, 0, MSG_MAX_LEN);
        log->msgLen = 0;
    }
    readHead = 0;
    writeHead = 0;
    count = 0;
}

static void incrementReadHead(void)
{
    readHead++;
    if (readHead == MAX_LOG_ENTRIES) {
        readHead = 0;
    }
    count--;
}

static void incrementWriteHead(void)
{
    writeHead++;
    if (writeHead == MAX_LOG_ENTRIES) {
        writeHead = 0;
    }
    if (writeHead == readHead) {
        incrementReadHead();
    }
    count++;
}

void log_info(const char* fmt, u8 val1, u8 val2, u8 val3)
{
    Log* log = &logs[writeHead];
    incrementWriteHead();
    sprintf(log->msg, fmt, val1, val2, val3);
    log->level = Info;
    log->msgLen = MSG_MAX_LEN;
}

Log* log_dequeue(void)
{
    if (count == 0) {
        return NULL;
    }
    Log* log = &logs[readHead];
    incrementReadHead();
    return log;
}
