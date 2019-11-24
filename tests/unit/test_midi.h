#pragma once
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "midi.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "psg_chip.h"
#include "synth.h"
#include "unused.h"
#include <cmocka.h>

#define A_SHARP 94
#define B 95

extern void __real_midi_noteOn(u8 chan, u8 pitch, u8 velocity);
extern void __real_midi_noteOff(u8 chan, u8 pitch);
extern void __real_midi_pitchBend(u8 chan, u16 bend);
extern bool __real_midi_getPolyphonic(void);
extern void __real_midi_cc(u8 chan, u8 controller, u8 value);
extern void __real_midi_clock(void);
extern void __real_midi_start(void);
extern void __real_midi_position(u16 beat);
extern void __real_midi_program(u8 chan, u8 program);
extern Timing* __real_midi_timing(void);
extern void __real_midi_sysex(const u8* data, u16 length);
extern void __real_midi_mappings(u8* mappingDest);
extern bool __real_midi_dynamicMode(void);

int test_midi_setup(UNUSED void** state);
void test_midi_directs_channels_above_10_to_psg(UNUSED void** state);
void test_midi_polyphonic_mode_returns_state(UNUSED void** state);
void test_midi_sets_all_notes_off(UNUSED void** state);
void test_midi_sets_unknown_CC(UNUSED void** state);
void test_midi_exposes_channel_mappings(UNUSED void** state);
