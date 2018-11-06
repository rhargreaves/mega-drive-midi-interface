#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <wraps.h>

void __wrap_synth_init(void)
{
    function_called();
}

void __wrap_synth_noteOn(u8 channel)
{
    check_expected(channel);
}

void __wrap_synth_noteOff(u8 channel)
{
    check_expected(channel);
}

void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    check_expected(channel);
    check_expected(octave);
    check_expected(freqNumber);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

void __wrap_midi_noteOff(u8 chan)
{
    check_expected(chan);
}

void __wrap_midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    check_expected(chan);
    check_expected(pitch);
    check_expected(velocity);
}

void __wrap_fm_writeReg(u16 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data)
{
}
