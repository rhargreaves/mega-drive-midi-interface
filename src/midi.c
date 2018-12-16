#include "midi.h"
#include "midi_fm.h"
#include "midi_nop.h"
#include "midi_psg.h"
#include "psg_chip.h"
#include "synth.h"

typedef struct VTable VTable;

struct VTable {
    void (*noteOn)(u8 chan, u8 pitch, u8 velocity);
    void (*noteOff)(u8 chan);
    void (*channelVolume)(u8 chan, u8 volume);
    void (*pitchBend)(u8 chan, u16 bend);
};

static const VTable PSG_VTable = { midi_psg_noteOn, midi_psg_noteOff,
    midi_psg_channelVolume, midi_psg_pitchBend };

static const VTable FM_VTable = { midi_fm_noteOn, midi_fm_noteOff,
    midi_fm_channelVolume, midi_fm_pitchBend };

static const VTable NOP_VTable = { midi_nop_noteOn, midi_nop_noteOff,
    midi_nop_channelVolume, midi_nop_pitchBend };

static const VTable* CHANNEL_OPS[16]
    = { &FM_VTable, &FM_VTable, &FM_VTable, &FM_VTable, &FM_VTable, &FM_VTable,
          &PSG_VTable, &PSG_VTable, &PSG_VTable, &PSG_VTable, &NOP_VTable,
          &NOP_VTable, &NOP_VTable, &NOP_VTable, &NOP_VTable, &NOP_VTable };

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

void midi_pitchBend(u8 chan, u16 bend)
{
    CHANNEL_OPS[chan]->pitchBend(chan, bend);
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
