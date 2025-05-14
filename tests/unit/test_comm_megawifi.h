#include "cmocka_inc.h"

int test_comm_megawifi_setup(UNUSED void** state);
void test_comm_megawifi_returns_detecting_status_by_default(UNUSED void** state);
void test_comm_megawifi_inits_and_sets_status_to_listening(UNUSED void** state);
void test_comm_megawifi_reads_midi_message(UNUSED void** state);
void test_comm_megawifi_logs_if_buffer_full(UNUSED void** state);
void test_comm_megawifi_returns_zero_when_buffer_empty(UNUSED void** state);
void test_comm_megawifi_sets_status_to_not_detected_when_not_present(UNUSED void** state);