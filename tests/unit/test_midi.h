#pragma once
#include "cmocka_inc.h"

#include "midi.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "synth.h"

#define MIDI_PITCH_A2 45
#define MIDI_PITCH_C4 60
#define MIDI_PITCH_CS4 61
#define MIDI_PITCH_AS6 94
#define MIDI_PITCH_B6 95

#define TONE_NTSC_AS4 479
#define TONE_NTSC_C4 427
#define TONE_NTSC_CS4 403
#define TONE_NTSC_DS4 380
#define TONE_NTSC_A2 1016

#define SYNTH_NTSC_C 653
#define SYNTH_NTSC_AS 1164
#define SYNTH_PAL_C 649

#define SYSEX_EXTENDED_MANU_ID_SECTION 0x00
#define SYSEX_UNUSED_EUROPEAN_SECTION 0x22
#define SYSEX_UNUSED_MANU_ID 0x77
#define SYSEX_DYNAMIC_ENABLED 0x01
#define SYSEX_DYNAMIC_DISABLED 0x00
#define SYSEX_NON_GENERAL_MIDI_CCS_ENABLED 0x01
#define SYSEX_NON_GENERAL_MIDI_CCS_DISABLED 0x00

extern void __real_midi_noteOn(u8 chan, u8 pitch, u8 velocity);
extern void __real_midi_noteOff(u8 chan, u8 pitch);
extern void __real_midi_pitchBend(u8 chan, u16 bend);
extern void __real_midi_cc(u8 chan, u8 controller, u8 value);
extern void __real_midi_program(u8 chan, u8 program);
extern void __real_midi_sysex(const u8* data, u16 length);
extern bool __real_midi_dynamicMode(void);
extern DeviceChannel* __real_midi_channelMappings(void);
extern void __real_midi_psg_tick(void);
extern void __real_midi_psg_loadEnvelope(const u8* eef);
extern void __real_midi_reset(void);

int test_midi_setup(UNUSED void** state);
void test_midi_polyphonic_mode_returns_state(UNUSED void** state);
void test_midi_sets_all_sound_off(UNUSED void** state);
void test_midi_sets_all_notes_off(UNUSED void** state);
void test_midi_sets_unknown_CC(UNUSED void** state);
void test_midi_shows_fm_parameter_ui(UNUSED void** state);
void test_midi_hides_fm_parameter_ui(UNUSED void** state);
void test_midi_reset_reinitialises_module(UNUSED void** state);
