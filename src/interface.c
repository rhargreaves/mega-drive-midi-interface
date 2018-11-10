#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

static u8 lastUnknownStatus = 0;

static void noteOn(u8 status);
static void noteOff(u8 status);

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
    u8 upperStatus = status >> 4;
    if (upperStatus == 0x9) {
        noteOn(status);
    } else if (upperStatus == 0x8) {
        noteOff(status);
    } else {
        lastUnknownStatus = status;
    }
}

static void noteOn(u8 status)
{
    u8 chan = status & 0x0F;
    u8 pitch = comm_read();
    u8 velocity = comm_read();
    midi_noteOn(
        chan,
        pitch,
        velocity);
}

static void noteOff(u8 status)
{
    u8 chan = status & 0x0F;
    comm_read();
    comm_read();
    midi_noteOff(chan);
}

u8 interface_lastUnknownStatus(void)
{
    return lastUnknownStatus;
}
