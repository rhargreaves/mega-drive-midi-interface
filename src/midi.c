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

typedef struct ChannelMapping ChannelMapping;

struct ChannelMapping {
    const VTable* ops;
    u8 channel;
};

static ChannelState channelState[DEV_CHANS];

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

typedef struct MidiChannel MidiChannel;

struct MidiChannel {
    u8 volume;
    u8 program;
    u8 pan;
};

static MidiChannel midiChannels[MIDI_CHANNELS];

static u8 polyphonicPitches[MAX_FM_CHANS];
static ControlChange lastUnknownControlChange;
static Timing timing;
static bool polyphonic;
static bool overflow;
static bool dynamicMode;

static void allNotesOff(u8 chan);
static void pooledNoteOn(u8 chan, u8 pitch, u8 velocity);
static void pooledNoteOff(u8 chan, u8 pitch);
static void setPolyphonic(bool state);
static void cc(u8 chan, u8 controller, u8 value);
static void generalMidiReset(void);
static ChannelMapping* channelMapping(u8 midiChannel);
static void sendPong(void);
static void setDynamicMode(bool enabled);

static void initChannelState(void)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        bool isFm = i < DEV_CHAN_MIN_PSG;
        state->deviceChannel = isFm ? i : i - DEV_CHAN_MIN_PSG;
        state->ops = isFm ? &FM_VTable : &PSG_VTable;
        state->noteOn = false;
        state->midiProgram = 0;
        state->midiChannel = 0;
        state->midiKey = 0;
        state->midiPan = DEFAULT_MIDI_PAN;
        state->midiVolume = MAX_MIDI_VOLUME;
    }
}

static void resetAllState(void)
{
    memset(&timing, 0, sizeof(Timing));
    memset(&lastUnknownControlChange, 0, sizeof(ControlChange));
    memset(&polyphonicPitches, 0, sizeof(polyphonicPitches));
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        midiChannels[i].program = 0;
        midiChannels[i].pan = DEFAULT_MIDI_PAN;
        midiChannels[i].volume = MAX_MIDI_VOLUME;
    }
    initChannelState();
}

void midi_init(
    Channel** defaultPresets, PercussionPreset** defaultPercussionPresets)
{
    memcpy(&ChannelMappings, DefaultChannelMappings, sizeof(ChannelMappings));
    overflow = false;
    polyphonic = false;
    dynamicMode = false;
    resetAllState();
    midi_psg_init();
    midi_fm_init(defaultPresets, defaultPercussionPresets);
}

static ChannelMapping* channelMapping(u8 midiChannel)
{
    return &ChannelMappings[midiChannel];
}

static ChannelState* findChannelPlayingNote(u8 midiChannel, u8 pitch)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* chan = &channelState[i];
        if (chan->noteOn && chan->midiChannel == midiChannel
            && chan->midiKey == pitch) {
            return chan;
        }
    }
    return NULL;
}

static bool isPsgNoise(ChannelState* state)
{
    return state->ops == &PSG_VTable
        && state->deviceChannel == DEV_CHAN_PSG_NOISE;
}

static bool isPsgAndIncomingChanIsPercussive(
    ChannelState* state, u8 incomingChan)
{
    return state->ops == &PSG_VTable
        && incomingChan == GENERAL_MIDI_PERCUSSION_CHANNEL;
}

static bool isChannelSuitable(ChannelState* state, u8 incomingMidiChan)
{
    return !state->noteOn && !isPsgNoise(state)
        && !isPsgAndIncomingChanIsPercussive(state, incomingMidiChan);
}

static ChannelState* findFreeChannel(u8 incomingMidiChan)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* chan = &channelState[i];
        if (chan->midiChannel == incomingMidiChan) {
            if (isChannelSuitable(chan, incomingMidiChan)) {
                return chan;
            }
        }
    }
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* chan = &channelState[i];
        if (isChannelSuitable(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static bool tooManyPercussiveNotes(u8 midiChan)
{
    const u8 MAX_POLYPHONY = 2;

    if (midiChan != GENERAL_MIDI_PERCUSSION_CHANNEL) {
        return false;
    }

    u16 counter = 0;
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* chan = &channelState[i];
        if (chan->midiChannel == GENERAL_MIDI_PERCUSSION_CHANNEL
            && chan->noteOn) {
            counter++;
        }
        if (counter >= MAX_POLYPHONY) {
            return true;
        }
    }
    return false;
}

static void updateChannelVolume(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->midiVolume != midiChannel->volume) {
        state->ops->channelVolume(state->deviceChannel, midiChannel->volume);
        state->midiVolume = midiChannel->volume;
    }
}

