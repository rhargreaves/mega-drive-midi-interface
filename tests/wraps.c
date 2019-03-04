#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "wraps.h"
#include <cmocka.h>

void __wrap_synth_init(void)
{
    function_called();
}

void __wrap_synth_enableLfo(u8 enable)
{
    check_expected(enable);
}

void __wrap_synth_globalLfoFrequency(u8 freq)
{
    check_expected(freq);
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

void __wrap_synth_totalLevel(u8 channel, u8 totalLevel)
{
    check_expected(channel);
    check_expected(totalLevel);
}

void __wrap_synth_stereo(u8 channel, u8 mode)
{
    check_expected(channel);
    check_expected(mode);
}

void __wrap_synth_algorithm(u8 channel, u8 algorithm)
{
    check_expected(channel);
    check_expected(algorithm);
}

void __wrap_synth_feedback(u8 channel, u8 feedback)
{
    check_expected(channel);
    check_expected(feedback);
}

void __wrap_synth_ams(u8 channel, u8 ams)
{
    check_expected(channel);
    check_expected(ams);
}

void __wrap_synth_fms(u8 channel, u8 fms)
{
    check_expected(channel);
    check_expected(fms);
}

void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(totalLevel);
}

void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple)
{
    check_expected(channel);
    check_expected(op);
    check_expected(multiple);
}

void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune)
{
    check_expected(channel);
    check_expected(op);
    check_expected(detune);
}

void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling)
{
    check_expected(channel);
    check_expected(op);
    check_expected(rateScaling);
}

void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(attackRate);
}

void __wrap_synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(firstDecayRate);
}

void __wrap_synth_operatorSecondDecayRate(u8 channel, u8 op, u8 secondDecayRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(secondDecayRate);
}

void __wrap_synth_operatorSecondaryAmplitude(
    u8 channel, u8 op, u8 secondaryAmplitude)
{
    check_expected(channel);
    check_expected(op);
    check_expected(secondaryAmplitude);
}

void __wrap_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation)
{
    check_expected(channel);
    check_expected(op);
    check_expected(amplitudeModulation);
}

void __wrap_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(releaseRate);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

u16 __wrap_comm_idleCount(void)
{
    return mock_type(u16);
}

u16 __wrap_comm_busyCount(void)
{
    return mock_type(u16);
}

void __wrap_comm_resetCounts(void)
{
}

void __wrap_midi_noteOff(u8 chan, u8 pitch)
{
    check_expected(chan);
    check_expected(pitch);
}

void __wrap_midi_channelVolume(u8 chan, u8 volume)
{
    check_expected(chan);
    check_expected(volume);
}

void __wrap_midi_pan(u8 chan, u8 pan)
{
    check_expected(chan);
    check_expected(pan);
}

void __wrap_midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    check_expected(chan);
    check_expected(pitch);
    check_expected(velocity);
}

void __wrap_midi_pitchBend(u8 chan, u16 bend)
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

void __wrap_midi_clock(void)
{
    function_called();
}

void __wrap_midi_start(void)
{
    function_called();
}

void __wrap_midi_position(u16 beat)
{
    check_expected(beat);
}

void __wrap_fm_writeReg(u16 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_psg_noteOn(u8 channel, u16 freq)
{
    check_expected(channel);
    check_expected(freq);
}

void __wrap_psg_noteOff(u8 channel)
{
    check_expected(channel);
}

void __wrap_psg_attenuation(u8 channel, u8 attenuation)
{
    check_expected(channel);
    check_expected(attenuation);
}

void __wrap_psg_frequency(u8 channel, u16 freq)
{
    check_expected(channel);
    check_expected(freq);
}

void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_VDP_drawText(const char* str, u16 x, u16 y)
{
}

void __wrap_SYS_setVIntCallback(_voidCallback* CB)
{
}

void __wrap_VDP_setTextPalette(u16 palette)
{
}

void __wrap_VDP_clearText(u16 x, u16 y, u16 w)
{
}

void __wrap_PSG_setEnvelope(u8 channel, u8 value)
{
    check_expected(channel);
    check_expected(value);
}

void __wrap_PSG_setFrequency(u8 channel, u16 value)
{
    check_expected(channel);
    check_expected(value);
}

Sprite* __wrap_SPR_addSprite(
    const SpriteDefinition* spriteDef, s16 x, s16 y, u16 attribut)
{
    return mock_type(Sprite*);
}

void __wrap_SPR_update()
{
}

void __wrap_SYS_disableInts()
{
}

void __wrap_SYS_enableInts()
{
}

void __wrap_SPR_init(u16 maxSprite, u16 vramSize, u16 unpackBufferSize)
{
}

void __wrap_VDP_setPaletteColors(u16 index, const u16* values, u16 count)
{
}

u8 __wrap_ssf_usb_read(void)
{
    return mock_type(u8);
}

u16 __wrap_ssf_usb_rd_ready(void)
{
    return mock_type(u16);
}
