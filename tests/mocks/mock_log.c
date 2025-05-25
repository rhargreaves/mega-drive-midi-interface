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
    vprint_message(fmt, args);
    print_message("\n");
    va_end(args);

    check_expected(fmt);
}

Log* __wrap_log_dequeue(void)
{
    return mock_type(Log*);
}

void _expect_log_info(const char* fmt, const char* const file, const int line)
{
    expect_string_with_pos(__wrap_log_info, fmt, fmt, file, line);
}

void _expect_log_warn(const char* fmt, const char* const file, const int line)
{
    expect_string_with_pos(__wrap_log_warn, fmt, fmt, file, line);
}