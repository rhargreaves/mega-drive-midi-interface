#include <types.h>

void synth_init(void);
void synth_noteOn(u8 channel);
void synth_noteOff(u8 channel);
void synth_pitch(u8 octave, u16 freqNumber);
