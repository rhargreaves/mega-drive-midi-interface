#include "synth.h"
#include "utils.h"
#include "ym2612_regs.h"

static Global global;
static FmChannel fmChannels[MAX_FM_CHANS];
static u8 noteOn;
static u8 volumes[MAX_FM_CHANS];

static ParameterUpdatedCallback* parameterUpdatedCallback = NULL;

static const u8 MAX_VOLUME = 0x7F;

static const u8 VOLUME_TO_TOTAL_LEVELS[] = { 127, 122, 117, 113, 108, 104, 100, 97, 93, 89, 86, 83,
    80, 77, 74, 71, 68, 66, 63, 61, 58, 56, 54, 52, 50, 48, 46, 44, 43, 41, 40, 38, 37, 35, 34, 32,
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12,
    12, 11, 11, 11, 10, 10, 9, 9, 9, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3,
    3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0 };

static void init_global(void);
static void update_channel(u8 chan);
static void write_global_lfo(void);
static void write_dac_reg(void);
static void write_algorithm_and_feedback(u8 channel);
static void write_operator_multiple_and_detune(u8 channel, u8 operator);
static void write_operator_rate_scaling_and_attack_rate(u8 channel, u8 operator);
static void write_operator_amplitude_modulation_and_decay_rate(u8 channel, u8 operator);
static void write_operator_release_rate_and_sustain_level(u8 channel, u8 operator);
static void write_operator_total_level(u8 channel, u8 operator);
static void write_operator_sustain_rate(u8 channel, u8 operator);
static void write_operator_ssg_eg(u8 channel, u8 operator);
static void write_stereo_ams_fms(u8 channel);
static void write_channel_reg(u8 channel, u8 baseReg, u8 data);
static void write_operator_reg(u8 channel, u8 op, u8 baseReg, u8 data);
static void write_octave_and_frequency(u8 channel);
static void write_special_mode_reg(void);
static u8 key_on_off_reg_offset(u8 channel);
static Operator* get_operator(u8 channel, u8 operator);
static u8 effective_total_level(u8 channel, u8 operator, u8 totalLevel);
static bool is_output_operator(u8 algorithm, u8 op);
static u8 volume_adjusted_total_level(u8 channel, u8 totalLevel);
static void channel_parameter_updated(u8 channel);
static void other_parameter_updated(u8 channel, ParameterUpdated parameterUpdated);
static void write_reg_safe(u8 part, u8 reg, u8 data);
static void release_z80_bus(void);

static void preset_to_channel(FmChannel* channel, const FmPreset* preset)
{
    channel->algorithm = preset->algorithm;
    channel->feedback = preset->feedback;
    channel->ams = preset->ams;
    channel->fms = preset->fms;
    channel->stereo = STEREO_MODE_CENTRE;
    channel->octave = 0;
    channel->freqNumber = 0;
    for (u8 i = 0; i < MAX_FM_OPERATORS; i++) {
        channel->operators[i] = preset->operators[i];
    }
}

void synth_init(const FmPreset* initialPreset)
{
    init_global();
    Z80_loadDriver(Z80_DRIVER_PCM, true);
    Z80_requestBus(TRUE);
    write_dac_reg();
    write_special_mode_reg();
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        volumes[chan] = MAX_VOLUME;
        synth_note_off(chan);

        FmChannel channel;
        preset_to_channel(&channel, initialPreset);
        memcpy(&fmChannels[chan], &channel, sizeof(FmChannel));
        update_channel(chan);
    }
    write_global_lfo();
    release_z80_bus();
}

static void update_channel(u8 chan)
{
    write_algorithm_and_feedback(chan);
    write_stereo_ams_fms(chan);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        write_operator_multiple_and_detune(chan, op);
        write_operator_rate_scaling_and_attack_rate(chan, op);
        write_operator_amplitude_modulation_and_decay_rate(chan, op);
        write_operator_sustain_rate(chan, op);
        write_operator_release_rate_and_sustain_level(chan, op);
        write_operator_total_level(chan, op);
        write_operator_ssg_eg(chan, op);
    }
}

static void init_global(void)
{
    global.lfoEnable = 1;
    global.lfoFrequency = 0;
    global.specialMode = false;
    global.dacEnable = false;
}

void synth_note_on(u8 channel)
{
    write_reg_safe(0, YM_KEY_ON_OFF, 0xF0 + key_on_off_reg_offset(channel));
    SET_BIT(noteOn, channel);
}

void synth_note_off(u8 channel)
{
    write_reg_safe(0, YM_KEY_ON_OFF, key_on_off_reg_offset(channel));
    CLEAR_BIT(noteOn, channel);
}

void synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    FmChannel* chan = &fmChannels[channel];
    chan->octave = octave;
    chan->freqNumber = freqNumber;
    write_octave_and_frequency(channel);
}

void synth_volume(u8 channel, u8 volume)
{
    if (volumes[channel] == volume) {
        return;
    }
    volumes[channel] = volume;
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        write_operator_total_level(channel, op);
    }
}

