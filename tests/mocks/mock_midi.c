#include "cmocka_inc.h"
#include "asserts.h"
#include "mocks/mock_midi.h"

void __wrap_midi_note_off(u8 chan, u8 pitch)
{
    check_expected(chan);
    check_expected(pitch);
}

void __wrap_midi_channel_volume(u8 chan, u8 volume)
{
    check_expected(chan);
    check_expected(volume);
}

void __wrap_midi_pan(u8 chan, u8 pan)
{
    check_expected(chan);
    check_expected(pan);
}

void __wrap_midi_note_on(u8 chan, u8 pitch, u8 velocity)
{
    check_expected(chan);
    check_expected(pitch);
    check_expected(velocity);
}

void __wrap_midi_pitch_bend(u8 chan, u16 bend)
{
    check_expected(chan);
    check_expected(bend);
}

void __wrap_midi_setPolyphonic(bool state)
{
    check_expected(state);
}

bool __wrap_midi_getPolyphonic(void)
{
    return mock_type(bool);
}

void __wrap_midi_cc(u8 chan, u8 controller, u8 value)
{
    check_expected(chan);
    check_expected(controller);
    check_expected(value);
}

void __wrap_midi_program(u8 chan, u8 program)
{
    check_expected(chan);
    check_expected(program);
}

void __wrap_midi_sysex(u8* data, u16 length)
{
    check_expected(data);
    check_expected(length);
}

void __wrap_midi_mappings(u8* mappingDest)
{
    check_expected(mappingDest);
}

bool __wrap_midi_dynamic_mode(void)
{
    return mock_type(bool);
}

DeviceChannel* __wrap_midi_channel_mappings(void)
{
    return mock_type(DeviceChannel*);
}

void __wrap_midi_reset(void)
{
    function_called();
}

void __wrap_midi_psg_tick(void)
{
    function_called();
}

void __wrap_midi_psg_load_envelope(const u8* eef)
{
    check_expected_ptr(eef);
}

void _expect_midi_emit(u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_comm_megawifi_midiEmitCallback, midiByte, value, file, line);
}

void _expect_midi_emit_duo(u8 value1, u8 value2, const char* const file, const int line)
{
    _expect_midi_emit(value1, file, line);
    _expect_midi_emit(value2, file, line);
}

void _expect_midi_emit_trio(u8 value1, u8 value2, u8 value3, const char* const file, const int line)
{
    _expect_midi_emit(value1, file, line);
    _expect_midi_emit(value2, file, line);
    _expect_midi_emit(value3, file, line);
}

void _expect_midi_note_on(u8 chan, u8 pitch, u8 velocity, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_note_on, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_note_on, pitch, pitch, file, line);
    expect_value_with_pos(__wrap_midi_note_on, velocity, velocity, file, line);
}

void _expect_midi_note_off(u8 chan, u8 pitch, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_note_off, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_note_off, pitch, pitch, file, line);
}

void _expect_midi_cc(u8 chan, u8 controller, u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_cc, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_cc, controller, controller, file, line);
    expect_value_with_pos(__wrap_midi_cc, value, value, file, line);
}

void _expect_midi_pitch_bend(u8 chan, u16 bend, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_pitch_bend, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_pitch_bend, bend, bend, file, line);
}

void _expect_midi_program(u8 chan, u8 program, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_program, chan, chan, file, line);
    expect_value_with_pos(__wrap_midi_program, program, program, file, line);
}
