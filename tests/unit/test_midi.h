#pragma once
#include "cmocka_inc.h"
#include "debug.h"
#include "midi.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "synth.h"
#include "pitchcents.h"
#include "mocks/mock_midi.h"
#include "mocks/mock_synth.h"
#include "mocks/mock_log.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_psg.h"

#define MIDI_PITCH_A2 45
#define MIDI_PITCH_C3 48
#define MIDI_PITCH_C4 60
#define MIDI_PITCH_CS4 61
#define MIDI_PITCH_AS6 94
#define MIDI_PITCH_B6 95

#define TONE_NTSC_AS4 479
#define TONE_NTSC_C4 427
#define TONE_NTSC_CS4 403
#define TONE_NTSC_DS4 380
#define TONE_NTSC_A2 1016

#define SYNTH_NTSC_C 0x284
#define SYNTH_NTSC_AS 1146
#define SYNTH_PAL_C 649

#define SYSEX_DYNAMIC_AUTO 0x02
#define SYSEX_DYNAMIC_ENABLED 0x01
#define SYSEX_DYNAMIC_DISABLED 0x00
#define SYSEX_NON_GENERAL_MIDI_CCS_ENABLED 0x01
#define SYSEX_NON_GENERAL_MIDI_CCS_DISABLED 0x00

#define UNASSIGNED_MIDI_CHANNEL 0x7F

int test_midi_setup(UNUSED void** state);
void test_midi_polyphonic_mode_returns_state(UNUSED void** state);
void test_midi_sets_all_sound_off(UNUSED void** state);
void test_midi_sets_all_notes_off(UNUSED void** state);
void test_midi_sets_unknown_CC(UNUSED void** state);
void test_midi_shows_fm_parameter_ui(UNUSED void** state);
void test_midi_hides_fm_parameter_ui(UNUSED void** state);
void test_midi_resets_fm_values_to_defaults(UNUSED void** state);
void test_midi_resets_psg_values_to_defaults(UNUSED void** state);
void test_midi_ignores_sustain_pedal_cc(UNUSED void** state);
void test_midi_ignores_sysex_nrpn_ccs(UNUSED void** state);
void test_midi_ignores_expression_cc(UNUSED void** state);
