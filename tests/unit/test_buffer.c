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

    buffer_write(expectedData);

    assert_int_equal(buffer_read(), expectedData);
}

void test_buffer_reads_and_writes_circularly_over_capacity(UNUSED void** state)
{
    const u16 chunkSize = BUFFER_SIZE / 2;

    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x00);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x01);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(), 0x00);
    };
    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x02);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(), 0x01);
    };
    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x03);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(), 0x02);
    };
}

void test_buffer_available_returns_correct_value(UNUSED void** state)
{
    const u16 chunkSize = BUFFER_SIZE / 2;

    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x00);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x01);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(), 0x00);
    };
    for (u16 i = 0; i < chunkSize; i++) {
        buffer_write(0x02);
    }
    for (u16 i = 0; i < chunkSize; i++) {
        assert_int_equal(buffer_read(), 0x01);
    };

    assert_int_equal(buffer_available(), chunkSize);
}

void test_buffer_available_returns_correct_value_when_empty(UNUSED void** state)
{
    assert_int_equal(buffer_available(), BUFFER_SIZE);
}

void test_buffer_available_returns_correct_value_when_full(UNUSED void** state)
{
    for (u16 i = 0; i < BUFFER_SIZE; i++) {
        buffer_write(0x00);
    }
    assert_int_equal(buffer_available(), 0);
}

void test_buffer_returns_cannot_write_if_full(UNUSED void** state)
{
    for (u16 i = 0; i < BUFFER_SIZE; i++) {
        buffer_write(0x00);
    }
    assert_int_equal(buffer_can_write(), false);
}

void test_buffer_returns_can_write_if_empty(UNUSED void** state)
{
    assert_int_equal(buffer_can_write(), true);
}
