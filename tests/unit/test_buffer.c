#include "test_buffer.h"
#include "comm/buffer.h"

int test_buffer_setup(UNUSED void** state)
{
    buffer_init();
    return 0;
}

void test_buffer_reads_and_writes_single_byte(UNUSED void** state)
{
    const u8 expectedData = 0x01;
    u8 actualData = 0;
    buffer_status_t writeStatus, readStatus;

    writeStatus = buffer_write(expectedData);
    assert_int_equal(writeStatus, BUFFER_OK);

    readStatus = buffer_read(&actualData);
    assert_int_equal(readStatus, BUFFER_OK);
    assert_int_equal(actualData, expectedData);
}

void test_buffer_reads_and_writes_circularly_over_capacity(UNUSED void** state)
{
    const u16 chunkSize = BUFFER_CAPACITY / 2;
    u8 data = 0;

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x00), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x01), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(&data), BUFFER_OK);
        assert_int_equal(data, 0x00);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x02), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(&data), BUFFER_OK);
        assert_int_equal(data, 0x01);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x03), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(&data), BUFFER_OK);
        assert_int_equal(data, 0x02);
    }
}

void test_buffer_available_returns_correct_value(UNUSED void** state)
{
    const u16 chunkSize = BUFFER_CAPACITY / 2;
    u8 data = 0;
    buffer_status_t status;

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x00), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x01), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        status = buffer_read(&data);
        assert_int_equal(status, BUFFER_OK);
        assert_int_equal(data, 0x00);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_write(0x02), BUFFER_OK);
    }

    for (u16 i = 0; i < chunkSize; i++) {
        status = buffer_read(&data);
        assert_int_equal(status, BUFFER_OK);
        assert_int_equal(data, 0x01);
    }

    assert_int_equal(buffer_available(), BUFFER_CAPACITY - chunkSize);
}

void test_buffer_available_returns_correct_value_when_empty(UNUSED void** state)
{
    assert_int_equal(buffer_available(), BUFFER_CAPACITY);
}

void test_buffer_available_returns_correct_value_when_full(UNUSED void** state)
{
    for (u16 i = 0; i < BUFFER_CAPACITY; i++) {
        assert_int_equal(buffer_write(0x00), BUFFER_OK);
    }
    assert_int_equal(buffer_available(), 0);
    assert_int_equal(buffer_write(0xFF), BUFFER_FULL);
}

void test_buffer_returns_cannot_write_if_full(UNUSED void** state)
{
    for (u16 i = 0; i < BUFFER_CAPACITY; i++) {
        assert_int_equal(buffer_write(0x00), BUFFER_OK);
    }
    assert_int_equal(buffer_can_write(), false);
}

void test_buffer_returns_can_write_if_empty(UNUSED void** state)
{
    assert_int_equal(buffer_can_write(), true);
}

void test_buffer_write_returns_full_when_buffer_is_full(UNUSED void** state)
{
    for (u16 i = 0; i < BUFFER_CAPACITY; i++) {
        assert_int_equal(buffer_write(0x00), BUFFER_OK);
    }

    assert_int_equal(buffer_write(0xFF), BUFFER_FULL);
}

void test_buffer_read_returns_empty_when_buffer_is_empty(UNUSED void** state)
{
    u8 data = 0;
    assert_int_equal(buffer_read(&data), BUFFER_EMPTY);
}

void test_buffer_read_returns_error_with_null_pointer(UNUSED void** state)
{
    buffer_write(0x42);

    assert_int_equal(buffer_read(NULL), BUFFER_ERROR);
}
