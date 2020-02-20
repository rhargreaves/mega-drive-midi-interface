#include "midi.h"
#include "comm.h"
#include "log.h"
#include "memcmp.h"
#include "memory.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "midi_sender.h"
#include "psg_chip.h"
#include "synth.h"
#include "ui_fm.h"
#include <stdbool.h>

#define MIN_MIDI_VELOCITY 0
#define RANGE(value, range) (value / (128 / range))
#define CHANNEL_UNASSIGNED 0xFF
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

static const u8 SYSEX_EXTENDED_MANU_ID_SECTION = 0x00;
static const u8 SYSEX_UNUSED_EUROPEAN_SECTION = 0x22;
static const u8 SYSEX_UNUSED_MANU_ID = 0x77;

static DeviceChannel deviceChannels[DEV_CHANS];

static const VTable PSG_VTable = { midi_psg_noteOn, midi_psg_noteOff,
    midi_psg_channelVolume, midi_psg_pitchBend, midi_psg_program,
    midi_psg_allNotesOff, midi_psg_pan };

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
static ControlChange lastUnknownControlChange;
static Timing timing;
static bool dynamicMode;
static bool disableNonGeneralMidiCCs;
static bool stickToDeviceType;

static void allNotesOff(u8 chan);
static void generalMidiReset(void);
static void sendPong(void);
static void setDynamicMode(bool enabled);
static void updateDeviceChannelFromAssociatedMidiChannel(
    DeviceChannel* devChan);
static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel);

static void initMidiChannel(u8 midiChan)
{
    MidiChannel* chan = &midiChannels[midiChan];
    chan->program = 0;
    chan->pan = DEFAULT_MIDI_PAN;
    chan->volume = MAX_MIDI_VOLUME;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
}

static void initDeviceChannel(u8 devChan)
{
    DeviceChannel* chan = &deviceChannels[devChan];
    bool isFm = devChan < DEV_CHAN_MIN_PSG;
    chan->number = isFm ? devChan : devChan - DEV_CHAN_MIN_PSG;
    chan->ops = isFm ? &FM_VTable : &PSG_VTable;
    chan->noteOn = false;
    chan->midiChannel = devChan;
    chan->pitch = 0;
    updateDeviceChannelFromAssociatedMidiChannel(chan);
}

static void initAllDeviceChannels(void)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        initDeviceChannel(i);
    }
}

static void resetAllState(void)
{
    memset(&timing, 0, sizeof(Timing));
    memset(&lastUnknownControlChange, 0, sizeof(ControlChange));
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        initMidiChannel(i);
    }
    initAllDeviceChannels();
    setDynamicMode(dynamicMode);
}

void midi_init(const FmChannel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets)
{
    dynamicMode = false;
    disableNonGeneralMidiCCs = false;
    stickToDeviceType = false;
    resetAllState();
    midi_psg_init();
    midi_fm_init(defaultPresets, defaultPercussionPresets);
}

static DeviceChannel* findChannelPlayingNote(u8 midiChannel, u8 pitch)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (chan->noteOn && chan->midiChannel == midiChannel
            && chan->pitch == pitch) {
            return chan;
        }
    }
    return NULL;
}

static bool isPsgNoise(DeviceChannel* chan)
{
    return chan->ops == &PSG_VTable && chan->number == DEV_CHAN_PSG_NOISE;
}

static bool isPsgAndIncomingChanIsPercussive(
    DeviceChannel* chan, u8 incomingChan)
{
    return chan->ops == &PSG_VTable
        && incomingChan == GENERAL_MIDI_PERCUSSION_CHANNEL;
}

static bool isChannelSuitable(DeviceChannel* chan, u8 incomingMidiChan)
{
    return !chan->noteOn && !isPsgNoise(chan)
        && !isPsgAndIncomingChanIsPercussive(chan, incomingMidiChan);
}

