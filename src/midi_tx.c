#include "midi_tx.h"
#include "midi.h"
#include "comm/comm.h"

void midi_tx_send_sysex(const u8* data, u16 length)
{
    comm_write(SYSEX_START);
    for (u16 i = 0; i < length; i++) {

        comm_write(data[i]);
    }
    comm_write(SYSEX_END);
}

void midi_tx_send_cc(u8 channel, u8 controller, u8 value)
{
    comm_write(0xB0 | (channel & 0x0F));
    comm_write(controller);
    comm_write(value);
}
