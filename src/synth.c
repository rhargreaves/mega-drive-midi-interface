#include "synth.h"
#include "bits.h"
#include "debug.h"
#include "ym2612_regs.h"

static Global global = { .lfoEnable = 1, .lfoFrequency = 0, .specialMode = false };
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

static void updateChannel(u8 chan);
static void writeGlobalLfo(void);
static void writeAlgorithmAndFeedback(u8 channel);
static void writeOperatorMultipleAndDetune(u8 channel, u8 operator);
static void writeOperatorRateScalingAndAttackRate(u8 channel, u8 operator);
static void writeOperatorAmplitudeModulationAndDecayRate(u8 channel, u8 operator);
static void writeOperatorReleaseRateAndSustainLevel(u8 channel, u8 operator);
static void writeOperatorTotalLevel(u8 channel, u8 operator);
static void writeOperatorSustainRate(u8 channel, u8 operator);
static void writeOperatorSsgEg(u8 channel, u8 operator);
static void writeStereoAmsFms(u8 channel);
static void writeChannelReg(u8 channel, u8 baseReg, u8 data);
static void writeOperatorReg(u8 channel, u8 op, u8 baseReg, u8 data);
static void writeOctaveAndFrequency(u8 channel);
static void writeSpecialModeReg(void);
static u8 keyOnOffRegOffset(u8 channel);
static Operator* getOperator(u8 channel, u8 operator);
static u8 effectiveTotalLevel(u8 channel, u8 operator, u8 totalLevel);
static bool isOutputOperator(u8 algorithm, u8 op);
static u8 volumeAdjustedTotalLevel(u8 channel, u8 totalLevel);
static void channelParameterUpdated(u8 channel);
static void otherParameterUpdated(u8 channel, ParameterUpdated parameterUpdated);
static void writeRegSafe(u8 part, u8 reg, u8 data);
static void releaseZ80Bus(void);

void synth_init(const FmChannel* initialPreset)
{
    Z80_loadDriver(Z80_DRIVER_PCM, true);
    Z80_requestBus(TRUE);
    writeSpecialModeReg();
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        volumes[chan] = MAX_VOLUME;
        synth_noteOff(chan);
        memcpy(&fmChannels[chan], initialPreset, sizeof(FmChannel));
        updateChannel(chan);
    }
    writeGlobalLfo();
    releaseZ80Bus();
}

static void updateChannel(u8 chan)
{
    writeAlgorithmAndFeedback(chan);
    writeStereoAmsFms(chan);
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        writeOperatorMultipleAndDetune(chan, op);
        writeOperatorRateScalingAndAttackRate(chan, op);
        writeOperatorAmplitudeModulationAndDecayRate(chan, op);
        writeOperatorSustainRate(chan, op);
        writeOperatorReleaseRateAndSustainLevel(chan, op);
        writeOperatorTotalLevel(chan, op);
        writeOperatorSsgEg(chan, op);
    }
}

void synth_noteOn(u8 channel)
{
    writeRegSafe(0, YM_KEY_ON_OFF, 0xF0 + keyOnOffRegOffset(channel));
    SET_BIT(noteOn, channel);
}

void synth_noteOff(u8 channel)
{
    writeRegSafe(0, YM_KEY_ON_OFF, keyOnOffRegOffset(channel));
    CLEAR_BIT(noteOn, channel);
}

void synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    FmChannel* chan = &fmChannels[channel];
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
    fmChannels[channel].stereo = stereo;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

void synth_algorithm(u8 channel, u8 algorithm)
{
    fmChannels[channel].algorithm = algorithm;
    writeAlgorithmAndFeedback(channel);
    channelParameterUpdated(channel);
}

void synth_feedback(u8 channel, u8 feedback)
{
    fmChannels[channel].feedback = feedback;
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

void synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate)
{
    getOperator(channel, op)->sustainRate = sustainRate;
    writeOperatorSustainRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    getOperator(channel, op)->releaseRate = releaseRate;
    writeOperatorReleaseRateAndSustainLevel(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg)
{
    getOperator(channel, op)->ssgEg = ssgEg;
    writeOperatorSsgEg(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel)
{
    getOperator(channel, op)->sustainLevel = sustainLevel;
    writeOperatorReleaseRateAndSustainLevel(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate)
{
    getOperator(channel, op)->decayRate = decayRate;
    writeOperatorAmplitudeModulationAndDecayRate(channel, op);
    channelParameterUpdated(channel);
}

void synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation)
{
    getOperator(channel, op)->amplitudeModulation = amplitudeModulation;
    writeOperatorAmplitudeModulationAndDecayRate(channel, op);
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
    fmChannels[channel].ams = ams;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

void synth_fms(u8 channel, u8 fms)
{
    fmChannels[channel].fms = fms;
    writeStereoAmsFms(channel);
    channelParameterUpdated(channel);
}

u8 synth_busy(void)
{
    return noteOn;
}

void synth_preset(u8 channel, const FmChannel* preset)
{
    FmChannel* chan = &fmChannels[channel];
    u8 currentStereo = chan->stereo;
    memcpy(chan, preset, sizeof(FmChannel));
    chan->stereo = currentStereo;
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
    u8 part = channel > 2 ? 1 : 0;
    u8 reg = baseReg + (channel % 3);
    writeRegSafe(part, reg, data);
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

static Operator* getOperator(u8 channel, u8 operator)
{
    return &fmChannels[channel].operators[operator];
}

static void writeGlobalLfo(void)
{
    writeRegSafe(0, YM_LFO_ENABLE, (global.lfoEnable << 3) | global.lfoFrequency);
}

static void writeOctaveAndFrequency(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    writeChannelReg(channel, YM_BASE_FREQ_MSB_BLK, (chan->freqNumber >> 8) | (chan->octave << 3));
    writeChannelReg(channel, YM_BASE_FREQ_LSB, chan->freqNumber);
}

static void writeAlgorithmAndFeedback(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    writeChannelReg(channel, YM_BASE_ALGORITHM_FEEDBACK, (chan->feedback << 3) + chan->algorithm);
}

static void writeStereoAmsFms(u8 channel)
{
    FmChannel* chan = &fmChannels[channel];
    writeChannelReg(
        channel, YM_BASE_STEREO_AMS_PMS, (chan->stereo << 6) + (chan->ams << 4) + chan->fms);
}

static void writeOperatorMultipleAndDetune(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_MULTIPLE_DETUNE, op->multiple + (op->detune << 4));
}

static void writeOperatorRateScalingAndAttackRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_ATTACK_RATE_SCALING_RATE,
        op->attackRate + (op->rateScaling << 6));
}

static void writeOperatorAmplitudeModulationAndDecayRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_DECAY_RATE_AM_ENABLE,
        op->decayRate + (op->amplitudeModulation << 7));
}