static DeviceChannel* findFreeMidiAssignedChannel(u8 incomingMidiChan)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (chan->midiChannel == incomingMidiChan) {
            if (isChannelSuitable(chan, incomingMidiChan)) {
                return chan;
            }
        }
    }
    return NULL;
}

static DeviceChannel* findFreePsgChannelForSquareWaveVoices(u8 incomingMidiChan)
{
    const u8 SQUARE_WAVE_MIDI_PROGRAMS[3] = { 80, 89, 99 };

    MidiChannel* midiChan = &midiChannels[incomingMidiChan];
    for (u16 p = 0; p < LENGTH_OF(SQUARE_WAVE_MIDI_PROGRAMS); p++) {
        u8 program = SQUARE_WAVE_MIDI_PROGRAMS[p];
        if (midiChan->program == program) {
            for (u16 i = DEV_CHAN_MIN_PSG; i < DEV_CHANS; i++) {
                DeviceChannel* chan = &deviceChannels[i];
                if (isChannelSuitable(chan, incomingMidiChan)) {
                    return chan;
                }
            }
        }
    }
    return NULL;
}

static DeviceChannel* findAnyFreeChannel(u8 incomingMidiChan)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (isChannelSuitable(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static DeviceChannel* findDeviceSpecificChannel(u8 incomingMidiChan)
{
    u8 minChan = 0;
    u8 maxChan = DEV_CHAN_MAX_TONE_PSG;
    if (stickToDeviceType) {
        DeviceChannel* assignedChan
            = deviceChannelByMidiChannel(incomingMidiChan);
        if (assignedChan == NULL) {
            return NULL;
        }
        bool isFm = (assignedChan->ops == &FM_VTable);
        minChan = isFm ? DEV_CHAN_MIN_FM : DEV_CHAN_MIN_PSG;
        maxChan = isFm ? DEV_CHAN_MAX_FM : DEV_CHAN_MAX_TONE_PSG;
    }

    for (u16 i = minChan; i <= maxChan; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (isChannelSuitable(chan, incomingMidiChan)) {
            return chan;
        }
    }
    for (u16 i = minChan; i <= maxChan; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (chan->midiChannel == incomingMidiChan && !isPsgNoise(chan)
            && !isPsgAndIncomingChanIsPercussive(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static DeviceChannel* findFreeChannel(u8 incomingMidiChan)
{
    DeviceChannel* chan = findFreeMidiAssignedChannel(incomingMidiChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findFreePsgChannelForSquareWaveVoices(incomingMidiChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findDeviceSpecificChannel(incomingMidiChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findAnyFreeChannel(incomingMidiChan);
    if (chan != NULL) {
        return chan;
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
        DeviceChannel* chan = &deviceChannels[i];
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

static void updateVolume(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->volume != midiChannel->volume) {
        devChan->ops->channelVolume(devChan->number, midiChannel->volume);
        devChan->volume = midiChannel->volume;
    }
}

static void updatePan(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->pan != midiChannel->pan) {
        devChan->ops->pan(devChan->number, midiChannel->pan);
        devChan->pan = midiChannel->pan;
    }
}

static void updatePitchBend(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->pitchBend != midiChannel->pitchBend) {
        devChan->ops->pitchBend(devChan->number, midiChannel->pitchBend);
        devChan->pitchBend = midiChannel->pitchBend;
    }
}

static void updateProgram(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->program != midiChannel->program) {
        devChan->ops->program(devChan->number, midiChannel->program);
        devChan->program = midiChannel->program;
    }
}

static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (chan->midiChannel == midiChannel) {
            return chan;
        }
    }
    return NULL;
}

static void updateDeviceChannelFromAssociatedMidiChannel(DeviceChannel* devChan)
{
    MidiChannel* midiChannel = &midiChannels[devChan->midiChannel];
    midi_fm_percussive(devChan->number,
        devChan->midiChannel == GENERAL_MIDI_PERCUSSION_CHANNEL);
    updateVolume(midiChannel, devChan);
    updatePan(midiChannel, devChan);
    updateProgram(midiChannel, devChan);
    updatePitchBend(midiChannel, devChan);
}

static DeviceChannel* findSuitableDeviceChannel(u8 midiChan)
{
    return dynamicMode ? findFreeChannel(midiChan)
                       : deviceChannelByMidiChannel(midiChan);
}

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (velocity == MIN_MIDI_VELOCITY) {
        midi_noteOff(chan, pitch);
        return;
    }
    if (tooManyPercussiveNotes(chan)) {
        return;
    }
    DeviceChannel* devChan = findSuitableDeviceChannel(chan);
    if (devChan == NULL) {
        log_warn("Ch %d: Dropped note %d", chan + 1, pitch, 0);
        return;
    }
    devChan->midiChannel = chan;
    updateDeviceChannelFromAssociatedMidiChannel(devChan);
    devChan->pitch = pitch;
    devChan->noteOn = true;
    devChan->ops->noteOn(devChan->number, pitch, velocity);
}

void midi_noteOff(u8 chan, u8 pitch)
{
    DeviceChannel* devChan;
    while ((devChan = findChannelPlayingNote(chan, pitch)) != NULL) {
        devChan->noteOn = false;
        devChan->pitch = 0;
        devChan->ops->noteOff(devChan->number, pitch);
    }
}

static void channelPan(u8 chan, u8 pan)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pan = pan;
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* devChan = &deviceChannels[i];
        if (devChan->midiChannel == chan) {
            updatePan(midiChannel, devChan);
        }
    }
}

static void channelVolume(u8 chan, u8 volume)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->volume = volume;
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* state = &deviceChannels[i];
        if (state->midiChannel == chan) {
            updateVolume(midiChannel, state);
        }
    }
}

