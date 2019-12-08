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
#define CHANNEL_UNASSIGNED 0xFF
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

static const u8 SYSEX_EXTENDED_MANU_ID_SECTION = 0x00;
static const u8 SYSEX_UNUSED_EUROPEAN_SECTION = 0x22;
static const u8 SYSEX_UNUSED_MANU_ID = 0x77;

static ChannelState channelState[DEV_CHANS];

static const VTable PSG_VTable = { midi_psg_noteOn, midi_psg_noteOff,
    midi_psg_channelVolume, midi_psg_pitchBend, midi_psg_program,
    midi_psg_allNotesOff, midi_nop_pan };

static const VTable FM_VTable = { midi_fm_noteOn, midi_fm_noteOff,
    midi_fm_channelVolume, midi_fm_pitchBend, midi_fm_program,
    midi_fm_allNotesOff, midi_fm_pan };

typedef struct MidiChannel MidiChannel;

struct MidiChannel {
    u8 volume;
    u8 program;
    u8 pan;
    u16 pitchBend;
};

static MidiChannel midiChannels[MIDI_CHANNELS];

static u8 polyphonicPitches[MAX_FM_CHANS];
static ControlChange lastUnknownControlChange;
static Timing timing;
static bool polyphonic;
static bool overflow;
static bool dynamicMode;
static bool disableNonGeneralMidiCCs;

static void allNotesOff(u8 chan);
static void setPolyphonic(bool state);
static void cc(u8 chan, u8 controller, u8 value);
static void generalMidiReset(void);
static void sendPong(void);
static void setDynamicMode(bool enabled);

static void initDeviceChannel(u8 devChan)
{
    ChannelState* state = &channelState[devChan];
    bool isFm = devChan < DEV_CHAN_MIN_PSG;
    state->deviceChannel = isFm ? devChan : devChan - DEV_CHAN_MIN_PSG;
    state->ops = isFm ? &FM_VTable : &PSG_VTable;
    state->noteOn = false;
    state->program = 0;
    state->midiChannel = devChan;
    state->pitch = 0;
    state->pan = DEFAULT_MIDI_PAN;
    state->volume = MAX_MIDI_VOLUME;
    state->pitchBend = DEFAULT_MIDI_PITCH_BEND;
}

static void initChannelState(void)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        initDeviceChannel(i);
    }
}

static void initMidiChannel(u8 midiChan)
{
    MidiChannel* chan = &midiChannels[midiChan];
    chan->program = 0;
    chan->pan = DEFAULT_MIDI_PAN;
    chan->volume = MAX_MIDI_VOLUME;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
}

static void resetAllState(void)
{
    memset(&timing, 0, sizeof(Timing));
    memset(&lastUnknownControlChange, 0, sizeof(ControlChange));
    memset(&polyphonicPitches, 0, sizeof(polyphonicPitches));
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        initMidiChannel(i);
    }
    initChannelState();
    setDynamicMode(dynamicMode);
}

void midi_init(const Channel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets)
{
    overflow = false;
    polyphonic = false;
    dynamicMode = false;
    disableNonGeneralMidiCCs = false;
    resetAllState();
    midi_psg_init();
    midi_fm_init(defaultPresets, defaultPercussionPresets);
}

static ChannelState* findChannelPlayingNote(u8 midiChannel, u8 pitch)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        ChannelState* chan = &channelState[i];
        if (chan->noteOn && chan->midiChannel == midiChannel
            && chan->pitch == pitch) {
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
    if (state->volume != midiChannel->volume) {
        state->ops->channelVolume(state->deviceChannel, midiChannel->volume);
        state->volume = midiChannel->volume;
    }
}

static void updateChannelPan(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->pan != midiChannel->pan) {
        state->ops->pan(state->deviceChannel, midiChannel->pan);
        state->pan = midiChannel->pan;
    }
}

