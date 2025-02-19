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
#include "test_helpers.h"
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