void resetAllControllers(u8 chan)
{
    initMidiChannel(chan);
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* state = &deviceChannels[i];
        if (state->midiChannel == chan) {
            initDeviceChannel(i);
        }
    }
    setDynamicMode(dynamicMode);
}

static bool isIgnoringNonGeneralMidiCCs(void)
{
    return disableNonGeneralMidiCCs;
}

static void setPolyphonicMode(bool enable)
{
    setDynamicMode(enable);
    if (enable) {
        for (u8 chan = 0; chan <= DEV_CHAN_MAX_FM; chan++) {
            midi_remapChannel(0, chan);
        }
    }
}

void midi_pitchBend(u8 chan, u16 bend)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pitchBend = bend;
    for (DeviceChannel* state = &deviceChannels[0];
         state < &deviceChannels[DEV_CHANS]; state++) {
        if (state->midiChannel == chan && state->noteOn) {
            updatePitchBend(midiChannel, state);
        }
    }
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
        DeviceChannel* state = &deviceChannels[i];
        if (state->midiChannel == chan) {
            updateProgram(midiChannel, state);
        }
    }
}

bool midi_dynamicMode(void)
{
    return dynamicMode;
}

DeviceChannel* midi_channelMappings(void)
{
    return deviceChannels;
}

ControlChange* midi_lastUnknownCC(void)
{
    return &lastUnknownControlChange;
}

