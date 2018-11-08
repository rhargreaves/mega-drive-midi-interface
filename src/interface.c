#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

static char lastUnknownStatusText[20];
static u8 lastUnknownStatus = 0;

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
        lastUnknownStatus = status;
    }
}

char* interface_lastError(void)
{
    if (lastUnknownStatus != 0) {
        sprintf(lastUnknownStatusText, "Unknown Status %02X", lastUnknownStatus);
        return lastUnknownStatusText;
    }
    return NULL;
}

void interface_clearError(void)
{
    lastUnknownStatus = 0;
}
