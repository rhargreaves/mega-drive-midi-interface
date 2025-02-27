#include "test_ring_buf.h"
#include "comm/ring_buf.h"

int test_ring_buf_setup(UNUSED void** state)
{
    ring_buf_init();
    return 0;
}

void test_ring_buf_reads_and_writes_single_byte(UNUSED void** state)
{
    const u8 expectedData = 0x01;
    u8 actualData = 0;
    ring_buf_status_t writeStatus, readStatus;

    writeStatus = ring_buf_write(expectedData);
    assert_int_equal(writeStatus, RING_BUF_OK);

    readStatus = ring_buf_read(&actualData);
    assert_int_equal(readStatus, RING_BUF_OK);
    assert_int_equal(actualData, expectedData);
}

void test_ring_buf_reads_and_writes_circularly_over_capacity(UNUSED void** state)
{
    const u16 chunkSize = RING_BUF_CAPACITY / 2;
    u8 data = 0;

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x00), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x01), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_read(&data), RING_BUF_OK);
        assert_int_equal(data, 0x00);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x02), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_read(&data), RING_BUF_OK);
        assert_int_equal(data, 0x01);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x03), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_read(&data), RING_BUF_OK);
        assert_int_equal(data, 0x02);
    }
}

void test_ring_buf_available_returns_correct_value(UNUSED void** state)
{
    const u16 chunkSize = RING_BUF_CAPACITY / 2;
    u8 data = 0;
    ring_buf_status_t status;

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x00), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x01), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        status = ring_buf_read(&data);
        assert_int_equal(status, RING_BUF_OK);
        assert_int_equal(data, 0x00);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(ring_buf_write(0x02), RING_BUF_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        status = ring_buf_read(&data);
        assert_int_equal(status, RING_BUF_OK);
        assert_int_equal(data, 0x01);
    }

    assert_int_equal(ring_buf_available(), RING_BUF_CAPACITY - chunkSize);
}

void test_ring_buf_available_returns_correct_value_when_empty(UNUSED void** state)
{
    assert_int_equal(ring_buf_available(), RING_BUF_CAPACITY);
}

void test_ring_buf_available_returns_correct_value_when_full(UNUSED void** state)
{
    for (u16 i = 0; i < RING_BUF_CAPACITY; i++) {
        assert_int_equal(ring_buf_write(0x00), RING_BUF_OK);
    }
    assert_int_equal(ring_buf_available(), 0);
    assert_int_equal(ring_buf_write(0xFF), RING_BUF_FULL);
}

void test_ring_buf_returns_cannot_write_if_full(UNUSED void** state)
{
    for (u16 i = 0; i < RING_BUF_CAPACITY; i++) {
        assert_int_equal(ring_buf_write(0x00), RING_BUF_OK);
    }
    assert_int_equal(ring_buf_can_write(), false);
}

void test_ring_buf_returns_can_write_if_empty(UNUSED void** state)
{
    assert_int_equal(ring_buf_can_write(), true);
}

void test_ring_buf_write_returns_full_when_buffer_is_full(UNUSED void** state)
{
    for (u16 i = 0; i < RING_BUF_CAPACITY; i++) {
        assert_int_equal(ring_buf_write(0x00), RING_BUF_OK);
    }

    assert_int_equal(ring_buf_write(0xFF), RING_BUF_FULL);
}

void test_ring_buf_read_returns_empty_when_buffer_is_empty(UNUSED void** state)
{
    u8 data = 0;
    assert_int_equal(ring_buf_read(&data), RING_BUF_EMPTY);
}

void test_ring_buf_read_returns_error_with_null_pointer(UNUSED void** state)
{
    ring_buf_write(0x42);

    assert_int_equal(ring_buf_read(NULL), RING_BUF_ERROR);
}
