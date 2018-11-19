#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "comm.h"
#include <cmocka.h>

extern u8 __real_comm_read(void);
extern u16 __real_comm_idleCount(void);
extern u16 __real_comm_busyCount(void);
extern void __real_comm_resetCounts(void);

static void test_comm_reads_when_ready(void** state)
{
    will_return(__wrap_ssf_usb_rd_ready, 0);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, 50);

    u8 read = __real_comm_read();

    assert_int_equal(read, 50);
}
