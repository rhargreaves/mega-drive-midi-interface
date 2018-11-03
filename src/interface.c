#include <comm.h>
#include <interface.h>
#include <midi.h>

void interface_tick(void)
{
    u8 status = comm_read();
    u8 data = comm_read();
    u8 data2 = comm_read();
    Message message = { status, data, data2 };
    midi_process(&message);
}
