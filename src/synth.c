#include "synth.h"
#include "bits.h"
#include <memory.h>
#include <z80_ctrl.h>
#include <ym2612.h>

static Global global
    = { .lfoEnable = 1, .lfoFrequency = 0, .specialMode = false };
static FmChannel fmChannels[MAX_FM_CHANS];
static u8 noteOn;
static u8 volumes[MAX_FM_CHANS];

static ParameterUpdatedCallback* parameterUpdatedCallback = NULL;

static const u8 MAX_VOLUME = 0x7F;

static const u8 VOLUME_TO_TOTAL_LEVELS[] = { 127, 122, 117, 113, 108, 104, 100,
    97, 93, 89, 86, 83, 80, 77, 74, 71, 68, 66, 63, 61, 58, 56, 54, 52, 50, 48,
    46, 44, 43, 41, 40, 38, 37, 35, 34, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23,
    22, 21, 20, 19, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 11,
    10, 10, 9, 9, 9, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3,
    3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void updateChannel(u8 chan);
static void writeGlobalLfo(void);
static void writeAlgorithmAndFeedback(u8 channel);
static void writeOperatorMultipleAndDetune(u8 channel, u8 op);
static void writeOperatorRateScalingAndAttackRate(u8 channel, u8 operator);
static void writeOperatorAmplitudeModulationAndFirstDecayRate(
    u8 channel, u8 operator);
static void writeOperatorReleaseRateAndSecondaryAmplitude(
    u8 channel, u8 operator);
static void writeOperatorTotalLevel(u8 channel, u8 operator);
static void writeOperatorSecondaryDecayRate(u8 channel, u8 operator);
static void writeOperatorSsgEg(u8 channel, u8 operator);
static void writeStereoAmsFms(u8 channel);
static void writeChannelReg(u8 channel, u8 baseReg, u8 data);
static void writeOperatorReg(u8 channel, u8 op, u8 baseReg, u8 data);
static void writeOctaveAndFrequency(u8 channel);
static void writeSpecialModeReg(void);
static u8 keyOnOffRegOffset(u8 channel);
static FmChannel* fmChannel(u8 channel);
static Operator* getOperator(u8 channel, u8 operator);
static u8 effectiveTotalLevel(u8 channel, u8 operator, u8 totalLevel);
static bool isOutputOperator(u8 algorithm, u8 operator);
static u8 volumeAdjustedTotalLevel(u8 channel, u8 totalLevel);
static void channelParameterUpdated(u8 channel);
static void otherParameterUpdated(
    u8 channel, ParameterUpdated parameterUpdated);

void synth_init(const FmChannel* initialPreset)
{
    Z80_requestBus(TRUE);
    writeSpecialModeReg();
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        volumes[chan] = MAX_VOLUME;
        synth_noteOff(chan);
        memcpy(&fmChannels[chan], initialPreset, sizeof(FmChannel));
        updateChannel(chan);
    }
    writeGlobalLfo();
}

static void updateChannel(u8 chan)
{
    writeAlgorithmAndFeedback(chan);
    writeStereoAmsFms(chan);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        writeOperatorMultipleAndDetune(chan, op);
        writeOperatorRateScalingAndAttackRate(chan, op);
        writeOperatorAmplitudeModulationAndFirstDecayRate(chan, op);
        writeOperatorSecondaryDecayRate(chan, op);
        writeOperatorReleaseRateAndSecondaryAmplitude(chan, op);
        writeOperatorTotalLevel(chan, op);
        writeOperatorSsgEg(chan, op);
    }
}

void synth_noteOn(u8 channel)
{
    YM2612_writeReg(0, 0x28, 0xF0 + keyOnOffRegOffset(channel));
    SET_BIT(noteOn, channel);
}

void synth_noteOff(u8 channel)
{
    YM2612_writeReg(0, 0x28, keyOnOffRegOffset(channel));
    CLEAR_BIT(noteOn, channel);
}

void synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    FmChannel* chan = fmChannel(channel);
    chan->octave = octave;
    chan->freqNumber = freqNumber;
    writeOctaveAndFrequency(channel);
}

void synth_volume(u8 channel, u8 volume)
{
    if (volumes[channel] == volume) {
        return;
    }
    volumes[channel] = volume;
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        writeOperatorTotalLevel(channel, op);
    }
}

