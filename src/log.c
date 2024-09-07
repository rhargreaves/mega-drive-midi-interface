#include "log.h"

#define MAX_LOG_ENTRIES 10

static Log logs[MAX_LOG_ENTRIES];
static u8 readHead;
static u8 writeHead;
static u16 count;

extern u16 vsprintf(char* buf, const char* fmt, va_list args);

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

static void log(LogLevel level, const char* fmt, va_list args)
{
    Log* log = &logs[writeHead];
    incrementWriteHead();
    vsprintf(log->msg, fmt, args);
    log->level = level;
    log->msgLen = MSG_MAX_LEN;
}

void log_info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log(Info, fmt, args);
    va_end(args);
}

void log_warn(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log(Warn, fmt, args);
    va_end(args);
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
