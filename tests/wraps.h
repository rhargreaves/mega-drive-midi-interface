#include <midi.h>
#include <types.h>

void __wrap_synth_init(void);
void __wrap_synth_noteOn(u8 channel);
void __wrap_synth_noteOff(u8 channel);
u8 __wrap_comm_read(void);
void __wrap_midi_process(Message* message);
void __wrap_fm_writeReg(u16 part, u8 reg, u8 data);
