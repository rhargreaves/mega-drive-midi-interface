#include "log.h"

#include <memory.h>
#include <string.h>

static Log theOneAndOnlyLog;

void log_init(void)
{
    theOneAndOnlyLog.level = Info;
    memset(&theOneAndOnlyLog.msg, 0, MSG_MAX_LEN);
    theOneAndOnlyLog.msgLen = 0;
}

void log_info(const char* fmt, u8 val1, u8 val2, u8 val3)
{
    Log* log = &theOneAndOnlyLog;
    sprintf(log->msg, fmt, val1, val2, val3);
    log->level = Info;
    log->msgLen = MSG_MAX_LEN;
}

Log* log_dequeue(void)
{
    return &theOneAndOnlyLog;
}