static void updateChannelPan(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->midiPan != midiChannel->pan) {
        state->ops->pan(state->deviceChannel, midiChannel->pan);
        state->midiPan = midiChannel->pan;
    }
}

static void updateProgram(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->midiProgram != midiChannel->program) {
        state->ops->program(state->deviceChannel, midiChannel->program);
        state->midiProgram = midiChannel->program;
    }
}

static void dynamicNoteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (tooManyPercussiveNotes(chan)) {
        return;
    }
    ChannelState* state = findFreeChannel(chan);
    if (state == NULL) {
        return;
    }
    state->midiChannel = chan;
    MidiChannel* midiChannel = &midiChannels[chan];
    midi_fm_percussive(
        state->deviceChannel, chan == GENERAL_MIDI_PERCUSSION_CHANNEL);
    updateChannelVolume(midiChannel, state);
    updateChannelPan(midiChannel, state);
    updateProgram(midiChannel, state);
    state->midiKey = pitch;
    state->noteOn = true;
    state->ops->noteOn(state->deviceChannel, pitch, velocity);
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (polyphonic) {
        pooledNoteOn(chan, pitch, velocity);
    } else if (dynamicMode) {
        dynamicNoteOn(chan, pitch, velocity);
    } else {
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->noteOn(mapping->channel, pitch, velocity);
    }
}

static void dynamicNoteOff(u8 chan, u8 pitch)
{
    ChannelState* state = findChannelPlayingNote(chan, pitch);
    if (state == NULL) {
        return;
    }
    state->noteOn = false;
    state->midiKey = 0;
    state->ops->noteOff(state->deviceChannel, pitch);
}

void midi_noteOff(u8 chan, u8 pitch)
{
    if (polyphonic) {
        pooledNoteOff(chan, pitch);
    } else if (dynamicMode) {
        dynamicNoteOff(chan, pitch);
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

static void channelPan(u8 chan, u8 pan)
{
    if (dynamicMode) {
        MidiChannel* midiChannel = &midiChannels[chan];
        midiChannel->pan = pan;
        for (u8 i = 0; i < DEV_CHANS; i++) {
            ChannelState* state = &channelState[i];
            if (state->midiChannel == chan && state->noteOn) {
                updateChannelPan(midiChannel, state);
            }
        }
    } else {
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->pan(mapping->channel, pan);
    }
}

static void channelVolume(u8 chan, u8 volume)
{
    if (dynamicMode) {
        MidiChannel* midiChannel = &midiChannels[chan];
        midiChannel->volume = volume;
        for (u8 i = 0; i < DEV_CHANS; i++) {
            ChannelState* state = &channelState[i];
            if (state->midiChannel == chan && state->noteOn) {
                updateChannelVolume(midiChannel, state);
            }
        }
    } else {
        ChannelMapping* mapping = channelMapping(chan);
        mapping->ops->channelVolume(mapping->channel, volume);
    }
}

static void cc(u8 chan, u8 controller, u8 value)
{
    ChannelMapping* mapping = channelMapping(chan);
    switch (controller) {
    case CC_VOLUME:
        channelVolume(chan, value);
        break;
    case CC_PAN:
        channelPan(chan, value);
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
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->program = program;
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

bool midi_dynamicMode(void)
{
    return dynamicMode;
}

ChannelState* midi_dynamicModeMappings(void)
{
    return channelState;
}

ControlChange* midi_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
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
        midi_remapChannel(data[4], data[5]);
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

void midi_remapChannel(u8 midiChannel, u8 deviceChannel)
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
    resetAllState();
}

static void setDynamicMode(bool enabled)
{
    dynamicMode = enabled;
}
