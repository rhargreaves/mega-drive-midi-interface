#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "comm.h"
#include <cmocka.h>

extern u8 __real_comm_read(void);
extern u16 __real_comm_idleCount(void);
extern u16 __real_comm_busyCount(void);
extern void __real_comm_resetCounts(void);

static int test_comm_setup(void** state)
{
    __real_comm_resetCounts();

    return 0;
}

static void test_comm_reads_when_ready(void** state)
{
    will_return(__wrap_ssf_usb_rd_ready, 0);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, 50);

    u8 read = __real_comm_read();

    assert_int_equal(read, 50);
}

static void test_comm_idle_count_is_correct(void** state)
{
    will_return(__wrap_ssf_usb_rd_ready, 0);
    will_return(__wrap_ssf_usb_rd_ready, 0);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, 50);

    __real_comm_read();
    u16 idle = __real_comm_idleCount();

    assert_int_equal(idle, 2);
}

static void test_comm_busy_count_is_correct(void** state)
{
    will_return(__wrap_ssf_usb_rd_ready, 0);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, 50);
    will_return(__wrap_ssf_usb_read, 50);

    __real_comm_read();
    __real_comm_read();
    u16 busy = __real_comm_busyCount();

    assert_int_equal(busy, 2);
}

static void test_comm_clamps_idle_count(void** state)
{
    for (int i = 0; i < 0xFFFF; i++) {
        will_return(__wrap_ssf_usb_rd_ready, 0);
    }
    will_return(__wrap_ssf_usb_rd_ready, 1);
    will_return(__wrap_ssf_usb_read, 50);

    __real_comm_read();
    u16 idle = __real_comm_idleCount();

    assert_int_equal(idle, 0);
}

static void test_comm_clamps_busy_count(void** state)
{
    for (int i = 0; i < 0xFFFF; i++) {
        will_return(__wrap_ssf_usb_rd_ready, 1);
        will_return(__wrap_ssf_usb_read, 50);
        __real_comm_read();
    }

    u16 busy = __real_comm_busyCount();

    assert_int_equal(busy, 0);
}
