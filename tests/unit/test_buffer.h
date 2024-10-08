#include "cmocka_inc.h"

int test_buffer_setup(UNUSED void** state);
void test_buffer_reads_and_writes_single_byte(UNUSED void** state);
void test_buffer_reads_and_writes_circularly_over_capacity(UNUSED void** state);
void test_buffer_available_returns_correct_value(UNUSED void** state);
void test_buffer_available_returns_correct_value_when_empty(UNUSED void** state);
void test_buffer_available_returns_correct_value_when_full(UNUSED void** state);
void test_buffer_returns_cannot_write_if_full(UNUSED void** state);
void test_buffer_returns_can_write_if_empty(UNUSED void** state);
