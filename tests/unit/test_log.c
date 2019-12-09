#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "log.h"
#include "unused.h"
#include <cmocka.h>

extern void __real_log_init(void);
extern void __real_log_info(const char* fmt, ...);
extern Log* __real_log_dequeue(void);

static int test_log_setup(void** state)
{
    __real_log_init();

    return 0;
}

static void test_log_info_writes_to_log_buffer(UNUSED void** state)
{
    __real_log_info("Test Message %d", 1, 0);

    Log* log = __real_log_dequeue();

    assert_int_not_equal(log->msgLen, 0);
    assert_memory_equal("Test Message 1", log->msg, log->msgLen);
}
