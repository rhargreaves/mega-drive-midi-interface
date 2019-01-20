#include "interface.h"
#include "comm.h"
#include "midi.h"
#include "synth.h"
#include <string.h>

#define STATUS_LOWER(status) (status & 0x0F)
#define STATUS_UPPER(status) (status >> 4)

#define EVENT_PITCH_BEND 0xE
#define EVENT_NOTE_ON 0x9
#define EVENT_NODE_OFF 0x8
#define EVENT_CC 0xB
#define EVENT_SYSTEM 0xF

#define SYSTEM_CLOCK 0x8

static u8 lastUnknownStatus = 0;
static u16 beat = 0;
static u16 clock = 0;

static void noteOn(u8 status);
static void noteOff(u8 status);
static void controlChange(u8 status);
static void pitchBend(u8 status);
static void systemMessage(u8 status);

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
    u8 event = STATUS_UPPER(status);
    switch (event) {
    case EVENT_NOTE_ON:
        noteOn(status);
        break;
    case EVENT_NODE_OFF:
        noteOff(status);
        break;
    case EVENT_CC:
        controlChange(status);
        break;
    case EVENT_PITCH_BEND:
        pitchBend(status);
        break;
    case EVENT_SYSTEM:
        systemMessage(status);
        break;
    default:
        lastUnknownStatus = status;
        break;
    }
}

u16 interface_beat(void)
{
    return beat;
}

u8 interface_lastUnknownStatus(void)
{
    return lastUnknownStatus;
}

static void controlChange(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 controller = comm_read();
    u8 value = comm_read();
    midi_cc(chan, controller, value);
}

bool interface_polyphonic(void)
{
    return midi_getPolyphonic();
}

static void noteOn(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch = comm_read();
    u8 velocity = comm_read();
    midi_noteOn(chan, pitch, velocity);
}

static void noteOff(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u8 pitch = comm_read();
    comm_read();
    midi_noteOff(chan, pitch);
}

static void pitchBend(u8 status)
{
    u8 chan = STATUS_LOWER(status);
    u16 lowerBend = comm_read();
    u16 upperBend = comm_read();
    u16 bend = (upperBend << 7) + lowerBend;
    midi_pitchBend(chan, bend);
}

static void systemMessage(u8 status)
{
    u8 type = STATUS_LOWER(status);
    switch (type) {
    case SYSTEM_CLOCK:
        clock++;
        if (clock == 6) {
            beat++;
            clock = 0;
        }
        break;
    default:
        lastUnknownStatus = status;
        break;
    }
}
