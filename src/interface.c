#include <comm.h>
#include <interface.h>
#include <midi.h>
#include <string.h>
#include <synth.h>

static u8 lastUnknownStatus = 0;
static ControlChange lastUnknownControlChange;

#define STATUS_CHANNEL(status) status & 0x0F
#define STATUS_EVENT(status) status >> 4

#define EVENT_NOTE_ON 0x9
#define EVENT_NODE_OFF 0x8
#define EVENT_CC 0xB

#define CC_VOLUME 0x7
#define CC_PAN 0xA

static void noteOn(u8 status);
static void noteOff(u8 status);
static void controlChange(u8 status);

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
    u8 event = STATUS_EVENT(status);
    if (event == EVENT_NOTE_ON) {
        noteOn(status);
    } else if (event == EVENT_NODE_OFF) {
        noteOff(status);
    } else if (event == EVENT_CC) {
        controlChange(status);
    } else {
        lastUnknownStatus = status;
    }
}

ControlChange* interface_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
}

static void controlChange(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    u8 controller = comm_read();
    u8 value = comm_read();
    if (controller == CC_VOLUME) {
        midi_channelVolume(chan, value);
    } else if (controller == CC_PAN) {
        midi_pan(chan, value);
    } else {
        lastUnknownControlChange.controller = controller;
        lastUnknownControlChange.value = value;
    }
}

static void noteOn(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    u8 pitch = comm_read();
    u8 velocity = comm_read();
    midi_noteOn(
        chan,
        pitch,
        velocity);
}

static void noteOff(u8 status)
{
    u8 chan = STATUS_CHANNEL(status);
    comm_read();
    comm_read();
    midi_noteOff(chan);
}

u8 interface_lastUnknownStatus(void)
{
    return lastUnknownStatus;
}
