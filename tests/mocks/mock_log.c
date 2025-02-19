#include "cmocka_inc.h"
#include "mocks/mock_log.h"

static bool loggingChecks = false;

void wraps_disable_logging_checks(void)
{
    loggingChecks = false;
}

void wraps_enable_logging_checks(void)
{
    loggingChecks = true;
}

void __wrap_log_init(void)
{
}

void __wrap_log_info(const char* fmt, ...)
{
    if (!loggingChecks) {
        return;
    }
    check_expected(fmt);
}

void __wrap_log_warn(const char* fmt, ...)
{
    if (!loggingChecks) {
        return;
    }
    check_expected(fmt);
}

Log* __wrap_log_dequeue(void)
{
    return mock_type(Log*);
}

void __wrap_VDP_clearTextArea(u16 x, u16 y, u16 w, u16 h)
{
}
