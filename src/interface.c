#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

static char lastError[20];

void interface_init(void)
{
    synth_init();
}

void interface_tick(void)
{
    u8 status = comm_read();
    if ((status & 0xF0) == 0x90) {
        u8 chan = status & 0x0F;
        u8 pitch = comm_read();
        u8 velocity = comm_read();
        midi_noteOn(
            chan,
            pitch,
            velocity);
    } else if ((status & 0xF0) == 0x80) {
        u8 chan = status & 0x0F;
        u8 pitch = comm_read();
        u8 velocity = comm_read();
        midi_noteOff(chan);
    } else {
        strcpy(lastError, "Unknown Status F0");
    }
}

char* interface_lastError(void)
{
    return &lastError[0];
}

void interface_clearError(void)
{
}
