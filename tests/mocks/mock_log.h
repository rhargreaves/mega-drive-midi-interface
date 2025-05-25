#pragma once
#include "cmocka_inc.h"
#include "log.h"

void __wrap_log_init(void);
void __wrap_log_info(const char* fmt, ...);
void __wrap_log_warn(const char* fmt, ...);
Log* __wrap_log_dequeue(void);

extern void __real_log_init(void);
extern void __real_log_info(const char* fmt, ...);
extern void __real_log_warn(const char* fmt, ...);
extern Log* __real_log_dequeue(void);

void mock_log_disable_checks(void);
void mock_log_enable_checks(void);

void _expect_log_info(const char* fmt, const char* const file, const int line);
void _expect_log_warn(const char* fmt, const char* const file, const int line);

#define expect_log_info(fmt) _expect_log_info(fmt, __FILE__, __LINE__)
#define expect_log_warn(fmt) _expect_log_warn(fmt, __FILE__, __LINE__)