#pragma once
#include <types.h>

#include <stdarg.h>

typedef enum LogLevel LogLevel;

enum LogLevel { Info, Warn, Error };

typedef struct Log Log;

struct Log {
    LogLevel level;
    u16 msgLen;
    char* msg;
};

void log_init(void);
void log_info(const char* fmt, ...);
Log* log_dequeue(void);
