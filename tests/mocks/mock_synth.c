#include "cmocka_inc.h"
#include "mocks/mock_synth.h"

static bool disableChecks = false;

void mock_synth_disable_checks(void)
{
    disableChecks = true;
}

void mock_synth_enable_checks(void)
{
    disableChecks = false;
}

void __wrap_synth_init(const FmChannel* defaultPreset)
{
    if (disableChecks)
        return;
    check_expected(defaultPreset);
}

void __wrap_synth_enable_lfo(u8 enable)
{
    check_expected(enable);
}

void __wrap_synth_global_lfo_frequency(u8 freq)
{
    check_expected(freq);
}

void __wrap_synth_note_on(u8 channel)
{
    debug_message("call: synth_note_on(%d)\n", channel);
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_note_off(u8 channel)
{
    debug_message("call: synth_note_off(%d)\n", channel);
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    debug_message("call: synth_pitch(%d, %d, %d)\n", channel, octave, freqNumber);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_total_level(u8 channel, u8 totalLevel)
{
    check_expected(channel);
    check_expected(totalLevel);
}

void __wrap_synth_stereo(u8 channel, u8 mode)
{
    if (disableChecks)
        return;
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

void __wrap_synth_operator_total_level(u8 channel, u8 op, u8 totalLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(totalLevel);
}

void __wrap_synth_operator_multiple(u8 channel, u8 op, u8 multiple)
{
    check_expected(channel);
    check_expected(op);
    check_expected(multiple);
}

void __wrap_synth_operator_detune(u8 channel, u8 op, u8 detune)
{
    check_expected(channel);
    check_expected(op);
    check_expected(detune);
}

void __wrap_synth_operator_rate_scaling(u8 channel, u8 op, u8 rateScaling)
{
    check_expected(channel);
    check_expected(op);
    check_expected(rateScaling);
}

void __wrap_synth_operator_attack_rate(u8 channel, u8 op, u8 attackRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(attackRate);
}

void __wrap_synth_operator_decay_rate(u8 channel, u8 op, u8 decayRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(decayRate);
}

void __wrap_synth_operator_sustain_rate(u8 channel, u8 op, u8 sustainRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainRate);
}

void __wrap_synth_operator_sustain_level(u8 channel, u8 op, u8 sustainLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainLevel);
}

void __wrap_synth_operator_amplitude_modulation(u8 channel, u8 op, u8 amplitudeModulation)
{
    check_expected(channel);
    check_expected(op);
    check_expected(amplitudeModulation);
}

void __wrap_synth_operator_release_rate(u8 channel, u8 op, u8 releaseRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(releaseRate);
}

void __wrap_synth_operator_ssg_eg(u8 channel, u8 op, u8 ssgEg)
{
    check_expected(channel);
    check_expected(op);
    check_expected(ssgEg);
}

void __wrap_synth_preset(u8 channel, const FmChannel* preset)
{
    debug_message("call: synth_preset(%d, %p)\n", channel, preset);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(preset);
}

void __wrap_synth_volume(u8 channel, u8 volume)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(volume);
}

const FmChannel* __wrap_synth_channel_parameters(u8 channel)
{
    return NULL;
}

const Global* __wrap_synth_global_parameters()
{
    return NULL;
}

void __wrap_synth_set_special_mode(bool enable)
{
    check_expected(enable);
}

void __wrap_synth_special_mode_pitch(u8 op, u8 octave, u16 freqNumber)
{
    check_expected(op);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_special_mode_volume(u8 op, u8 volume)
{
    check_expected(op);
    check_expected(volume);
}

void __wrap_synth_direct_write_ym2612(u8 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_synth_enable_dac(bool enable)
{
    check_expected(enable);
}

static bool operator_equality_check(Operator* l, Operator* r)
{
    return ((l->amplitudeModulation == r->amplitudeModulation) && (l->attackRate == r->attackRate)
        && (l->decayRate == r->decayRate) && (l->detune == r->detune)
        && (l->multiple == r->multiple) && (l->rateScaling == r->rateScaling)
        && (l->releaseRate == r->releaseRate) && (l->ssgEg == r->ssgEg)
        && (l->sustainLevel == r->sustainLevel) && (l->sustainRate == r->sustainRate)
        && (l->totalLevel == r->totalLevel));
}

void _expect_synth_pitch_any(const char* const file, const int line)
{
    debug_message("expect: synth_pitch(channel=*, octave=*, freqNumber=*)\n");
    expect_any_with_pos(__wrap_synth_pitch, channel, file, line);
    expect_any_with_pos(__wrap_synth_pitch, octave, file, line);
    expect_any_with_pos(__wrap_synth_pitch, freqNumber, file, line);
}

void _expect_synth_pitch(
    u8 channel, u8 octave, u16 freqNumber, const char* const file, const int line)
{
    debug_message(
        "expect: synth_pitch(channel=%d, octave=%d, freqNumber=%d)\n", channel, octave, freqNumber);
    expect_value_with_pos(__wrap_synth_pitch, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_pitch, octave, octave, file, line);
    expect_value_with_pos(__wrap_synth_pitch, freqNumber, freqNumber, file, line);
}

void _expect_synth_note_on(u8 channel, const char* const file, const int line)
{
    debug_message("expect: synth_note_on(%d)\n", channel);
    expect_value_with_pos(__wrap_synth_note_on, channel, channel, file, line);
}

void _expect_synth_note_off(u8 channel, const char* const file, const int line)
{
    debug_message("expect: synth_note_off(%d)\n", channel);
    expect_value_with_pos(__wrap_synth_note_off, channel, channel, file, line);
}

int fmchannel_equality_check(
    const LargestIntegralType value, const LargestIntegralType check_value_data)
{
    FmChannel* expected = (FmChannel*)value;
    FmChannel* actual = (FmChannel*)check_value_data;

    if ((actual->algorithm == expected->algorithm) && (actual->ams == expected->ams)
        && (actual->feedback == expected->feedback) && (actual->fms == expected->fms)
        && (actual->freqNumber == expected->freqNumber) && (actual->octave == expected->octave)
        && (actual->stereo == expected->stereo)
        && operator_equality_check(&actual->operators[0], &expected->operators[0])
        && operator_equality_check(&actual->operators[1], &expected->operators[1])
        && operator_equality_check(&actual->operators[2], &expected->operators[2])
        && operator_equality_check(&actual->operators[3], &expected->operators[3])) {
        return 1;
    }

    return 0;
}

void _expect_synth_algorithm(u8 channel, u8 algorithm, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_algorithm, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_algorithm, algorithm, algorithm, file, line);
}

void _expect_synth_volume(u8 channel, u8 volume, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_volume, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_volume, volume, volume, file, line);
}

