#pragma once
#include "genesis.h"

#define MAX_FM_OPERATORS 4
#define MAX_FM_CHANS 6
#define FM_ALGORITHMS 8

#define CH3_SPECIAL_MODE 2

#define STEREO_MODE_CENTRE 3
#define STEREO_MODE_RIGHT 1
#define STEREO_MODE_LEFT 2

typedef struct Operator {
    u8 multiple : 4;
    u8 detune : 3;
    u8 attackRate : 5;
    u8 rateScaling : 2;
    u8 decayRate : 5;
    u8 amplitudeModulation : 1;
    u8 sustainLevel : 4;
    u8 sustainRate : 5;
    u8 releaseRate : 4;
    u8 totalLevel : 7;
    u8 ssgEg : 4;
} Operator;

typedef struct FmChannel {
    u8 algorithm : 3;
    u8 feedback : 3;
    u8 stereo : 2;
    u8 ams : 2;
    u8 fms : 3;
    u8 octave : 3;
    u16 freqNumber : 11;
    Operator operators[MAX_FM_OPERATORS];
} FmChannel;

typedef struct Global {
    u8 lfoEnable : 1;
    u8 lfoFrequency : 3;
    bool specialMode : 1;
} Global;

typedef enum ParameterUpdated { Channel, Lfo, SpecialMode } ParameterUpdated;

typedef void ParameterUpdatedCallback(u8 fmChan, ParameterUpdated parameterUpdated);

void synth_init(const FmChannel* initialPreset);
void synth_noteOn(u8 channel);
void synth_noteOff(u8 channel);
void synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void synth_volume(u8 channel, u8 volume);
void synth_stereo(u8 channel, u8 mode);
void synth_algorithm(u8 channel, u8 algorithm);
void synth_feedback(u8 channel, u8 feedback);
void synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
void synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
void synth_operatorDetune(u8 channel, u8 op, u8 detune);
void synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
void synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
void synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate);
void synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate);
void synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel);
void synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation);
void synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
void synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
void synth_enableLfo(u8 enable);
void synth_globalLfoFrequency(u8 freq);
void synth_ams(u8 channel, u8 ams);
void synth_fms(u8 channel, u8 fms);
u8 synth_busy(void);
void synth_preset(u8 channel, const FmChannel* preset);
const FmChannel* synth_channelParameters(u8 channel);
const Global* synth_globalParameters();
void synth_setParameterUpdateCallback(ParameterUpdatedCallback* cb);
void synth_setSpecialMode(bool enable);
void synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
void synth_specialModeVolume(u8 op, u8 volume);
void synth_directWriteYm2612(u8 part, u8 reg, u8 data);
void synth_enableDac(bool enable);
