#include "cmocka_inc.h"

int test_comm_setup(UNUSED void** state);
void test_comm_reads_from_serial_when_ready(UNUSED void** state);
void test_comm_reads_everdrive_when_ready(UNUSED void** state);
void test_comm_reads_demo_when_ready(UNUSED void** state);
void test_comm_writes_when_ready(UNUSED void** state);
void test_comm_idle_count_is_correct(UNUSED void** state);
void test_comm_busy_count_is_correct(UNUSED void** state);
void test_comm_clamps_idle_count(UNUSED void** state);
void test_comm_clamps_busy_count(UNUSED void** state);
void test_comm_read_with_data_ready_immediately(UNUSED void** state);
void test_comm_read_with_data_ready_before_timeout(UNUSED void** state);
void test_comm_read_with_timeout_expired(UNUSED void** state);
void test_comm_read_non_blocking_no_data(UNUSED void** state);