static void updatePitchBend(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->pitchBend != midiChannel->pitchBend) {
        state->ops->pitchBend(state->deviceChannel, midiChannel->pitchBend);
        state->pitchBend = midiChannel->pitchBend;
    }
}

static void updateProgram(MidiChannel* midiChannel, ChannelState* state)
{
    if (state->program != midiChannel->program) {
        state->ops->program(state->deviceChannel, midiChannel->program);
        state->program = midiChannel->program;
    }
}

static ChannelState* deviceChannelByMidiChannel(u8 midiChannel)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == midiChannel) {
            return state;
        }
    }
    return NULL;
}

static void dynamicNoteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (tooManyPercussiveNotes(chan)) {
        return;
    }
    ChannelState* state = dynamicMode ? findFreeChannel(chan)
                                      : deviceChannelByMidiChannel(chan);
    if (state == NULL) {
        overflow = true;
        return;
    }
    overflow = false;
    state->midiChannel = chan;
    MidiChannel* midiChannel = &midiChannels[chan];
    midi_fm_percussive(
        state->deviceChannel, chan == GENERAL_MIDI_PERCUSSION_CHANNEL);
    updateChannelVolume(midiChannel, state);
    updateChannelPan(midiChannel, state);
    updateProgram(midiChannel, state);
    updatePitchBend(midiChannel, state);
    state->pitch = pitch;
    state->noteOn = true;
    state->ops->noteOn(state->deviceChannel, pitch, velocity);
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (velocity == 0) {
        midi_noteOff(chan, pitch);
        return;
    }
    dynamicNoteOn(chan, pitch, velocity);
}

static void dynamicNoteOff(u8 chan, u8 pitch)
{
    ChannelState* state;
    while ((state = findChannelPlayingNote(chan, pitch)) != NULL) {
        state->noteOn = false;
        state->pitch = 0;
        state->ops->noteOff(state->deviceChannel, pitch);
    }
}

void midi_noteOff(u8 chan, u8 pitch)
{
    dynamicNoteOff(chan, pitch);
}

void midi_cc(u8 chan, u8 controller, u8 value)
{
    cc(chan, controller, value);
}

bool midi_overflow(void)
{
    return overflow;
}

static void channelPan(u8 chan, u8 pan)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pan = pan;
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan) {
            updateChannelPan(midiChannel, state);
        }
    }
}

static void channelVolume(u8 chan, u8 volume)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->volume = volume;
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan) {
            updateChannelVolume(midiChannel, state);
        }
    }
}

void resetAllControllers(u8 chan)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan) {
            initDeviceChannel(i);
        }
    }
    initMidiChannel(chan);
    setDynamicMode(dynamicMode);
}

static bool isIgnoringNonGeneralMidiCCs(void)
{
    return disableNonGeneralMidiCCs;
}