void synth_stereo(u8 channel, u8 stereo)
{
    fmChannel(channel)->stereo = stereo;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

void synth_algorithm(u8 channel, u8 algorithm)
{
    fmChannel(channel)->algorithm = algorithm;
    writeAlgorithmAndFeedback(channel);
    channelParameterUpdated(channel);
}

void synth_feedback(u8 channel, u8 feedback)
{
    fmChannel(channel)->feedback = feedback;
    writeAlgorithmAndFeedback(channel);
    channelParameterUpdated(channel);
}

void synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    Operator* oper = getOperator(channel, op);
    if (oper->totalLevel == totalLevel) {
        return;
    }
    oper->totalLevel = totalLevel;
    writeOperatorTotalLevel(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorMultiple(u8 channel, u8 op, u8 multiple)
{
    getOperator(channel, op)->multiple = multiple;
    writeOperatorMultipleAndDetune(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorDetune(u8 channel, u8 op, u8 detune)
{
    getOperator(channel, op)->detune = detune;
    writeOperatorMultipleAndDetune(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling)
{
    getOperator(channel, op)->rateScaling = rateScaling;
    writeOperatorRateScalingAndAttackRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate)
{
    getOperator(channel, op)->attackRate = attackRate;
    writeOperatorRateScalingAndAttackRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorSecondDecayRate(u8 channel, u8 op, u8 secondDecayRate)
{
    getOperator(channel, op)->secondaryDecayRate = secondDecayRate;
    writeOperatorSecondaryDecayRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    getOperator(channel, op)->releaseRate = releaseRate;
    writeOperatorReleaseRateAndSecondaryAmplitude(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg)
{
    getOperator(channel, op)->ssgEg = ssgEg;
    writeOperatorSsgEg(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorSecondaryAmplitude(u8 channel, u8 op, u8 secondaryAmplitude)
{
    getOperator(channel, op)->secondaryAmplitude = secondaryAmplitude;
    writeOperatorReleaseRateAndSecondaryAmplitude(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate)
{
    getOperator(channel, op)->firstDecayRate = firstDecayRate;
    writeOperatorAmplitudeModulationAndFirstDecayRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation)
{
    getOperator(channel, op)->amplitudeModulation = amplitudeModulation;
    writeOperatorAmplitudeModulationAndFirstDecayRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_enableLfo(u8 enable)
{
    global.lfoEnable = enable;
    writeGlobalLfo();
    otherParameterUpdated(0, Lfo);
}

void synth_globalLfoFrequency(u8 freq)
{
    global.lfoFrequency = freq;
    writeGlobalLfo();
    otherParameterUpdated(0, Lfo);
}

void synth_ams(u8 channel, u8 ams)
{
    fmChannel(channel)->ams = ams;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

void synth_fms(u8 channel, u8 fms)
{
    fmChannel(channel)->fms = fms;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

u8 synth_busy(void)
{
    return noteOn;
}

void synth_preset(u8 channel, const FmChannel* preset)
{
    memcpy(&fmChannels[channel], preset, sizeof(FmChannel));
    updateChannel(channel);
    channelParameterUpdated(channel);
}

static void otherParameterUpdated(u8 channel, ParameterUpdated parameterUpdated)
{
    if (parameterUpdatedCallback) {
        parameterUpdatedCallback(channel, parameterUpdated);
    }
}

static void channelParameterUpdated(u8 channel)
{
    if (parameterUpdatedCallback) {
        parameterUpdatedCallback(channel, Channel);
    }
}

static void writeChannelReg(u8 channel, u8 baseReg, u8 data)
{
    YM2612_writeReg(channel > 2 ? 1 : 0, baseReg + (channel % 3), data);
}

static u8 regOperatorIndex(u8 op)
{
    if (op == 1)
        return 2;
    else if (op == 2)
        return 1;
    else {
        return op;
    }
}

static void writeOperatorReg(u8 channel, u8 op, u8 baseReg, u8 data)
{
    writeChannelReg(channel, baseReg + (regOperatorIndex(op) * 4), data);
}

static u8 keyOnOffRegOffset(u8 channel)
{
    return (channel < 3) ? channel : (channel + 1);
}

static FmChannel* fmChannel(u8 channel)
{
    return &fmChannels[channel];
}

static Operator* getOperator(u8 channel, u8 operator)
{
    return &fmChannel(channel)->operators[operator];
}

static void writeGlobalLfo(void)
{
    YM2612_writeReg(0, 0x22, (global.lfoEnable << 3) | global.lfoFrequency);
}

static void writeOctaveAndFrequency(u8 channel)
{
    FmChannel* chan = fmChannel(channel);
    writeChannelReg(
        channel, 0xA4, (chan->freqNumber >> 8) | (chan->octave << 3));
    writeChannelReg(channel, 0xA0, chan->freqNumber);
}

static void writeAlgorithmAndFeedback(u8 channel)
{
    FmChannel* chan = fmChannel(channel);
    writeChannelReg(channel, 0xB0, (chan->feedback << 3) + chan->algorithm);
}

static void writeStereoAmsFms(u8 channel)
{
    FmChannel* chan = fmChannel(channel);
    writeChannelReg(
        channel, 0xB4, (chan->stereo << 6) + (chan->ams << 4) + chan->fms);
}

static void writeOperatorMultipleAndDetune(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x30, op->multiple + (op->detune << 4));
}

static void writeOperatorRateScalingAndAttackRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(
        channel, operator, 0x50, op->attackRate + (op->rateScaling << 6));
}

static void writeOperatorAmplitudeModulationAndFirstDecayRate(
    u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x60,
        op->firstDecayRate + (op->amplitudeModulation << 7));
}

static void writeOperatorSecondaryDecayRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x70, op->secondaryDecayRate);
}

static void writeOperatorReleaseRateAndSecondaryAmplitude(
    u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x80,
        op->releaseRate + (op->secondaryAmplitude << 4));
}

static void writeOperatorSsgEg(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x90, op->ssgEg);
}

static void writeOperatorTotalLevel(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x40,
        effectiveTotalLevel(channel, operator, op->totalLevel));
}

static u8 effectiveTotalLevel(u8 channel, u8 operator, u8 totalLevel)
{
    return isOutputOperator(fmChannel(channel)->algorithm, operator)
        ? volumeAdjustedTotalLevel(channel, totalLevel)
        : totalLevel;
}

static bool isOutputOperator(u8 algorithm, u8 operator)
{
    return (algorithm < 4 && operator== 3)
        || (algorithm == 4 && (operator== 2 || operator== 3))
        || ((algorithm == 5 || algorithm == 6) && operator> 0)
        || algorithm == 7;
}

static u8 volumeAdjustedTotalLevel(u8 channel, u8 totalLevel)
{
    u8 volume = volumes[channel];
    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - totalLevel;
    u8 inverseNewTotalLevel
        = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    return 0x7F - inverseNewTotalLevel;
}

const FmChannel* synth_channelParameters(u8 channel)
{
    return fmChannel(channel);
}

const Global* synth_globalParameters()
{
    return &global;
}

void synth_setParameterUpdateCallback(ParameterUpdatedCallback* cb)
{
    parameterUpdatedCallback = cb;
}

static void writeSpecialModeReg(void)
{
    YM2612_writeReg(0, 0x27, global.specialMode << 6);
}

void synth_setSpecialMode(bool enable)
{
    global.specialMode = enable;
    writeSpecialModeReg();
}

void synth_specialModePitch(u8 op, u8 octave, u16 freqNumber)
{
    u8 offset = (op + 1) % 3;
    YM2612_writeReg(0, 0xAC + offset, (freqNumber >> 8) | (octave << 3));
    YM2612_writeReg(0, 0xA8 + offset, freqNumber);
}

void synth_specialModeVolume(u8 operator, u8 volume)
{
    Operator* op = getOperator(CH_SPECIAL_MODE, operator);

    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - op->totalLevel;
    u8 inverseNewTotalLevel
        = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    u8 newTotalLevel = 0x7F - inverseNewTotalLevel;

    writeOperatorReg(CH_SPECIAL_MODE, operator, 0x40,
        effectiveTotalLevel(CH_SPECIAL_MODE, operator, newTotalLevel));
}