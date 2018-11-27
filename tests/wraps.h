#include <midi.h>
#include <types.h>

void __wrap_synth_init(void);
void __wrap_synth_noteOn(u8 channel);
void __wrap_synth_noteOff(u8 channel);
void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void __wrap_synth_totalLevel(u8 channel, u8 totalLevel);
void __wrap_synth_stereo(u8 channel, u8 mode);
void __wrap_synth_algorithm(u8 channel, u8 algorithm);
void __wrap_synth_feedback(u8 channel, u8 feedback);
void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune);
void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
void __wrap_synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate);
void __wrap_synth_operatorSecondDecayRate(
    u8 channel, u8 op, u8 secondDecayRate);
void __wrap_synth_operatorSecondaryAmplitude(
    u8 channel, u8 op, u8 secondaryAmplitude);
void __wrap_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation);
u8 __wrap_comm_read(void);
void __wrap_fm_writeReg(u16 part, u8 reg, u8 data);
void __wrap_psg_noteOn(u8 channel, u16 freq);
void __wrap_psg_noteOff(u8 channel);
void __wrap_psg_attenuation(u8 channel, u8 attenuation);
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
u8 __wrap_ssf_usb_read(void);
u16 __wrap_ssf_usb_rd_ready(void);