static void allNotesOff(u8 chan)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* devChan = &deviceChannels[i];
        if (devChan->midiChannel == chan) {
            devChan->noteOn = false;
            devChan->pitch = 0;
            devChan->ops->allNotesOff(devChan->number);
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

static void setNonGeneralMidiCCs(bool enable)
{
    disableNonGeneralMidiCCs = !enable;
}

static void setStickToDeviceType(bool enable)
{
    stickToDeviceType = enable;
}

void midi_sysex(const u8* data, u16 length)
{
    const u8 SYSEX_REMAP_COMMAND_ID = 0x00;
    const u8 SYSEX_PING_COMMAND_ID = 0x01;
    const u8 SYSEX_DYNAMIC_COMMAND_ID = 0x03;
    const u8 SYSEX_NON_GENERAL_MIDI_CCS_COMMAND_ID = 0x04;
    const u8 SYSEX_STICK_TO_DEVICE_TYPE_COMMAND_ID = 0x05;

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

    const u8 STICK_TO_DEVICE_TYPE_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_STICK_TO_DEVICE_TYPE_COMMAND_ID };

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
    } else if (sysex_valid(data, length, STICK_TO_DEVICE_TYPE_SEQUENCE,
                   LENGTH_OF(STICK_TO_DEVICE_TYPE_SEQUENCE), 1)) {
        setStickToDeviceType((bool)data[4]);
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

void midi_remapChannel(u8 midiChan, u8 devChan)
{
    const u8 SYSEX_UNASSIGNED_DEVICE_CHANNEL = 0x7F;
    const u8 SYSEX_UNASSIGNED_MIDI_CHANNEL = 0x7F;

    if (devChan == SYSEX_UNASSIGNED_DEVICE_CHANNEL) {
        DeviceChannel* assignedChan = deviceChannelByMidiChannel(midiChan);
        if (assignedChan != NULL) {
            assignedChan->midiChannel = DEFAULT_MIDI_CHANNEL;
        }
        return;
    }
    DeviceChannel* chan = &deviceChannels[devChan];
    chan->midiChannel = (midiChan == SYSEX_UNASSIGNED_MIDI_CHANNEL)
        ? DEFAULT_MIDI_CHANNEL
        : midiChan;
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
    for (u8 chan = 0; chan < DEV_CHANS; chan++) {
        DeviceChannel* devChan = &deviceChannels[chan];
        devChan->midiChannel = enabled ? DEFAULT_MIDI_CHANNEL : chan;
    }
}

static void setFmChanParameter(u8 devChan, u8 controller, u8 value)
{
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
        synth_operatorMultiple(
            devChan, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorDetune(
            devChan, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorRateScaling(
            devChan, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAttackRate(
            devChan, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_FIRST_DECAY_RATE_OP1:
    case CC_GENMDM_FIRST_DECAY_RATE_OP2:
    case CC_GENMDM_FIRST_DECAY_RATE_OP3:
    case CC_GENMDM_FIRST_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorFirstDecayRate(devChan,
            controller - CC_GENMDM_FIRST_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SECOND_DECAY_RATE_OP1:
    case CC_GENMDM_SECOND_DECAY_RATE_OP2:
    case CC_GENMDM_SECOND_DECAY_RATE_OP3:
    case CC_GENMDM_SECOND_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSecondDecayRate(devChan,
            controller - CC_GENMDM_SECOND_DECAY_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SECOND_AMPLITUDE_OP1:
    case CC_GENMDM_SECOND_AMPLITUDE_OP2:
    case CC_GENMDM_SECOND_AMPLITUDE_OP3:
    case CC_GENMDM_SECOND_AMPLITUDE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSecondaryAmplitude(devChan,
            controller - CC_GENMDM_SECOND_AMPLITUDE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorReleaseRate(
            devChan, controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAmplitudeModulation(devChan,
            controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSsgEg(
            devChan, controller - CC_GENMDM_SSG_EG_OP1, RANGE(value, 16));
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

static void fmParameterCC(u8 chan, u8 controller, u8 value)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* devChan = &deviceChannels[i];
        if (devChan->midiChannel == chan) {
            setFmChanParameter(devChan->number, controller, value);
        }
    }
}

void midi_cc(u8 chan, u8 controller, u8 value)
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
        setPolyphonicMode(RANGE(value, 2) != 0);
        break;
    case CC_RESET_ALL_CONTROLLERS:
        resetAllControllers(chan);
        break;
    case CC_SHOW_PARAMETERS_ON_UI:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        ui_fm_setMidiChannelParametersVisibility(chan, RANGE(value, 2));
        break;
    default:
        fmParameterCC(chan, controller, value);
        break;
    }
}
