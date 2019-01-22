#include "midi.h"
#include "midi_fm.h"
#include "midi_nop.h"
#include "midi_psg.h"
#include "psg_chip.h"
#include "synth.h"
#include <stdbool.h>

#define RANGE(value, range) (value / (128 / range))

typedef struct Timing Timing;

struct Timing {
    u16 beat;
    u16 clock;
};

typedef struct VTable VTable;

struct VTable {
    void (*noteOn)(u8 chan, u8 pitch, u8 velocity);
    void (*noteOff)(u8 chan, u8 pitch);
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

static bool polyphonic;
static u8 polyphonicPitches[MAX_FM_CHANS];
static ControlChange lastUnknownControlChange;
static bool overflow;
static Timing timing;

static void allNotesOff(u8 chan);
static void pooledNoteOn(u8 chan, u8 pitch, u8 velocity);
static void pooledNoteOff(u8 chan, u8 pitch);
static void channelVolume(u8 chan, u8 volume);
static void pan(u8 chan, u8 pan);
static void setPolyphonic(bool state);
static void cc(u8 chan, u8 controller, u8 value);

void midi_reset(void)
{
    timing.clock = 0;
    timing.beat = 0;
    overflow = false;
    polyphonic = false;
    lastUnknownControlChange.controller = 0;
    lastUnknownControlChange.value = 0;
    for (u16 c = 0; c <= MAX_FM_CHAN; c++) {
        polyphonicPitches[c] = 0;
    }
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (polyphonic) {
        pooledNoteOn(chan, pitch, velocity);
    } else {
        CHANNEL_OPS[chan]->noteOn(chan, pitch, velocity);
    }
}

void midi_noteOff(u8 chan, u8 pitch)
{
    if (polyphonic) {
        pooledNoteOff(chan, pitch);
    } else {
        CHANNEL_OPS[chan]->noteOff(chan, pitch);
    }
}

void midi_cc(u8 chan, u8 controller, u8 value)
{
    if (polyphonic) {
        for (u8 c = 0; c <= MAX_FM_CHAN; c++) {
            cc(c, controller, value);
        }
    } else {
        cc(chan, controller, value);
    }
}

bool midi_overflow(void)
{
    return overflow;
}

static void cc(u8 chan, u8 controller, u8 value)
{
    switch (controller) {
    case CC_VOLUME:
        channelVolume(chan, value);
        break;
    case CC_PAN:
        pan(chan, value);
        break;
    case CC_ALL_NOTES_OFF:
        allNotesOff(chan);
        break;
    case CC_GENMDM_FM_ALGORITHM:
        synth_algorithm(chan, RANGE(value, 8));
        break;
    case CC_GENMDM_FM_FEEDBACK:
        synth_feedback(chan, RANGE(value, 8));
        break;
    case CC_GENMDM_TOTAL_LEVEL_OP1:
    case CC_GENMDM_TOTAL_LEVEL_OP2:
    case CC_GENMDM_TOTAL_LEVEL_OP3:
    case CC_GENMDM_TOTAL_LEVEL_OP4:
        synth_operatorTotalLevel(
            chan, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        synth_operatorMultiple(
            chan, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        synth_operatorDetune(
            chan, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        synth_operatorRateScaling(
            chan, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        synth_operatorAttackRate(
            chan, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_FIRST_DECAY_RATE_OP1:
    case CC_GENMDM_FIRST_DECAY_RATE_OP2:
    case CC_GENMDM_FIRST_DECAY_RATE_OP3:
    case CC_GENMDM_FIRST_DECAY_RATE_OP4:
        synth_operatorFirstDecayRate(chan,
            controller - CC_GENMDM_FIRST_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SECOND_DECAY_RATE_OP1:
    case CC_GENMDM_SECOND_DECAY_RATE_OP2:
    case CC_GENMDM_SECOND_DECAY_RATE_OP3:
    case CC_GENMDM_SECOND_DECAY_RATE_OP4:
        synth_operatorSecondDecayRate(chan,
            controller - CC_GENMDM_SECOND_DECAY_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SECOND_AMPLITUDE_OP1:
    case CC_GENMDM_SECOND_AMPLITUDE_OP2:
    case CC_GENMDM_SECOND_AMPLITUDE_OP3:
    case CC_GENMDM_SECOND_AMPLITUDE_OP4:
        synth_operatorSecondaryAmplitude(chan,
            controller - CC_GENMDM_SECOND_AMPLITUDE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        synth_operatorReleaseRate(
            chan, controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        synth_operatorAmplitudeModulation(chan,
            controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_GLOBAL_LFO_ENABLE:
        synth_enableLfo(RANGE(value, 2));
        break;
    case CC_GENMDM_GLOBAL_LFO_FREQUENCY:
        synth_globalLfoFrequency(RANGE(value, 8));
        break;
    case CC_GENMDM_AMS:
        synth_ams(chan, RANGE(value, 4));
        break;
    case CC_GENMDM_FMS:
        synth_fms(chan, RANGE(value, 8));
        break;
    case CC_POLYPHONIC_MODE:
        setPolyphonic(RANGE(value, 2) != 0);
        break;
    default:
        lastUnknownControlChange.controller = controller;
        lastUnknownControlChange.value = value;
        break;
    }
}

void midi_pitchBend(u8 chan, u16 bend)
{
    CHANNEL_OPS[chan]->pitchBend(chan, bend);
}

bool midi_getPolyphonic(void)
{
    return polyphonic;
}

void midi_clock(void)
{
    timing.clock++;
    if (timing.clock == 6) {
        timing.beat++;
        timing.clock = 0;
    }
}

u16 midi_beat(void)
{
    return timing.beat;
}

void midi_start(void)
{
    midi_position(0);
}

void midi_position(u16 beat)
{
    timing.clock = 0;
    timing.beat = beat;
}

ControlChange* midi_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
}

static void channelVolume(u8 chan, u8 volume)
{
    CHANNEL_OPS[chan]->channelVolume(chan, volume);
}

static void pan(u8 chan, u8 pan)
{
    if (pan > 96) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}

static void allNotesOff(u8 chan)
{
    CHANNEL_OPS[chan]->noteOff(chan, 0);
}

static void setPolyphonic(bool state)
{
    polyphonic = state;
}

static void pooledNoteOn(u8 chan, u8 pitch, u8 velocity)
{
    for (u8 c = 0; c < MAX_FM_CHANS; c++) {
        if (polyphonicPitches[c] == 0) {
            polyphonicPitches[c] = pitch;
            CHANNEL_OPS[chan]->noteOn(c, pitch, velocity);
            overflow = false;
            return;
        }
    }
    overflow = true;
}

static void pooledNoteOff(u8 chan, u8 pitch)
{
    for (u8 c = 0; c < MAX_FM_CHANS; c++) {
        if (polyphonicPitches[c] == pitch) {
            polyphonicPitches[c] = 0;
            CHANNEL_OPS[chan]->noteOff(c, pitch);
        }
    }
}