void _expect_synth_operator_total_level(
    u8 channel, u8 op, u8 totalLevel, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_total_level, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_total_level, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_total_level, totalLevel, totalLevel, file, line);
}

void _expect_synth_operator_multiple(
    u8 channel, u8 op, u8 multiple, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_multiple, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_multiple, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_multiple, multiple, multiple, file, line);
}

void _expect_synth_operator_detune(
    u8 channel, u8 op, u8 detune, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_detune, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_detune, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_detune, detune, detune, file, line);
}

void _expect_synth_operator_rate_scaling(
    u8 channel, u8 op, u8 rateScaling, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_rate_scaling, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_rate_scaling, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_rate_scaling, rateScaling, rateScaling, file, line);
}

void _expect_synth_operator_attack_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_attack_rate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_attack_rate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_attack_rate, attackRate, rate, file, line);
}

void _expect_synth_operator_decay_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_decay_rate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_decay_rate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_decay_rate, decayRate, rate, file, line);
}

void _expect_synth_operator_sustain_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_sustain_rate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_sustain_rate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_sustain_rate, sustainRate, rate, file, line);
}

void _expect_synth_operator_sustain_level(
    u8 channel, u8 op, u8 level, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_sustain_level, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_sustain_level, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_sustain_level, sustainLevel, level, file, line);
}

void _expect_synth_operator_release_rate(
    u8 channel, u8 op, u8 rate, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_release_rate, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_release_rate, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_release_rate, releaseRate, rate, file, line);
}

void _expect_synth_operator_ssg_eg(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_ssg_eg, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_ssg_eg, op, op, file, line);
    expect_value_with_pos(__wrap_synth_operator_ssg_eg, ssgEg, enabled, file, line);
}

void _expect_synth_operator_amplitude_modulation(
    u8 channel, u8 op, u8 enabled, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_operator_amplitude_modulation, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_operator_amplitude_modulation, op, op, file, line);
    expect_value_with_pos(
        __wrap_synth_operator_amplitude_modulation, amplitudeModulation, enabled, file, line);
}

void _expect_synth_ams(u8 channel, u8 ams, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_ams, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_ams, ams, ams, file, line);
}

void _expect_synth_fms(u8 channel, u8 fms, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_fms, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_fms, fms, fms, file, line);
}

void _expect_synth_stereo(u8 channel, u8 mode, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_stereo, channel, channel, file, line);
    expect_value_with_pos(__wrap_synth_stereo, mode, mode, file, line);
}

void _expect_synth_enable_lfo(u8 enable, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_enable_lfo, enable, enable, file, line);
}

void _expect_synth_global_lfo_frequency(u8 freq, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_global_lfo_frequency, freq, freq, file, line);
}

void _expect_synth_special_mode_pitch(
    u8 op, u8 octave, u16 freqNumber, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_special_mode_pitch, op, op, file, line);
    expect_value_with_pos(__wrap_synth_special_mode_pitch, octave, octave, file, line);
    expect_value_with_pos(__wrap_synth_special_mode_pitch, freqNumber, freqNumber, file, line);
}

void _expect_synth_special_mode_volume(u8 op, u8 volume, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_special_mode_volume, op, op, file, line);
    expect_value_with_pos(__wrap_synth_special_mode_volume, volume, volume, file, line);
}

void _expect_synth_set_special_mode(bool enable, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_set_special_mode, enable, enable, file, line);
}

void _expect_synth_direct_write_ym2612(
    u8 part, u8 reg, u8 data, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_direct_write_ym2612, part, part, file, line);
    expect_value_with_pos(__wrap_synth_direct_write_ym2612, reg, reg, file, line);
    expect_value_with_pos(__wrap_synth_direct_write_ym2612, data, data, file, line);
}

void _expect_synth_preset(
    u8 channel, const FmChannel* preset, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_synth_preset, channel, channel, file, line);
    expect_check_with_pos(
        __wrap_synth_preset, preset, fmchannel_equality_check, preset, file, line);
}
