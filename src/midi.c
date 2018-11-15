#include "midi.h"
#include "midi_fm.h"
#include "psg_chip.h"
#include "synth.h"

static void midi_psg_noteOn(u8 chan, u8 pitch, u8 velocity);
static void midi_psg_noteOff(u8 chan);
static void midi_psg_channelVolume(u8 chan, u8 volume);
static void midi_nop_noteOn(u8 chan, u8 pitch, u8 velocity);
static void midi_nop_noteOff(u8 chan);
static void midi_nop_channelVolume(u8 chan, u8 volume);

static const u16 FREQUENCIES[] = {
    8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 21, 22, 23, 25,
    26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73,
    78, 82, 87, 93, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175,
    185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392,
    415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880,
    932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760,
    1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
    3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040,
    7459, 7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
};

static const u8 ATTENUATIONS[] = {
    15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 5, 5, 5,
    5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

typedef struct VTable VTable;

struct VTable {
    void (*noteOn)(u8 chan, u8 pitch, u8 velocity);
    void (*noteOff)(u8 chan);
    void (*channelVolume)(u8 chan, u8 volume);
};

static const VTable PSG_VTable = {
    midi_psg_noteOn,
    midi_psg_noteOff,
    midi_psg_channelVolume
};

static const VTable FM_VTable = {
    midi_fm_noteOn,
    midi_fm_noteOff,
    midi_fm_channelVolume
};

static const VTable NOP_VTable = {
    midi_nop_noteOn,
    midi_nop_noteOff,
    midi_nop_channelVolume
};

static const VTable* CHANNEL_OPS[16] = {
    &FM_VTable,
    &FM_VTable,
    &FM_VTable,
    &FM_VTable,
    &FM_VTable,
    &FM_VTable,
    &PSG_VTable,
    &PSG_VTable,
    &PSG_VTable,
    &PSG_VTable,
    &NOP_VTable,
    &NOP_VTable,
    &NOP_VTable,
    &NOP_VTable,
    &NOP_VTable,
    &NOP_VTable
};

static void midi_nop_noteOn(u8 chan, u8 pitch, u8 velocity)
{
}

static void midi_nop_noteOff(u8 chan)
{
}

static void midi_nop_channelVolume(u8 chan, u8 volume)
{
}

static void midi_psg_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    psg_noteOn(chan - MIN_PSG_CHAN, FREQUENCIES[pitch]);
}

static void midi_psg_noteOff(u8 chan)
{
    psg_noteOff(chan - MIN_PSG_CHAN);
}

static void midi_psg_channelVolume(u8 chan, u8 volume)
{
    psg_attenuation(chan - MIN_PSG_CHAN, ATTENUATIONS[volume]);
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    CHANNEL_OPS[chan]->noteOn(chan, pitch, velocity);
}

void midi_noteOff(u8 chan)
{
    CHANNEL_OPS[chan]->noteOff(chan);
}

void midi_channelVolume(u8 chan, u8 volume)
{
    CHANNEL_OPS[chan]->channelVolume(chan, volume);
}

void midi_pan(u8 chan, u8 pan)
{
    if (pan > 96) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}
