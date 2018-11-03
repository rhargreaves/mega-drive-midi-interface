#include <comm.h>
#include <interface.h>
#include <midi.h>

void interface_tick(void)
{
    u8 data = comm_read();
    midi_process(data);
}
