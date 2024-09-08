#include "cmocka_inc.h"

int test_log_setup(UNUSED void** state);
void test_log_info_writes_to_log_buffer(UNUSED void** state);
void test_log_warn_writes_to_log_buffer(UNUSED void** state);
void test_log_stores_two_logs(UNUSED void** state);
void test_log_stores_multiple_logs_and_overwrites_older(UNUSED void** state);
void test_log_returns_null_when_no_more_logs_are_available_to_be_dequeued(UNUSED void** state);
