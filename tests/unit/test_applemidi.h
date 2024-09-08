#include "cmocka_inc.h"
#include "comm/applemidi.h"

int test_applemidi_setup(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_single_midi_event(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_single_2_byte_midi_event(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_multiple_2_byte_midi_events(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_single_midi_event_long_header(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_two_midi_events(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_multiple_midi_events(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_sysex(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_sysex_ending_with_F0(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_sysex_with_0xF7_at_end(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_multiple_different_midi_events(UNUSED void** state);
void test_applemidi_parses_rtpmidi_packet_with_system_reset(UNUSED void** state);
void test_applemidi_parses_notes_sysex_cc_in_one_packet(UNUSED void** state);
void test_applemidi_ignores_middle_sysex_segments(UNUSED void** state);
void test_applemidi_processes_multiple_sysex_segments(UNUSED void** state);
void test_applemidi_processes_ccs(UNUSED void** state);
void test_applemidi_sets_last_sequence_number(UNUSED void** state);
void test_applemidi_sends_receiver_feedback(UNUSED void** state);
void test_applemidi_does_not_read_beyond_length(UNUSED void** state);
