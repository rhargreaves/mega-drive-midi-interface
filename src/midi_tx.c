#include "midi_tx.h"
#include "midi.h"
#include "comm/comm.h"

void midi_tx_send_sysex(const u8* data, u16 length)
{
    u16 totalLength = length + 2;
    u8 sysexData[totalLength];

    sysexData[0] = SYSEX_START;
    memcpy(&sysexData[1], data, length);
    sysexData[totalLength - 1] = SYSEX_END;

    comm_write(sysexData, totalLength);
}
