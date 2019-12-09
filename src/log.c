#include "log.h"

static Log theOneAndOnlyLog = {};

void log_init(void) {}
void log_info(const char* fmt, ...){}
Log* log_dequeue(void){
 return &theOneAndOnlyLog;
}
