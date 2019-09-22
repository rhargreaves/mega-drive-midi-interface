#include "midi.h"
#include "comm.h"
#include "memcmp.h"
#include "memory.h"
#include "midi_fm.h"
#include "midi_nop.h"
#include "midi_psg.h"
#include "midi_sender.h"
#include "psg_chip.h"
#include "synth.h"
#include <stdbool.h>

#define RANGE(value, range) (value / (128 / range))
#define CHANNEL_UNASSIGNED 0x7F
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

static const u8 SYSEX_EXTENDED_MANU_ID_SECTION = 0x00;
static const u8 SYSEX_UNUSED_EUROPEAN_SECTION = 0x22;
static const u8 SYSEX_UNUSED_MANU_ID = 0x77;

typedef struct VTable VTable;

struct VTable {
    void (*noteOn)(u8 chan, u8 pitch, u8 velocity);
    void (*noteOff)(u8 chan, u8 pitch);
    void (*channelVolume)(u8 chan, u8 volume);
    void (*pitchBend)(u8 chan, u16 bend);
    void (*program)(u8 chan, u8 program);
    void (*allNotesOff)(u8 chan);
    void (*pan)(u8 chan, u8 pan);
};

typedef struct ChannelMapping ChannelMapping;

struct ChannelMapping {
    const VTable* ops;
    u8 channel;
};

typedef struct ChannelState ChannelState;

struct ChannelState {
    bool noteOn;
    u8 destinationChannel;
};

static ChannelState channelState[MIDI_CHANNELS];

static const VTable PSG_VTable = { midi_psg_noteOn, midi_psg_noteOff,
    midi_psg_channelVolume, midi_psg_pitchBend, midi_psg_program,
    midi_psg_allNotesOff, midi_nop_pan };

static const VTable FM_VTable = { midi_fm_noteOn, midi_fm_noteOff,
    midi_fm_channelVolume, midi_fm_pitchBend, midi_fm_program,
    midi_fm_allNotesOff, midi_fm_pan };

static const VTable NOP_VTable = { midi_nop_noteOn, midi_nop_noteOff,
    midi_nop_channelVolume, midi_nop_pitchBend, midi_nop_program,
    midi_nop_allNotesOff, midi_nop_pan };

static const ChannelMapping DefaultChannelMappings[MIDI_CHANNELS]
    = { { &FM_VTable, 0 }, { &FM_VTable, 1 }, { &FM_VTable, 2 },
          { &FM_VTable, 3 }, { &FM_VTable, 4 }, { &FM_VTable, 5 },
          { &PSG_VTable, 0 }, { &PSG_VTable, 1 }, { &PSG_VTable, 2 },
          { &PSG_VTable, 3 }, { &PSG_VTable, 0 }, { &PSG_VTable, 1 },
          { &PSG_VTable, 2 }, { &PSG_VTable, 0 }, { &PSG_VTable, 1 },
          { &PSG_VTable, 2 } };

static ChannelMapping ChannelMappings[MIDI_CHANNELS];

static u8 polyphonicPitches[MAX_FM_CHANS];
static ControlChange lastUnknownControlChange;
static Timing timing;
static bool polyphonic;
static bool overflow;
static bool dynamicMode;

static void allNotesOff(u8 chan);
static void pooledNoteOn(u8 chan, u8 pitch, u8 velocity);
static void pooledNoteOff(u8 chan, u8 pitch);
static void channelVolume(u8 chan, u8 volume);
static void setPolyphonic(bool state);
static void cc(u8 chan, u8 controller, u8 value);
static void generalMidiReset(void);
static ChannelMapping* channelMapping(u8 midiChannel);
static void remapChannel(u8 midiChannel, u8 deviceChannel);
static void sendPong(void);
static void setDynamicMode(bool enabled);

void midi_init(
    Channel** defaultPresets, PercussionPreset** defaultPercussionPresets)
{
    memcpy(&ChannelMappings, DefaultChannelMappings, sizeof(ChannelMappings));
    memset(&timing, 0, sizeof(Timing));
    memset(&lastUnknownControlChange, 0, sizeof(ControlChange));
    memset(&polyphonicPitches, 0, sizeof(polyphonicPitches));
    overflow = false;
    polyphonic = false;

    midi_psg_init();
    midi_fm_init(defaultPresets, defaultPercussionPresets);
}

static ChannelMapping* channelMapping(u8 midiChannel)
{
    return &ChannelMappings[midiChannel];
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (polyphonic) {
        pooledNoteOn(chan, pitch, velocity);
    } else if (dynamicMode) {
        ChannelState* state = &channelState[chan];
        while (state->noteOn) {
            chan++;
            state = &channelState[chan];
        }
        state->noteOn = true;
        state->destinationChannel = chan;
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->noteOn(mapping->channel, pitch, velocity);

    } else {
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->noteOn(mapping->channel, pitch, velocity);
    }
}

