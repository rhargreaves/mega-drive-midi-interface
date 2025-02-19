#pragma once
#include "cmocka_inc.h"

void wraps_disable_logging_checks(void);
void wraps_enable_logging_checks(void);

void __wrap_log_init(void);
void __wrap_log_info(const char* fmt, ...);
void __wrap_log_warn(const char* fmt, ...);
Log* __wrap_log_dequeue(void);
extern void __real_log_init(void);
extern void __real_log_info(const char* fmt, ...);
extern void __real_log_warn(const char* fmt, ...);
extern Log* __real_log_dequeue(void);

#define expect_log_info(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_info, fmt, f, sizeof(f));                                         \
    }

#define expect_log_warn(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_warn, fmt, f, sizeof(f));                                         \
    }
