#include "cmocka_inc.h"

int test_comm_megawifi_setup(UNUSED void** state);
void test_comm_megawifi_reads_midi_message(UNUSED void** state);
void test_comm_megawifi_logs_if_buffer_full(UNUSED void** state);
void test_comm_megawifi_returns_zero_when_buffer_empty(UNUSED void** state);
void test_comm_megawifi_write_sends_sysex_over_udp_reuse_send(UNUSED void** state);
void test_comm_megawifi_send_logs_and_drops_when_tx_buffer_overflows(UNUSED void** state);