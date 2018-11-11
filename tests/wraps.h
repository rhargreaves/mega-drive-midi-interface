#include <midi.h>
#include <types.h>

void __wrap_synth_init(void);
void __wrap_synth_noteOn(u8 channel);
void __wrap_synth_noteOff(u8 channel);
void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void __wrap_synth_totalLevel(u8 channel, u8 totalLevel);
void __wrap_synth_stereo(u8 channel, u8 mode);
u8 __wrap_comm_read(void);
void __wrap_fm_writeReg(u16 part, u8 reg, u8 data);
void __wrap_psg_noteOn(u8 channel, u16 freq, u8 attenuation);
void __wrap_midi_noteOff(u8 chan);
void __wrap_midi_noteOn(u8 chan, u8 pitch, u8 velocity);
void __wrap_midi_channelVolume(u8 chan, u8 volume);
void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data);
void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_SYS_setVIntCallback(_voidCallback* CB);
void __wrap_VDP_setTextPalette(u16 palette);
void __wrap_VDP_clearText(u16 x, u16 y, u16 w);
void __wrap_PSG_setEnvelope(u8 channel, u8 value);
void __wrap_PSG_setFrequency(u8 channel, u16 value);