void midi_noteOff(u8 chan, u8 pitch)
{
    if (polyphonic) {
        pooledNoteOff(chan, pitch);
    } else if (dynamicMode) {
        ChannelState* state = &channelState[chan];
        if (state->noteOn) {
            ChannelMapping* mapping = channelMapping(state->destinationChannel);
            mapping->ops->noteOff(mapping->channel, pitch);
            state->noteOn = false;
        }
    } else {
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->noteOff(mapping->channel, pitch);
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
    ChannelMapping* mapping = channelMapping(chan);
    switch (controller) {
    case CC_VOLUME:
        channelVolume(chan, value);
        break;
    case CC_PAN:
        mapping->ops->pan(mapping->channel, value);
        break;
    case CC_ALL_NOTES_OFF:
        allNotesOff(chan);
        break;
    case CC_GENMDM_FM_ALGORITHM:
        synth_algorithm(mapping->channel, RANGE(value, 8));
        break;
    case CC_GENMDM_FM_FEEDBACK:
        synth_feedback(mapping->channel, RANGE(value, 8));
        break;
    case CC_GENMDM_TOTAL_LEVEL_OP1:
    case CC_GENMDM_TOTAL_LEVEL_OP2:
    case CC_GENMDM_TOTAL_LEVEL_OP3:
    case CC_GENMDM_TOTAL_LEVEL_OP4:
        synth_operatorTotalLevel(
            mapping->channel, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        synth_operatorMultiple(mapping->channel,
            controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        synth_operatorDetune(mapping->channel,
            controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        synth_operatorRateScaling(mapping->channel,
            controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        synth_operatorAttackRate(mapping->channel,
            controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_FIRST_DECAY_RATE_OP1:
    case CC_GENMDM_FIRST_DECAY_RATE_OP2:
    case CC_GENMDM_FIRST_DECAY_RATE_OP3:
    case CC_GENMDM_FIRST_DECAY_RATE_OP4:
        synth_operatorFirstDecayRate(mapping->channel,
            controller - CC_GENMDM_FIRST_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SECOND_DECAY_RATE_OP1:
    case CC_GENMDM_SECOND_DECAY_RATE_OP2:
    case CC_GENMDM_SECOND_DECAY_RATE_OP3:
    case CC_GENMDM_SECOND_DECAY_RATE_OP4:
        synth_operatorSecondDecayRate(mapping->channel,
            controller - CC_GENMDM_SECOND_DECAY_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SECOND_AMPLITUDE_OP1:
    case CC_GENMDM_SECOND_AMPLITUDE_OP2:
    case CC_GENMDM_SECOND_AMPLITUDE_OP3:
    case CC_GENMDM_SECOND_AMPLITUDE_OP4:
        synth_operatorSecondaryAmplitude(mapping->channel,
            controller - CC_GENMDM_SECOND_AMPLITUDE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        synth_operatorReleaseRate(mapping->channel,
            controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        synth_operatorAmplitudeModulation(mapping->channel,
            controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        synth_operatorSsgEg(mapping->channel, controller - CC_GENMDM_SSG_EG_OP1,
            RANGE(value, 16));
        break;
    case CC_GENMDM_GLOBAL_LFO_ENABLE:
        synth_enableLfo(RANGE(value, 2));
        break;
    case CC_GENMDM_GLOBAL_LFO_FREQUENCY:
        synth_globalLfoFrequency(RANGE(value, 8));
        break;
    case CC_GENMDM_AMS:
        synth_ams(mapping->channel, RANGE(value, 4));
        break;
    case CC_GENMDM_FMS:
        synth_fms(mapping->channel, RANGE(value, 8));
        break;
    case CC_GENMDM_STEREO:
        synth_stereo(mapping->channel, RANGE(value, 4));
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
    ChannelMapping* mapping = channelMapping(chan);
    mapping->ops->pitchBend(mapping->channel, bend);
}

bool midi_getPolyphonic(void)
{
    return polyphonic;
}

void midi_clock(void)
{
    timing.clocks++;
    if (++timing.clock == 6) {
        timing.clock = 0;
        if (++timing.sixteenth == 4) {
            timing.sixteenth = 0;
            if (++timing.barBeat == 4) {
                timing.bar++;
                timing.barBeat = 0;
            }
        }
    }
}

void midi_start(void)
{
    midi_position(0);
}

/* midiBeat = 1/16th note = 6 clocks */
void midi_position(u16 midiBeat)
{
    const u16 BEATS_IN_BAR = 4;

    timing.clocks = midiBeat * 6;
    timing.clock = timing.clocks % 6;
    timing.bar = midiBeat / 16;
    timing.barBeat = (midiBeat / 4) % BEATS_IN_BAR;
    timing.sixteenth = midiBeat % 4;
}

void midi_program(u8 chan, u8 program)
{
    ChannelMapping* mapping = channelMapping(chan);
    mapping->ops->program(mapping->channel, program);
}

void midi_mappings(u8* mappingDest)
{
    const u8 UNMAPPED = 0x7E;
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        ChannelMapping* mapping = &ChannelMappings[i];
        if (mapping->ops == &FM_VTable)
            mappingDest[i] = mapping->channel;
        else if (mapping->ops == &PSG_VTable)
            mappingDest[i] = mapping->channel + MIN_PSG_CHAN;
        else {
            mappingDest[i] = UNMAPPED;
        }
    }
}

ControlChange* midi_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
}

static void channelVolume(u8 chan, u8 volume)
{
    ChannelMapping* mapping = channelMapping(chan);
    mapping->ops->channelVolume(mapping->channel, volume);
}

static void allNotesOff(u8 chan)
{
    ChannelMapping* mapping = channelMapping(chan);
    mapping->ops->allNotesOff(mapping->channel);
}

static void setPolyphonic(bool state)
{
    polyphonic = state;
}

static void pooledNoteOn(u8 chan, u8 pitch, u8 velocity)
{
    for (u8 c = 0; c < MAX_FM_CHANS; c++) {
        ChannelMapping* mapping = channelMapping(c);
        if (polyphonicPitches[c] == 0) {
            polyphonicPitches[c] = pitch;
            mapping->ops->noteOn(mapping->channel, pitch, velocity);
            overflow = false;
            return;
        }
    }
    overflow = true;
}

static void pooledNoteOff(u8 chan, u8 pitch)
{
    for (u8 c = 0; c < MAX_FM_CHANS; c++) {
        ChannelMapping* mapping = channelMapping(c);
        if (polyphonicPitches[c] == pitch) {
            polyphonicPitches[c] = 0;
            mapping->ops->noteOff(mapping->channel, pitch);
        }
    }
}

Timing* midi_timing(void)
{
    return &timing;
}

static bool sysex_valid(const u8* sourceData, const u16 sourceLength,
    const u8* commandSequenceData, const u16 commandSequenceLength,
    const u16 commandParametersLength)
{
    if (sourceLength != (commandSequenceLength + commandParametersLength)) {
        return false;
    }
    return memcmp(sourceData, commandSequenceData, commandSequenceLength) == 0;
}

void midi_sysex(const u8* data, u16 length)
{
    const u8 SYSEX_REMAP_COMMAND_ID = 0x00;
    const u8 SYSEX_PING_COMMAND_ID = 0x01;
    const u8 SYSEX_DYNAMIC_COMMAND_ID = 0x03;

    const u8 GENERAL_MIDI_RESET_SEQUENCE[] = { 0x7E, 0x7F, 0x09, 0x01 };

    const u8 REMAP_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_REMAP_COMMAND_ID };

    const u8 PING_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_PING_COMMAND_ID };

    const u8 DYNAMIC_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_DYNAMIC_COMMAND_ID };

    if (sysex_valid(data, length, GENERAL_MIDI_RESET_SEQUENCE,
            LENGTH_OF(GENERAL_MIDI_RESET_SEQUENCE), 0)) {
        generalMidiReset();
    } else if (sysex_valid(data, length, REMAP_SEQUENCE,
                   LENGTH_OF(REMAP_SEQUENCE), 2)) {
        remapChannel(data[4], data[5]);
    } else if (sysex_valid(
                   data, length, PING_SEQUENCE, LENGTH_OF(PING_SEQUENCE), 0)) {
        sendPong();
    } else if (sysex_valid(data, length, DYNAMIC_SEQUENCE,
                   LENGTH_OF(DYNAMIC_SEQUENCE), 1)) {
        setDynamicMode((bool)data[4]);
    }
}

static void sendPong(void)
{
    const u8 SYSEX_PONG_COMMAND_ID = 0x02;

    const u8 pongSequence[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_PONG_COMMAND_ID };

    midi_sender_send_sysex(pongSequence, sizeof(pongSequence));
}

static void remapChannel(u8 midiChannel, u8 deviceChannel)
{
    ChannelMapping* mapping = channelMapping(midiChannel);
    if (deviceChannel < MIN_PSG_CHAN) {
        mapping->channel = deviceChannel;
        mapping->ops = &FM_VTable;
        if (midiChannel == 9) {
            midi_fm_percussive(deviceChannel, true);
        }
    } else if (deviceChannel <= MAX_PSG_CHAN) {
        mapping->channel = deviceChannel - MIN_PSG_CHAN;
        mapping->ops = &PSG_VTable;
    } else {
        mapping->channel = 0;
        mapping->ops = &NOP_VTable;
    }
}

static void generalMidiReset(void)
{
    for (u8 chan = 0; chan < MIDI_CHANNELS; chan++) {
        allNotesOff(chan);
    }
}

static void setDynamicMode(bool enabled)
{
    dynamicMode = enabled;
}
