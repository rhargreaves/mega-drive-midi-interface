#include "midi.h"
#include "midi_sender.h"
#include "comm/comm.h"

void midi_sender_send_sysex(const u8* data, u16 length)
{
    comm_write(SYSEX_START);
    for (u16 i = 0; i < length; i++) {

        comm_write(data[i]);
    }
    comm_write(SYSEX_END);
}
