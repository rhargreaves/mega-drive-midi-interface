#include "synth.h"
#include "bits.h"
#include "presets.h"
#include <memory.h>
#include <ym2612.h>

typedef struct Global Global;

struct Global {
    u8 lfoEnable;
    u8 lfoFrequency;
};

static Global global = { .lfoEnable = 0, .lfoFrequency = 0 };
static Channel channels[MAX_FM_CHANS];
static u8 noteOn;

static void initChannel(u8 chan);
static void updateChannel(u8 chan);
static void updateGlobalLfo(void);
static void updateAlgorithmAndFeedback(u8 channel);
static void updateOperatorMultipleAndDetune(u8 channel, u8 op);
static void updateOperatorRateScalingAndAttackRate(u8 channel, u8 operator);
static void updateOperatorAmplitudeModulationAndFirstDecayRate(
    u8 channel, u8 operator);
static void updateOperatorReleaseRateAndSecondaryAmplitude(
    u8 channel, u8 operator);
static void updateOperatorTotalLevel(u8 channel, u8 operator);
static void updateOperatorSecondaryDecayRate(u8 channel, u8 operator);
static void updateOperatorSsgEg(u8 channel, u8 operator);
static void updateStereoAmsFms(u8 channel);
static void writeChannelReg(u8 channel, u8 baseReg, u8 data);
static void writeOperatorReg(u8 channel, u8 op, u8 baseReg, u8 data);
static void updateOctaveAndFrequency(u8 channel);
static u8 keyOnOffRegOffset(u8 channel);
static Channel* getChannel(u8 channel);
static Operator* getOperator(u8 channel, u8 operator);

void synth_init(void)
{
    YM2612_writeReg(0, 0x27, 0); // Ch 3 Normal
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        synth_noteOff(chan);
        initChannel(chan);
    }
}

static void initChannel(u8 chan)
{
    memcpy(&channels[chan], &PRESET_DEFAULT, sizeof(Channel));
    updateChannel(chan);
}

static void updateChannel(u8 chan)
{
    updateAlgorithmAndFeedback(chan);
    updateStereoAmsFms(chan);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        updateOperatorMultipleAndDetune(chan, op);
        updateOperatorRateScalingAndAttackRate(chan, op);
        updateOperatorAmplitudeModulationAndFirstDecayRate(chan, op);
        updateOperatorSecondaryDecayRate(chan, op);
        updateOperatorReleaseRateAndSecondaryAmplitude(chan, op);
        updateOperatorTotalLevel(chan, op);
        updateOperatorSsgEg(chan, op);
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
    Channel* chan = getChannel(channel);
    chan->octave = octave;
    chan->freqNumber = freqNumber;
    updateOctaveAndFrequency(channel);
}

void synth_totalLevel(u8 channel, u8 totalLevel)
{
    const u8 REG_TOTAL_LEVEL = 0x40;
    Channel* chan = getChannel(channel);
    switch (chan->algorithm) {
    case 7:
        writeOperatorReg(channel, 0, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 1, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 2, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 3, REG_TOTAL_LEVEL, totalLevel);
        break;
    case 6:
    case 5:
        writeOperatorReg(channel, 1, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 2, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 3, REG_TOTAL_LEVEL, totalLevel);
        break;
    case 4:
        writeOperatorReg(channel, 1, REG_TOTAL_LEVEL, totalLevel);
        writeOperatorReg(channel, 3, REG_TOTAL_LEVEL, totalLevel);
        break;
    default:
        writeOperatorReg(channel, 3, REG_TOTAL_LEVEL, totalLevel);
        break;
    }
}

void synth_stereo(u8 channel, u8 stereo)
{
    getChannel(channel)->stereo = stereo;
    updateStereoAmsFms(channel);
}

void synth_algorithm(u8 channel, u8 algorithm)
{
    getChannel(channel)->algorithm = algorithm;
    updateAlgorithmAndFeedback(channel);
}

void synth_feedback(u8 channel, u8 feedback)
{
    getChannel(channel)->feedback = feedback;
    updateAlgorithmAndFeedback(channel);
}

void synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    getOperator(channel, op)->totalLevel = totalLevel;
    updateOperatorTotalLevel(channel, op);
}

void synth_operatorMultiple(u8 channel, u8 op, u8 multiple)
{
    getOperator(channel, op)->multiple = multiple;
    updateOperatorMultipleAndDetune(channel, op);
}

void synth_operatorDetune(u8 channel, u8 op, u8 detune)
{
    getOperator(channel, op)->detune = detune;
    updateOperatorMultipleAndDetune(channel, op);
}

void synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling)
{
    getOperator(channel, op)->rateScaling = rateScaling;
    updateOperatorRateScalingAndAttackRate(channel, op);
}

void synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate)
{
    getOperator(channel, op)->attackRate = attackRate;
    updateOperatorRateScalingAndAttackRate(channel, op);
}

void synth_operatorSecondDecayRate(u8 channel, u8 op, u8 secondDecayRate)
{
    getOperator(channel, op)->secondaryDecayRate = secondDecayRate;
    updateOperatorSecondaryDecayRate(channel, op);
}

void synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    getOperator(channel, op)->releaseRate = releaseRate;
    updateOperatorReleaseRateAndSecondaryAmplitude(channel, op);
}

void synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg)
{
    getOperator(channel, op)->ssgEg = ssgEg;
    updateOperatorSsgEg(channel, op);
}

void synth_operatorSecondaryAmplitude(u8 channel, u8 op, u8 secondaryAmplitude)
{
    getOperator(channel, op)->secondaryAmplitude = secondaryAmplitude;
    updateOperatorReleaseRateAndSecondaryAmplitude(channel, op);
}

void synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate)
{
    getOperator(channel, op)->firstDecayRate = firstDecayRate;
    updateOperatorAmplitudeModulationAndFirstDecayRate(channel, op);
}

void synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation)
{
    getOperator(channel, op)->amplitudeModulation = amplitudeModulation;
    updateOperatorAmplitudeModulationAndFirstDecayRate(channel, op);
}

void synth_enableLfo(u8 enable)
{
    global.lfoEnable = enable;
    updateGlobalLfo();
}

void synth_globalLfoFrequency(u8 freq)
{
    global.lfoFrequency = freq;
    updateGlobalLfo();
}

void synth_ams(u8 channel, u8 ams)
{
    getChannel(channel)->ams = ams;
    updateStereoAmsFms(channel);
}

void synth_fms(u8 channel, u8 fms)
{
    getChannel(channel)->fms = fms;
    updateStereoAmsFms(channel);
}

u8 synth_busy(void)
{
    return noteOn;
}

void synth_preset(u8 channel, u8 preset)
{
    const Channel* data;
    switch (preset) {
    case 1:
        data = &PRESET_DEEP_PAD;
        break;
    default:
        data = &PRESET_DEFAULT;
        break;
    }
    memcpy(&channels[channel], data, sizeof(Channel));
    updateChannel(channel);
}

static void writeChannelReg(u8 channel, u8 baseReg, u8 data)
{
    YM2612_writeReg(channel > 2 ? 1 : 0, baseReg + (channel % 3), data);
}

static void writeOperatorReg(u8 channel, u8 op, u8 baseReg, u8 data)
{
    writeChannelReg(channel, baseReg + (op * 4), data);
}

static u8 keyOnOffRegOffset(u8 channel)
{
    return (channel < 3) ? channel : (channel + 1);
}

static Channel* getChannel(u8 channel)
{
    return &channels[channel];
}

static Operator* getOperator(u8 channel, u8 operator)
{
    return &getChannel(channel)->operators[operator];
}

static void updateGlobalLfo(void)
{
    YM2612_writeReg(0, 0x22, (global.lfoEnable << 3) | global.lfoFrequency);
}

static void updateOctaveAndFrequency(u8 channel)
{
    Channel* chan = getChannel(channel);
    writeChannelReg(
        channel, 0xA4, (chan->freqNumber >> 8) | (chan->octave << 3));
    writeChannelReg(channel, 0xA0, chan->freqNumber);
}

static void updateAlgorithmAndFeedback(u8 channel)
{
    Channel* chan = getChannel(channel);
    writeChannelReg(channel, 0xB0, (chan->feedback << 3) + chan->algorithm);
}

static void updateStereoAmsFms(u8 channel)
{
    Channel* chan = getChannel(channel);
    writeChannelReg(
        channel, 0xB4, (chan->stereo << 6) + (chan->ams << 4) + chan->fms);
}

static void updateOperatorMultipleAndDetune(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x30, op->multiple +(op->detune << 4));
}

static void updateOperatorRateScalingAndAttackRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(
        channel, operator, 0x50, op->attackRate +(op->rateScaling << 6));
}

static void updateOperatorAmplitudeModulationAndFirstDecayRate(
    u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x60,
        op->firstDecayRate +(op->amplitudeModulation << 7));
}

static void updateOperatorSecondaryDecayRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x70, op->secondaryDecayRate);
}

static void updateOperatorReleaseRateAndSecondaryAmplitude(
    u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x80,
        op->releaseRate +(op->secondaryAmplitude << 4));
}

static void updateOperatorSsgEg(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x90, op->ssgEg);
}

static void updateOperatorTotalLevel(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, 0x40, op->totalLevel);
}