void synth_stereo(u8 channel, u8 stereo)
{
    fmChannels[channel].stereo = stereo;
    write_stereo_ams_fms(channel);
    channel_parameter_updated(channel);
}

void synth_algorithm(u8 channel, u8 algorithm)
{
    fmChannels[channel].algorithm = algorithm;
    write_algorithm_and_feedback(channel);
    channel_parameter_updated(channel);
}

void synth_feedback(u8 channel, u8 feedback)
{
    fmChannels[channel].feedback = feedback;
    write_algorithm_and_feedback(channel);
    channel_parameter_updated(channel);
}

void synth_operator_total_level(u8 channel, u8 op, u8 totalLevel)
{
    Operator* oper = get_operator(channel, op);
    if (oper->totalLevel == totalLevel) {
        return;
    }
    oper->totalLevel = totalLevel;
    write_operator_total_level(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_multiple(u8 channel, u8 op, u8 multiple)
{
    get_operator(channel, op)->multiple = multiple;
    write_operator_multiple_and_detune(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_detune(u8 channel, u8 op, u8 detune)
{
    get_operator(channel, op)->detune = detune;
    write_operator_multiple_and_detune(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_rate_scaling(u8 channel, u8 op, u8 rateScaling)
{
    get_operator(channel, op)->rateScaling = rateScaling;
    write_operator_rate_scaling_and_attack_rate(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_attack_rate(u8 channel, u8 op, u8 attackRate)
{
    get_operator(channel, op)->attackRate = attackRate;
    write_operator_rate_scaling_and_attack_rate(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_sustain_rate(u8 channel, u8 op, u8 sustainRate)
{
    get_operator(channel, op)->sustainRate = sustainRate;
    write_operator_sustain_rate(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_release_rate(u8 channel, u8 op, u8 releaseRate)
{
    get_operator(channel, op)->releaseRate = releaseRate;
    write_operator_release_rate_and_sustain_level(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_ssg_eg(u8 channel, u8 op, u8 ssgEg)
{
    get_operator(channel, op)->ssgEg = ssgEg;
    write_operator_ssg_eg(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_sustain_level(u8 channel, u8 op, u8 sustainLevel)
{
    get_operator(channel, op)->sustainLevel = sustainLevel;
    write_operator_release_rate_and_sustain_level(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_decay_rate(u8 channel, u8 op, u8 decayRate)
{
    get_operator(channel, op)->decayRate = decayRate;
    write_operator_amplitude_modulation_and_decay_rate(channel, op);
    channel_parameter_updated(channel);
}

void synth_operator_amplitude_modulation(u8 channel, u8 op, u8 amplitudeModulation)
{
    get_operator(channel, op)->amplitudeModulation = amplitudeModulation;
    write_operator_amplitude_modulation_and_decay_rate(channel, op);
    channel_parameter_updated(channel);
}

void synth_enable_lfo(u8 enable)
{
    global.lfoEnable = enable;
    write_global_lfo();
    other_parameter_updated(0, Lfo);
}

void synth_global_lfo_frequency(u8 freq)
{
    global.lfoFrequency = freq;
    write_global_lfo();
    other_parameter_updated(0, Lfo);
}

void synth_ams(u8 channel, u8 ams)
{
    fmChannels[channel].ams = ams;
    write_stereo_ams_fms(channel);
    channel_parameter_updated(channel);
}

void synth_fms(u8 channel, u8 fms)
{
    fmChannels[channel].fms = fms;
    write_stereo_ams_fms(channel);
    channel_parameter_updated(channel);
}

u8 synth_busy(void)
{
    return noteOn;
}

void synth_preset(u8 channel, const FmPreset* preset)
{
    FmChannel* chan = &fmChannels[channel];
    u8 currentStereo = chan->stereo;
    preset_to_channel(chan, preset);
    chan->stereo = currentStereo;
    update_channel(channel);
    channel_parameter_updated(channel);
}

static void other_parameter_updated(u8 channel, ParameterUpdated parameterUpdated)
{
    if (parameterUpdatedCallback) {
        parameterUpdatedCallback(channel, parameterUpdated);
    }
}

static void channel_parameter_updated(u8 channel)
{
    if (parameterUpdatedCallback) {
        parameterUpdatedCallback(channel, Channel);
    }
}

static void write_channel_reg(u8 channel, u8 baseReg, u8 data)
{
    u8 part = channel > 2 ? 1 : 0;
    u8 reg = baseReg + (channel % 3);
    write_reg_safe(part, reg, data);
}

static u8 reg_operator_index(u8 op)
{
    if (op == 1)
        return 2;
    else if (op == 2)
        return 1;
    else {
        return op;
    }
}

static void write_operator_reg(u8 channel, u8 op, u8 baseReg, u8 data)
{
    write_channel_reg(channel, baseReg + (reg_operator_index(op) * 4), data);
}

static u8 key_on_off_reg_offset(u8 channel)
{
    return (channel < 3) ? channel : (channel + 1);
}

static Operator* get_operator(u8 channel, u8 operator)
{
    return &fmChannels[channel].operators[operator];
}

static void write_global_lfo(void)
{
    write_reg_safe(0, YM_LFO_ENABLE, (global.lfoEnable << 3) | global.lfoFrequency);
}

static void write_dac_reg(void)
{
    write_reg_safe(0, YM_DAC_ENABLE, global.dacEnable ? 0x80 : 0);
}

static void write_octave_and_frequency(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    write_channel_reg(channel, YM_BASE_FREQ_MSB_BLK, (chan->freqNumber >> 8) | (chan->octave << 3));
    write_channel_reg(channel, YM_BASE_FREQ_LSB, chan->freqNumber);
}

static void write_algorithm_and_feedback(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    write_channel_reg(channel, YM_BASE_ALGORITHM_FEEDBACK, (chan->feedback << 3) + chan->algorithm);
}

static void write_stereo_ams_fms(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    write_channel_reg(
        channel, YM_BASE_STEREO_AMS_PMS, (chan->stereo << 6) + (chan->ams << 4) + chan->fms);
}

static void write_operator_multiple_and_detune(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(
        channel, operator, YM_BASE_MULTIPLE_DETUNE, op->multiple + (op->detune << 4));
}

static void write_operator_rate_scaling_and_attack_rate(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_ATTACK_RATE_SCALING_RATE,
        op->attackRate + (op->rateScaling << 6));
}

static void write_operator_amplitude_modulation_and_decay_rate(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_DECAY_RATE_AM_ENABLE,
        op->decayRate + (op->amplitudeModulation << 7));
}

static void write_operator_sustain_rate(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_SUSTAIN_RATE, op->sustainRate);
}

static void write_operator_release_rate_and_sustain_level(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL,
        op->releaseRate + (op->sustainLevel << 4));
}

static void write_operator_ssg_eg(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_SSG_EG, op->ssgEg);
}

static void write_operator_total_level(u8 channel, u8 operator)
{
    Operator* op = get_operator(channel, operator);
    write_operator_reg(channel, operator, YM_BASE_TOTAL_LEVEL,
        effective_total_level(channel, operator, op->totalLevel));
}

static u8 effective_total_level(u8 channel, u8 operator, u8 totalLevel)
{
    return is_output_operator(fmChannels[channel].algorithm, operator)
        ? volume_adjusted_total_level(channel, totalLevel)
        : totalLevel;
}

static bool is_output_operator(u8 algorithm, u8 op)
{
    return (algorithm < 4 && op == 3) || (algorithm == 4 && (op == 1 || op == 3))
        || ((algorithm == 5 || algorithm == 6) && op > 0) || algorithm == 7;
}

static u8 volume_adjusted_total_level(u8 channel, u8 totalLevel)
{
    u8 volume = volumes[channel];
    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - totalLevel;
    u8 inverseNewTotalLevel = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    return 0x7F - inverseNewTotalLevel;
}

const FmChannel* synth_channel_parameters(u8 channel)
{
    return &fmChannels[channel];
}

const Global* synth_global_parameters()
{
    return &global;
}

void synth_set_parameter_update_callback(ParameterUpdatedCallback* cb)
{
    parameterUpdatedCallback = cb;
}

static void write_special_mode_reg(void)
{
    write_reg_safe(0, YM_CH3_MODE, global.specialMode << 6);
}

void synth_set_special_mode(bool enable)
{
    global.specialMode = enable;
    write_special_mode_reg();
    other_parameter_updated(0, SpecialMode);
}

void synth_special_mode_pitch(u8 op, u8 octave, u16 freqNumber)
{
    u8 offset = (op + 1) % 3;
    write_reg_safe(0, YM_CH3SM_BASE_FREQ_MSB_BLK + offset, (freqNumber >> 8) | (octave << 3));
    write_reg_safe(0, YM_CH3SM_BASE_FREQ_LSB + offset, freqNumber);
}

void synth_special_mode_volume(u8 operator, u8 volume)
{
    Operator* op = get_operator(CH3_SPECIAL_MODE, operator);
    if (!is_output_operator(fmChannels[CH3_SPECIAL_MODE].algorithm, operator)) {
        return;
    }

    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - op->totalLevel;
    u8 inverseNewTotalLevel = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    u8 newTotalLevel = 0x7F - inverseNewTotalLevel;

    write_operator_reg(CH3_SPECIAL_MODE, operator, YM_BASE_TOTAL_LEVEL,
        effective_total_level(CH3_SPECIAL_MODE, operator, newTotalLevel));
}

void synth_direct_write_ym2612(u8 part, u8 reg, u8 data)
{
    write_reg_safe(part, reg, data);
}

void synth_enable_dac(bool enable)
{
    global.dacEnable = enable;
    write_dac_reg();
}

static void write_reg_safe(u8 part, u8 reg, u8 data)
{
    debug_message("call: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n", part, reg, data);

    bool takenAlready = Z80_getAndRequestBus(TRUE);
    YM2612_writeReg(part, reg, data);
    if (!takenAlready) {
        release_z80_bus();
    }
}

static void release_z80_bus(void)
{
    YM2612_write(0, YM_DAC_DATA); // Latch reg address for PCM driver
    Z80_releaseBus();
}