static void cc(u8 chan, u8 controller, u8 value)
{

    switch (controller) {
    case CC_VOLUME:
        channelVolume(chan, value);
        break;
    case CC_PAN:
        channelPan(chan, value);
        break;
    case CC_ALL_NOTES_OFF:
    case CC_ALL_SOUND_OFF:
        allNotesOff(chan);
        break;
    case CC_POLYPHONIC_MODE:
        setPolyphonic(RANGE(value, 2) != 0);
        break;
    case CC_RESET_ALL_CONTROLLERS:
        resetAllControllers(chan);
        break;
    default: {
        for (u8 i = 0; i < DEV_CHANS; i++) {
            ChannelState* state = &channelState[i];
            if (state->midiChannel == chan) {
                u8 devChan = state->deviceChannel;
                switch (controller) {
                case CC_GENMDM_FM_ALGORITHM:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_algorithm(devChan, RANGE(value, 8));
                    break;
                case CC_GENMDM_FM_FEEDBACK:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_feedback(devChan, RANGE(value, 8));
                    break;
                case CC_GENMDM_TOTAL_LEVEL_OP1:
                case CC_GENMDM_TOTAL_LEVEL_OP2:
                case CC_GENMDM_TOTAL_LEVEL_OP3:
                case CC_GENMDM_TOTAL_LEVEL_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorTotalLevel(
                        devChan, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
                    break;
                case CC_GENMDM_MULTIPLE_OP1:
                case CC_GENMDM_MULTIPLE_OP2:
                case CC_GENMDM_MULTIPLE_OP3:
                case CC_GENMDM_MULTIPLE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorMultiple(devChan,
                        controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
                    break;
                case CC_GENMDM_DETUNE_OP1:
                case CC_GENMDM_DETUNE_OP2:
                case CC_GENMDM_DETUNE_OP3:
                case CC_GENMDM_DETUNE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorDetune(devChan,
                        controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
                    break;
                case CC_GENMDM_RATE_SCALING_OP1:
                case CC_GENMDM_RATE_SCALING_OP2:
                case CC_GENMDM_RATE_SCALING_OP3:
                case CC_GENMDM_RATE_SCALING_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorRateScaling(devChan,
                        controller - CC_GENMDM_RATE_SCALING_OP1,
                        RANGE(value, 4));
                    break;
                case CC_GENMDM_ATTACK_RATE_OP1:
                case CC_GENMDM_ATTACK_RATE_OP2:
                case CC_GENMDM_ATTACK_RATE_OP3:
                case CC_GENMDM_ATTACK_RATE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorAttackRate(devChan,
                        controller - CC_GENMDM_ATTACK_RATE_OP1,
                        RANGE(value, 32));
                    break;
                case CC_GENMDM_FIRST_DECAY_RATE_OP1:
                case CC_GENMDM_FIRST_DECAY_RATE_OP2:
                case CC_GENMDM_FIRST_DECAY_RATE_OP3:
                case CC_GENMDM_FIRST_DECAY_RATE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorFirstDecayRate(devChan,
                        controller - CC_GENMDM_FIRST_DECAY_RATE_OP1,
                        RANGE(value, 32));
                    break;
                case CC_GENMDM_SECOND_DECAY_RATE_OP1:
                case CC_GENMDM_SECOND_DECAY_RATE_OP2:
                case CC_GENMDM_SECOND_DECAY_RATE_OP3:
                case CC_GENMDM_SECOND_DECAY_RATE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorSecondDecayRate(devChan,
                        controller - CC_GENMDM_SECOND_DECAY_RATE_OP1,
                        RANGE(value, 16));
                    break;
                case CC_GENMDM_SECOND_AMPLITUDE_OP1:
                case CC_GENMDM_SECOND_AMPLITUDE_OP2:
                case CC_GENMDM_SECOND_AMPLITUDE_OP3:
                case CC_GENMDM_SECOND_AMPLITUDE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorSecondaryAmplitude(devChan,
                        controller - CC_GENMDM_SECOND_AMPLITUDE_OP1,
                        RANGE(value, 16));
                    break;
                case CC_GENMDM_RELEASE_RATE_OP1:
                case CC_GENMDM_RELEASE_RATE_OP2:
                case CC_GENMDM_RELEASE_RATE_OP3:
                case CC_GENMDM_RELEASE_RATE_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorReleaseRate(devChan,
                        controller - CC_GENMDM_RELEASE_RATE_OP1,
                        RANGE(value, 16));
                    break;
                case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
                case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
                case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
                case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorAmplitudeModulation(devChan,
                        controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1,
                        RANGE(value, 2));
                    break;
                case CC_GENMDM_SSG_EG_OP1:
                case CC_GENMDM_SSG_EG_OP2:
                case CC_GENMDM_SSG_EG_OP3:
                case CC_GENMDM_SSG_EG_OP4:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_operatorSsgEg(devChan,
                        controller - CC_GENMDM_SSG_EG_OP1, RANGE(value, 16));
                    break;
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                case CC_GENMDM_GLOBAL_LFO_ENABLE:
                    synth_enableLfo(RANGE(value, 2));
                    break;
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                case CC_GENMDM_GLOBAL_LFO_FREQUENCY:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_globalLfoFrequency(RANGE(value, 8));
                    break;
                case CC_GENMDM_AMS:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_ams(devChan, RANGE(value, 4));
                    break;
                case CC_GENMDM_FMS:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_fms(devChan, RANGE(value, 8));
                    break;
                case CC_GENMDM_STEREO:
                    if (isIgnoringNonGeneralMidiCCs())
                        break;
                    synth_stereo(devChan, RANGE(value, 4));
                    break;
                default:
                    lastUnknownControlChange.controller = controller;
                    lastUnknownControlChange.value = value;
                    break;
                }
            }
        }
    }
    }
}

void midi_pitchBend(u8 chan, u16 bend)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pitchBend = bend;
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan && state->noteOn) {
            updatePitchBend(midiChannel, state);
        }
    }
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
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan) {
            updateProgram(midiChannel, state);
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
    for (u8 i = 0; i < DEV_CHANS; i++) {
        ChannelState* state = &channelState[i];
        if (state->midiChannel == chan) {
            state->noteOn = false;
            state->pitch = 0;
            state->ops->allNotesOff(state->deviceChannel);
        }
    }
}

