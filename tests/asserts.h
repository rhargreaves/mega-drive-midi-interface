#include <types.h>

void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2);
void expect_ym2612_write_reg(u8 part, u8 reg, u8 data);
void expect_ym2612_writeOperator(u8 chan, u8 op, u8 baseReg, u8 data);
void expect_ym2612_writeOperator_any_data(u8 chan, u8 op, u8 baseReg);
void expect_ym2612_write_channel(u8 chan, u8 baseReg, u8 data);
