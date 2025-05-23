#include "cmocka_inc.h"
#include "mocks/mock_log.h"

static bool loggingChecks = false;

void mock_log_disable_checks(void)
{
    loggingChecks = false;
}

void mock_log_enable_checks(void)
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

    va_list args;
    va_start(args, fmt);
    print_message(fmt, args);
    va_end(args);

    check_expected(fmt);
}

Log* __wrap_log_dequeue(void)
{
    return mock_type(Log*);
}