static void setPolyphonic(bool state)
{
    polyphonic = state;
    setDynamicMode(state);
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

static void setNonGeneralMidiCCs(bool enable)
{
    disableNonGeneralMidiCCs = !enable;
}

void midi_sysex(const u8* data, u16 length)
{
    const u8 SYSEX_REMAP_COMMAND_ID = 0x00;
    const u8 SYSEX_PING_COMMAND_ID = 0x01;
    const u8 SYSEX_DYNAMIC_COMMAND_ID = 0x03;
    const u8 SYSEX_NON_GENERAL_MIDI_CCS_COMMAND_ID = 0x04;

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

    const u8 NON_GENERAL_MIDI_CCS_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_NON_GENERAL_MIDI_CCS_COMMAND_ID };

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
    } else if (sysex_valid(data, length, NON_GENERAL_MIDI_CCS_SEQUENCE,
                   LENGTH_OF(NON_GENERAL_MIDI_CCS_SEQUENCE), 1)) {
        setNonGeneralMidiCCs((bool)data[4]);
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

static void dynamicRemapChannel(u8 midiChannel, u8 deviceChannel)
{
    const u8 SYSEX_UNASSIGNED_DEVICE_CHANNEL = 0x7F;
    const u8 SYSEX_UNASSIGNED_MIDI_CHANNEL = 0x7F;

    ChannelState* assignedChan = deviceChannelByMidiChannel(midiChannel);
    if (assignedChan != NULL) {
        assignedChan->midiChannel = DEFAULT_MIDI_CHANNEL;
    }
    if (deviceChannel == SYSEX_UNASSIGNED_DEVICE_CHANNEL) {
        return;
    }
    ChannelState* chan = &channelState[deviceChannel];
    chan->midiChannel = (midiChannel == SYSEX_UNASSIGNED_MIDI_CHANNEL)
        ? DEFAULT_MIDI_CHANNEL
        : midiChannel;
}

void midi_remapChannel(u8 midiChannel, u8 deviceChannel)
{
    dynamicRemapChannel(midiChannel, deviceChannel);
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

    if (enabled) {
        for (u8 i = 0; i < DEV_CHANS; i++) {
            ChannelState* chan = &channelState[i];
            chan->midiChannel = DEFAULT_MIDI_CHANNEL;
        }
    } else {
        for (u8 i = 0; i < DEV_CHANS; i++) {
            ChannelState* chan = &channelState[i];
            chan->midiChannel = i;
        }
    }
}
