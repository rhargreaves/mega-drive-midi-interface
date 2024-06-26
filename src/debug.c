#include "debug.h"

#if defined UNIT_TESTS && defined DEBUG
#include "cmocka_inc.h"
#endif

void debug_message(const char* fmt, ...)
{
#if defined UNIT_TESTS && defined DEBUG
    va_list args;
    va_start(args, fmt);
    vprint_message(fmt, args);
    va_end(args);
#endif
}