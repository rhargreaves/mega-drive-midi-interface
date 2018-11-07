#include <types.h>

#define MAX_SYNTH_CHANS 6

void synth_init(void);
void synth_noteOn(u8 channel);
void synth_noteOff(u8 channel);
void synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void synth_totalLevel(u8 channel, u8 totalLevel);
