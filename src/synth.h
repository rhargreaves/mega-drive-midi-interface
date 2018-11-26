#include <types.h>

#define MAX_FM_OPERATORS 4
#define MAX_FM_CHANS 6

#define STEREO_MODE_CENTRE 3
#define STEREO_MODE_RIGHT 1
#define STEREO_MODE_LEFT 2

void synth_init(void);
void synth_noteOn(u8 channel);
void synth_noteOff(u8 channel);
void synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void synth_totalLevel(u8 channel, u8 totalLevel);
void synth_stereo(u8 channel, u8 mode);
void synth_algorithm(u8 channel, u8 algorithm);
void synth_feedback(u8 channel, u8 feedback);
void synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
void synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
void synth_operatorDetune(u8 channel, u8 op, u8 detune);
void synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
void synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
void synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate);
void synth_operatorSecondDecayRate(u8 channel, u8 op, u8 secondDecayRate);
