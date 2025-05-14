#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"
#include "midi.h"

void __wrap_midi_note_off(u8 chan, u8 pitch);
void __wrap_midi_note_on(u8 chan, u8 pitch, u8 velocity);
void __wrap_midi_channel_volume(u8 chan, u8 volume);
void __wrap_midi_pitch_bend(u8 chan, u16 bend);
void __wrap_midi_setPolyphonic(bool state);
bool __wrap_midi_getPolyphonic(void);
void __wrap_midi_cc(u8 chan, u8 controller, u8 value);
void __wrap_midi_program(u8 chan, u8 program);
void __wrap_midi_sysex(u8* data, u16 length);
bool __wrap_midi_dynamic_mode(void);
DeviceChannel* __wrap_midi_channel_mappings(void);
void __wrap_midi_psg_tick(u16 delta);
void __wrap_midi_psg_load_envelope(const u8* eef);
void __wrap_midi_reset(void);
extern void __real_midi_note_on(u8 chan, u8 pitch, u8 velocity);
extern void __real_midi_note_off(u8 chan, u8 pitch);
extern void __real_midi_pitch_bend(u8 chan, u16 bend);
extern void __real_midi_cc(u8 chan, u8 controller, u8 value);
extern void __real_midi_program(u8 chan, u8 program);
extern void __real_midi_sysex(const u8* data, u16 length);
extern bool __real_midi_dynamic_mode(void);
extern DeviceChannel* __real_midi_channel_mappings(void);
extern void __real_midi_psg_tick(void);
extern void __real_midi_psg_load_envelope(const u8* eef);
extern void __real_midi_reset(void);

void _expect_midi_emit(u8 value, const char* const file, const int line);
void _expect_midi_emit_duo(u8 value1, u8 value2, const char* const file, const int line);
void _expect_midi_emit_trio(
    u8 value1, u8 value2, u8 value3, const char* const file, const int line);
void _expect_midi_note_on(u8 chan, u8 pitch, u8 velocity, const char* const file, const int line);
void _expect_midi_note_off(u8 chan, u8 pitch, const char* const file, const int line);
void _expect_midi_cc(u8 chan, u8 controller, u8 value, const char* const file, const int line);
void _expect_midi_pitch_bend(u8 chan, u16 bend, const char* const file, const int line);
void _expect_midi_program(u8 chan, u8 program, const char* const file, const int line);

#define expect_midi_emit(value) _expect_midi_emit(value, __FILE__, __LINE__)
#define expect_midi_emit_duo(value1, value2)                                                       \
    _expect_midi_emit_duo(value1, value2, __FILE__, __LINE__)
#define expect_midi_emit_trio(value1, value2, value3)                                              \
    _expect_midi_emit_trio(value1, value2, value3, __FILE__, __LINE__)
#define expect_midi_note_on(chan, pitch, velocity)                                                 \
    _expect_midi_note_on(chan, pitch, velocity, __FILE__, __LINE__)
#define expect_midi_note_off(chan, pitch) _expect_midi_note_off(chan, pitch, __FILE__, __LINE__)
#define expect_midi_cc(chan, controller, value)                                                    \
    _expect_midi_cc(chan, controller, value, __FILE__, __LINE__)
#define expect_midi_pitch_bend(chan, bend) _expect_midi_pitch_bend(chan, bend, __FILE__, __LINE__)
#define expect_midi_program(chan, program) _expect_midi_program(chan, program, __FILE__, __LINE__)
