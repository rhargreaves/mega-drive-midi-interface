#include "test_comm.h"
#include "comm/comm.h"
#include "mocks/mock_comm.h"

static const u16 MAX_COMM_IDLE = 0x28F;
static const u16 MAX_COMM_BUSY = 0x28F;

int test_comm_setup(UNUSED void** state)
{
    __real_comm_init();

    return 0;
}

static void switch_comm_type_to_everdrive(void)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, true);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read(&data, 0);
    __real_comm_reset_counts();
}

void test_comm_reads_from_serial_when_ready(UNUSED void** state)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, false);
    will_return(__wrap_comm_everdrive_pro_is_present, false);
    will_return(__wrap_comm_serial_is_present, true);
    will_return(__wrap_comm_serial_read_ready, 1);
    will_return(__wrap_comm_serial_read, 50);

    CommStatus status = __real_comm_read(&data, 0);

    assert_int_equal(status, COMM_OK);
    assert_int_equal(data, 50);
}

void test_comm_reads_everdrive_when_ready(UNUSED void** state)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, true);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    CommStatus status = __real_comm_read(&data, 0);

    assert_int_equal(status, COMM_OK);
    assert_int_equal(data, 50);
}

void test_comm_reads_demo_when_ready(UNUSED void** state)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, false);
    will_return(__wrap_comm_everdrive_pro_is_present, false);
    will_return(__wrap_comm_serial_is_present, true);
    will_return(__wrap_comm_serial_read_ready, 0);
    will_return(__wrap_comm_megawifi_is_present, true);
    will_return(__wrap_comm_megawifi_read_ready, 0);
    will_return(__wrap_comm_demo_is_present, true);
    will_return(__wrap_comm_demo_read_ready, 1);
    will_return(__wrap_comm_demo_read, 50);

    CommStatus status = __real_comm_read(&data, 0);

    assert_int_equal(status, COMM_OK);
    assert_int_equal(data, 50);
}

void test_comm_writes_when_ready(UNUSED void** state)
{
    const u8 test_data = 50;
    u8 data;

    will_return(__wrap_comm_everdrive_is_present, true);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read(&data, 0);

    will_return(__wrap_comm_everdrive_write_ready, 0);
    will_return(__wrap_comm_everdrive_write_ready, 1);
    expect_value(__wrap_comm_everdrive_write, data, test_data);

    __real_comm_write(test_data);
}

void test_comm_idle_count_is_correct(UNUSED void** state)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, true);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read(&data, 10);

    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read(&data, 10);
    u16 idle = __real_comm_idle_count();

    assert_int_equal(idle, 2);
}

void test_comm_busy_count_is_correct(UNUSED void** state)
{
    u8 data;
    will_return(__wrap_comm_everdrive_is_present, true);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    __real_comm_read(&data, 0);
    __real_comm_reset_counts();

    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read(&data, 0);
    __real_comm_read(&data, 0);
    u16 busy = __real_comm_busy_count();

    assert_int_equal(busy, 2);
}

void test_comm_clamps_idle_count(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    for (u16 i = 0; i < MAX_COMM_IDLE + 1; i++) {
        will_return(__wrap_comm_everdrive_read_ready, 0);
    }
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    __real_comm_read(&data, 1000);
    u16 idle = __real_comm_idle_count();

    assert_int_equal(idle, MAX_COMM_IDLE);
}

void test_comm_clamps_busy_count(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    for (u16 i = 0; i < MAX_COMM_BUSY + 1; i++) {
        will_return(__wrap_comm_everdrive_read_ready, 1);
        will_return(__wrap_comm_everdrive_read, 50);
        __real_comm_read(&data, 0);
    }

    u16 busy = __real_comm_busy_count();

    assert_int_equal(busy, MAX_COMM_BUSY);
}

void test_comm_read_with_data_ready_immediately(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 50);

    CommStatus status = __real_comm_read(&data, 100);

    assert_int_equal(status, COMM_OK);
    assert_int_equal(data, 50);
}

void test_comm_read_with_data_ready_before_timeout(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 0);
    will_return(__wrap_comm_everdrive_read_ready, 1);
    will_return(__wrap_comm_everdrive_read, 75);

    CommStatus status = __real_comm_read(&data, 100);

    assert_int_equal(status, COMM_OK);
    assert_int_equal(data, 75);
}

void test_comm_read_with_timeout_expired(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    for (u16 i = 0; i < 100; i++) {
        will_return(__wrap_comm_everdrive_read_ready, 0);
    }

    CommStatus status = __real_comm_read(&data, 100);

    assert_int_equal(status, COMM_TIMEOUT);
}

void test_comm_read_non_blocking_no_data(UNUSED void** state)
{
    u8 data;
    switch_comm_type_to_everdrive();

    will_return(__wrap_comm_everdrive_read_ready, 0);

    CommStatus status = __real_comm_read(&data, 0);

    assert_int_equal(status, COMM_NOT_READY);
}
