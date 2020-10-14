#include "mediator.h"
#include "applemidi.h"
#include "sys.h"

#define MAX_UDP_DATA_LENGTH 1460

mw_err mediator_receive(void)
{
    char buffer[MAX_UDP_DATA_LENGTH];
    s16 length = sizeof(buffer);
    u8 ch;
    mw_err err = mw_recv_sync(&ch, buffer, &length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    switch (ch) {
    case CH_CONTROL_PORT:
        return applemidi_processSessionControlPacket(buffer, length);
    case CH_MIDI_PORT:
        return applemidi_processSessionMidiPacket(buffer, length);
    }

    return MW_ERR_NONE;
}

void mediator_send(u8 ch, char* data, u16 len)
{
    mw_err err = mw_send_sync(ch, data, len, 0);
    if (err != MW_ERR_NONE) {
        SYS_die("mw_send_sync returned error");
    }
}
