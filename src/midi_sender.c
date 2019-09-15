#include "midi_sender.h"
#include "comm.h"

void midi_sender_send_sysex(const u8* data, u16 length)
{
    comm_write(0xF0);
    for (u16 i = 0; i < length; i++) {

        comm_write(data[i]);
    }
    comm_write(0xF7);
}