static void writeOperatorSustainRate(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_SUSTAIN_RATE, op->sustainRate);
}

static void writeOperatorReleaseRateAndSustainLevel(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_RELEASE_RATE_SUSTAIN_LEVEL,
        op->releaseRate + (op->sustainLevel << 4));
}

static void writeOperatorSsgEg(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_SSG_EG, op->ssgEg);
}

static void writeOperatorTotalLevel(u8 channel, u8 operator)
{
    Operator* op = getOperator(channel, operator);
    writeOperatorReg(channel, operator, YM_BASE_TOTAL_LEVEL,
        effectiveTotalLevel(channel, operator, op->totalLevel));
}

static u8 effectiveTotalLevel(u8 channel, u8 operator, u8 totalLevel)
{
    return isOutputOperator(fmChannels[channel].algorithm, operator)
        ? volumeAdjustedTotalLevel(channel, totalLevel)
        : totalLevel;
}

static bool isOutputOperator(u8 algorithm, u8 op)
{
    return (algorithm < 4 && op == 3) || (algorithm == 4 && (op == 1 || op == 3))
        || ((algorithm == 5 || algorithm == 6) && op > 0) || algorithm == 7;
}

static u8 volumeAdjustedTotalLevel(u8 channel, u8 totalLevel)
{
    u8 volume = volumes[channel];
    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - totalLevel;
    u8 inverseNewTotalLevel = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    return 0x7F - inverseNewTotalLevel;
}

const FmChannel* synth_channelParameters(u8 channel)
{
    return &fmChannels[channel];
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
    writeRegSafe(0, YM_CH3_MODE, global.specialMode << 6);
}

void synth_setSpecialMode(bool enable)
{
    global.specialMode = enable;
    writeSpecialModeReg();
    otherParameterUpdated(0, SpecialMode);
}

void synth_specialModePitch(u8 op, u8 octave, u16 freqNumber)
{
    u8 offset = (op + 1) % 3;
    writeRegSafe(0, YM_CH3SM_BASE_FREQ_MSB_BLK + offset, (freqNumber >> 8) | (octave << 3));
    writeRegSafe(0, YM_CH3SM_BASE_FREQ_LSB + offset, freqNumber);
}

void synth_specialModeVolume(u8 operator, u8 volume)
{
    Operator* op = getOperator(CH3_SPECIAL_MODE, operator);
    if (!isOutputOperator(fmChannels[CH3_SPECIAL_MODE].algorithm, operator)) {
        return;
    }

    u8 logarithmicVolume = 0x7F - VOLUME_TO_TOTAL_LEVELS[volume];
    u8 inverseTotalLevel = 0x7F - op->totalLevel;
    u8 inverseNewTotalLevel = (u16)inverseTotalLevel * (u16)logarithmicVolume / (u16)0x7F;
    u8 newTotalLevel = 0x7F - inverseNewTotalLevel;

    writeOperatorReg(CH3_SPECIAL_MODE, operator, YM_BASE_TOTAL_LEVEL,
        effectiveTotalLevel(CH3_SPECIAL_MODE, operator, newTotalLevel));
}

void synth_directWriteYm2612(u8 part, u8 reg, u8 data)
{
    writeRegSafe(part, reg, data);
}

void synth_enableDac(bool enable)
{
    writeRegSafe(0, YM_DAC_ENABLE, enable ? 0x80 : 0);
}

static void writeRegSafe(u8 part, u8 reg, u8 data)
{
    debug_message("call: YM2612_writeReg(part=%d, reg=0x%X, data=0x%X)\n", part, reg, data);

    bool takenAlready = Z80_getAndRequestBus(TRUE);
    YM2612_writeReg(part, reg, data);
    if (!takenAlready) {
        releaseZ80Bus();
    }
}

static void releaseZ80Bus(void)
{
    YM2612_write(0, YM_DAC_DATA); // Latch reg address for PCM driver
    Z80_releaseBus();
}
