#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "comm_megawifi.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"

#include "wraps.h"

#include "unused.h"
#include <cmocka.h>

extern void __real_comm_megawifi_init(void);

static int test_comm_megawifi_setup(UNUSED void** state)
{
    //  __real_comm_megawifi_init();
    log_init();
    wraps_enable_logging_checks();
    return 0;
}

static void test_comm_megawifi_initialises(UNUSED void** state)
{
    expect_any(__wrap_mw_init, cmd_buf);
    expect_any(__wrap_mw_init, buf_len);
    will_return(__wrap_mw_init, MW_ERR_NONE);

    expect_value(__wrap_loop_init, max_func, 2);
    expect_value(__wrap_loop_init, max_timer, 4);
    will_return(__wrap_loop_init, MW_ERR_NONE);

    expect_any(__wrap_loop_func_add, func);
    will_return(__wrap_loop_func_add, MW_ERR_NONE);

    mock_mw_detect(3, 1);
    will_return(__wrap_mw_detect, MW_ERR_NONE);

    expect_memory(__wrap_log_info, fmt, "Found MegaWiFi %d.%d", 21);
    expect_any(__wrap_log_info, val1);
    expect_any(__wrap_log_info, val2);
    expect_value(__wrap_log_info, val3, 0);

    __real_comm_megawifi_init();
}
