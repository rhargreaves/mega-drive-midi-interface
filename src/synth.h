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

typedef struct FmPreset {
    u8 algorithm : 3;
    u8 feedback : 3;
    u8 ams : 2;
    u8 fms : 3;
    Operator operators[MAX_FM_OPERATORS];
} FmPreset;

typedef struct Global {
    u8 lfoEnable : 1;
    u8 lfoFrequency : 3;
    bool specialMode : 1;
    bool dacEnable : 1;
} Global;

typedef enum ParameterUpdated { Channel, Lfo, SpecialMode } ParameterUpdated;

typedef void ParameterUpdatedCallback(u8 fmChan, ParameterUpdated parameterUpdated);

void synth_init(const FmPreset* initialPreset);
void synth_note_on(u8 channel);
void synth_note_off(u8 channel);
void synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void synth_volume(u8 channel, u8 volume);
void synth_stereo(u8 channel, u8 mode);
void synth_algorithm(u8 channel, u8 algorithm);
void synth_feedback(u8 channel, u8 feedback);
void synth_operator_total_level(u8 channel, u8 op, u8 totalLevel);
void synth_operator_multiple(u8 channel, u8 op, u8 multiple);
void synth_operator_detune(u8 channel, u8 op, u8 detune);
void synth_operator_rate_scaling(u8 channel, u8 op, u8 rateScaling);
void synth_operator_attack_rate(u8 channel, u8 op, u8 attackRate);
void synth_operator_decay_rate(u8 channel, u8 op, u8 decayRate);
void synth_operator_sustain_rate(u8 channel, u8 op, u8 sustainRate);
void synth_operator_sustain_level(u8 channel, u8 op, u8 sustainLevel);
void synth_operator_amplitude_modulation(u8 channel, u8 op, u8 amplitudeModulation);
void synth_operator_release_rate(u8 channel, u8 op, u8 releaseRate);
void synth_operator_ssg_eg(u8 channel, u8 op, u8 ssgEg);
void synth_enable_lfo(u8 enable);
void synth_global_lfo_frequency(u8 freq);
void synth_ams(u8 channel, u8 ams);
void synth_fms(u8 channel, u8 fms);
u8 synth_busy(void);
void synth_preset(u8 channel, const FmPreset* preset);
const FmChannel* synth_channel_parameters(u8 channel);
void synth_extract_preset(u8 channel, FmPreset* preset);
const Global* synth_global_parameters();
void synth_set_parameter_update_callback(ParameterUpdatedCallback* cb);
void synth_set_special_mode(bool enable);
void synth_special_mode_pitch(u8 op, u8 octave, u16 freqNumber);
void synth_special_mode_volume(u8 op, u8 volume);
void synth_direct_write_ym2612(u8 part, u8 reg, u8 data);
void synth_enable_dac(bool enable);
