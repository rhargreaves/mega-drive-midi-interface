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

    return 0;
}

static void test_comm_megawifi_initialises(UNUSED void** state)
{
    //    mw_init(cmd_buf, MW_BUFLEN);
    // mw_process();

    expect_any(__wrap_mw_init, cmd_buf);
    expect_any(__wrap_mw_init, buf_len);
    will_return(__wrap_mw_init, 0);
    expect_function_call(__wrap_mw_process);

    __real_comm_megawifi_init();
}
