#pragma once
#include "types.h"
#include "vstring.h"

#define MSG_MAX_LEN 40

typedef enum LogLevel { Info, Warn } LogLevel;

typedef struct Log {
    LogLevel level;
    u16 msgLen;
    char msg[MSG_MAX_LEN];
} Log;

void log_init(void);
void log_info(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void log_warn(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
Log* log_dequeue(void);
