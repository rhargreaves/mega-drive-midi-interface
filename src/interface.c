#include <interface.h>
#include <midi.h>

void interface_tick(void)
{
    midi_process(10);
}
