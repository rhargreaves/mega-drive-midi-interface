#include "cmocka_inc.h"

int test_rtpmidi_setup(UNUSED void** state);
void test_rtpmidi_does_not_read_beyond_buffer_length(UNUSED void** state);
void test_rtpmidi_returns_when_header_too_short(UNUSED void** state);
void test_rtpmidi_updates_applemidi_last_sequence_number(UNUSED void** state);
void test_rtpmidi_parses_packet_with_single_midi_event(UNUSED void** state);
void test_rtpmidi_parses_packet_with_single_2_byte_midi_event(UNUSED void** state);
void test_rtpmidi_parses_packet_with_multiple_2_byte_midi_events(UNUSED void** state);
void test_rtpmidi_parses_packet_with_single_midi_event_long_header(UNUSED void** state);
void test_rtpmidi_parses_packet_with_two_midi_events(UNUSED void** state);
void test_rtpmidi_parses_packet_with_multiple_midi_events(UNUSED void** state);
void test_rtpmidi_parses_packet_with_sysex(UNUSED void** state);
void test_rtpmidi_parses_packet_with_sysex_ending_with_F0(UNUSED void** state);
void test_rtpmidi_parses_packet_with_sysex_with_0xF7_at_end(UNUSED void** state);
void test_rtpmidi_parses_packet_with_multiple_different_midi_events(UNUSED void** state);
void test_rtpmidi_parses_packet_with_system_reset(UNUSED void** state);
void test_rtpmidi_parses_notes_sysex_cc_in_one_packet(UNUSED void** state);
void test_rtpmidi_ignores_middle_sysex_segments(UNUSED void** state);
void test_rtpmidi_processes_multiple_sysex_segments(UNUSED void** state);
void test_rtpmidi_processes_ccs(UNUSED void** state);
void test_rtpmidi_sets_last_sequence_number(UNUSED void** state);
void test_rtpmidi_does_not_read_beyond_length(UNUSED void** state);
void test_rtpmidi_pack_packet_with_short_header(UNUSED void** state);
void test_rtpmidi_pack_packet_with_long_header(UNUSED void** state);
void test_rtpmidi_pack_packet_returns_zero_when_buffer_too_small(UNUSED void** state);
void test_rtpmidi_pack_packet_sequence_starts_at_zero(UNUSED void** state);
