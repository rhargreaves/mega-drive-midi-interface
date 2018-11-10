#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

static u8 lastUnknownStatus = 0;

void interface_init(void)
{
    synth_init();
}

void interface_loop(void)
{
    while (TRUE) {
        interface_tick();
    }
}

void interface_tick(void)
{
    u8 status = comm_read();
    u8 upperStatus = status & 0xF0;
    if (upperStatus == 0x90) {
        u8 chan = status & 0x0F;
        u8 pitch = comm_read();
        u8 velocity = comm_read();
        midi_noteOn(
            chan,
            pitch,
            velocity);
    } else if (upperStatus == 0x80) {
        u8 chan = status & 0x0F;
        comm_read();
        comm_read();
        midi_noteOff(chan);
    } else {
        lastUnknownStatus = status;
    }
}

u8 interface_lastUnknownStatus(void)
{
    return lastUnknownStatus;
}
