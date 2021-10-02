#include "midi.h"
#include "comm.h"
#include "log.h"
#include "memcmp.h"
#include "memory.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "midi_sender.h"
#include "synth.h"
#include "ui_fm.h"
#include <stdbool.h>

#define MIN_MIDI_VELOCITY 0
#define RANGE(value, range) (value / (128 / range))
#define CHANNEL_UNASSIGNED 0xFF
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

static DeviceChannel deviceChannels[DEV_CHANS];

static const VTable PSG_VTable = { midi_psg_noteOn, midi_psg_noteOff,
    midi_psg_channelVolume, midi_psg_pitchBend, midi_psg_program,
    midi_psg_allNotesOff, midi_psg_pan };

static const VTable FM_VTable = { midi_fm_noteOn, midi_fm_noteOff,
    midi_fm_channelVolume, midi_fm_pitchBend, midi_fm_program,
    midi_fm_allNotesOff, midi_fm_pan };

typedef enum DeviceSelect DeviceSelect;

enum DeviceSelect { Auto, FM, PSG_Tone, PSG_Noise };

typedef struct MidiChannel MidiChannel;

struct MidiChannel {
    u8 volume;
    u8 program;
    u8 pan;
    u16 pitchBend;
    DeviceSelect deviceSelect;
};

static MidiChannel midiChannels[MIDI_CHANNELS];
static bool dynamicMode;
static bool disableNonGeneralMidiCCs;
static bool stickToDeviceType;
static bool invertTotalLevel;

static void allNotesOff(u8 chan);
static void generalMidiReset(void);
static void sendPong(void);
static void setInvertTotalLevel(bool enabled);
static void setDynamicMode(bool enabled);
static void setOperatorTotalLevel(u8 chan, u8 op, u8 value);
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
    chan->deviceSelect = Auto;
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
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
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
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        initMidiChannel(i);
    }
    initAllDeviceChannels();
    setDynamicMode(dynamicMode);
}

static const u8** defaultEnvelopes;
static const FmChannel** defaultPresets;
static const PercussionPreset** defaultPercussionPresets;

static void init(void)
{
    midi_psg_init(defaultEnvelopes);
    midi_fm_init(defaultPresets, defaultPercussionPresets);
    dynamicMode = false;
    disableNonGeneralMidiCCs = false;
    stickToDeviceType = false;
    resetAllState();
}

void midi_init(const FmChannel** presets,
    const PercussionPreset** percussionPresets, const u8** envelopes)
{
    defaultEnvelopes = envelopes;
    defaultPresets = presets;
    defaultPercussionPresets = percussionPresets;
    init();
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

static bool isPsgAndIncomingChanIsPercussive(
    DeviceChannel* chan, u8 incomingChan)
{
    return chan->ops == &PSG_VTable
        && incomingChan == GENERAL_MIDI_PERCUSSION_CHANNEL;
}

static bool isChannelSuitable(DeviceChannel* chan, u8 incomingMidiChan)
{
    return !chan->noteOn
        && !isPsgAndIncomingChanIsPercussive(chan, incomingMidiChan);
}

static DeviceChannel* findFreeMidiAssignedChannel(
    u8 incomingMidiChan, u8 devChanMin, u8 devChanMax)
{
    for (u16 i = devChanMin; i <= devChanMax; i++) {
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
            for (u16 i = DEV_CHAN_MIN_PSG; i < DEV_CHAN_MAX_TONE_PSG; i++) {
                DeviceChannel* chan = &deviceChannels[i];
                if (isChannelSuitable(chan, incomingMidiChan)) {
                    return chan;
                }
            }
        }
    }
    return NULL;
}

static DeviceChannel* findAnyFreeChannel(
    u8 incomingMidiChan, u8 minDevChan, u8 maxDevChan)
{
    for (u16 i = minDevChan; i < maxDevChan; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (isChannelSuitable(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static DeviceChannel* findDeviceSpecificChannel(
    u8 incomingMidiChan, u8 minDevChan, u8 maxDevChan)
{
    u8 minChan = minDevChan;
    u8 maxChan = maxDevChan;
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
        if (chan->midiChannel == incomingMidiChan
            && !isPsgAndIncomingChanIsPercussive(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static void setDeviceMinMaxChans(
    u8 incomingMidiChan, u8* minDevChan, u8* maxDevChan)
{
    MidiChannel* midiChannel = &midiChannels[incomingMidiChan];
    if (midiChannel->deviceSelect == Auto) {
        *minDevChan = DEV_CHAN_MIN_FM;
        *maxDevChan = DEV_CHAN_MAX_TONE_PSG;
    } else if (midiChannel->deviceSelect == FM) {
        *minDevChan = DEV_CHAN_MIN_FM;
        *maxDevChan = DEV_CHAN_MAX_FM;
    } else if (midiChannel->deviceSelect == PSG_Tone) {
        *minDevChan = DEV_CHAN_MIN_PSG;
        *maxDevChan = DEV_CHAN_MAX_TONE_PSG;
    } else {
        *minDevChan = DEV_CHAN_PSG_NOISE;
        *maxDevChan = DEV_CHAN_PSG_NOISE;
    }
}

static DeviceChannel* findFreeChannel(u8 incomingMidiChan)
{
    u8 minDevChan;
    u8 maxDevChan;
    setDeviceMinMaxChans(incomingMidiChan, &minDevChan, &maxDevChan);
    DeviceChannel* chan
        = findFreeMidiAssignedChannel(incomingMidiChan, minDevChan, maxDevChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findFreePsgChannelForSquareWaveVoices(incomingMidiChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findDeviceSpecificChannel(incomingMidiChan, minDevChan, maxDevChan);
    if (chan != NULL) {
        return chan;
    }
    chan = findAnyFreeChannel(incomingMidiChan, minDevChan, maxDevChan);
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

static void channelDeviceSelect(u8 midiChan, DeviceSelect deviceSelect)
{
    MidiChannel* midiChannel = &midiChannels[midiChan];
    midiChannel->deviceSelect = deviceSelect;
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
        log_warn("Ch %d: Dropped note %d", chan + 1, pitch);
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

static void loadPsgEnvelope(const u8* data, u16 length)
{
    u8 buffer[256];
    u16 eefStepIndex = 0;
    for (u16 i = 4; i < length; i += 2) {
        buffer[eefStepIndex++] = (data[i] << 4) | data[i + 1];
    }
    buffer[eefStepIndex] = EEF_END;
    midi_psg_loadEnvelope(buffer);
    log_info("Loaded User Defined Envelope");
}

void midi_sysex(const u8* data, u16 length)
{
    const u8 GENERAL_MIDI_RESET_SEQUENCE[] = { 0x7E, 0x7F, 0x09, 0x01 };

    const u8 REMAP_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_REMAP };

    const u8 PING_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_PING };

    const u8 DYNAMIC_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_DYNAMIC };

    const u8 NON_GENERAL_MIDI_CCS_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_NON_GENERAL_MIDI_CCS };

    const u8 STICK_TO_DEVICE_TYPE_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_STICK_TO_DEVICE_TYPE };

    const u8 LOAD_PSG_ENVELOPE_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_LOAD_PSG_ENVELOPE };

    const u8 INVERT_TOTAL_LEVEL_SEQUENCE[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_INVERT_TOTAL_LEVEL };

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
    } else if (sysex_valid(data, length, INVERT_TOTAL_LEVEL_SEQUENCE,
                   LENGTH_OF(INVERT_TOTAL_LEVEL_SEQUENCE), 1)) {
        setInvertTotalLevel((bool)data[4]);
    } else if (memcmp(data, LOAD_PSG_ENVELOPE_SEQUENCE,
                   LENGTH_OF(LOAD_PSG_ENVELOPE_SEQUENCE))
        == 0) {
        loadPsgEnvelope(data, length);
    }
}

static void sendPong(void)
{
    const u8 pongSequence[]
        = { SYSEX_EXTENDED_MANU_ID_SECTION, SYSEX_UNUSED_EUROPEAN_SECTION,
              SYSEX_UNUSED_MANU_ID, SYSEX_COMMAND_PONG };

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

static void setOperatorTotalLevel(u8 chan, u8 op, u8 value)
{
    const u8 MAX_TOTAL_LEVEL = 127;

    if (invertTotalLevel) {
        value = MAX_TOTAL_LEVEL - value;
    }
    synth_operatorTotalLevel(chan, op, value);
}

static void setFmChanParameter(DeviceChannel* devChan, u8 controller, u8 value)
{
    switch (controller) {
    case CC_GENMDM_FM_ALGORITHM:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_algorithm(devChan->number, RANGE(value, 8));
        break;
    case CC_GENMDM_FM_FEEDBACK:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_feedback(devChan->number, RANGE(value, 8));
        break;
    case CC_GENMDM_TOTAL_LEVEL_OP1:
    case CC_GENMDM_TOTAL_LEVEL_OP2:
    case CC_GENMDM_TOTAL_LEVEL_OP3:
    case CC_GENMDM_TOTAL_LEVEL_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        setOperatorTotalLevel(
            devChan->number, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorMultiple(devChan->number,
            controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorDetune(devChan->number, controller - CC_GENMDM_DETUNE_OP1,
            RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorRateScaling(devChan->number,
            controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAttackRate(devChan->number,
            controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_FIRST_DECAY_RATE_OP1:
    case CC_GENMDM_FIRST_DECAY_RATE_OP2:
    case CC_GENMDM_FIRST_DECAY_RATE_OP3:
    case CC_GENMDM_FIRST_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorFirstDecayRate(devChan->number,
            controller - CC_GENMDM_FIRST_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SECOND_DECAY_RATE_OP1:
    case CC_GENMDM_SECOND_DECAY_RATE_OP2:
    case CC_GENMDM_SECOND_DECAY_RATE_OP3:
    case CC_GENMDM_SECOND_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSecondDecayRate(devChan->number,
            controller - CC_GENMDM_SECOND_DECAY_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SECOND_AMPLITUDE_OP1:
    case CC_GENMDM_SECOND_AMPLITUDE_OP2:
    case CC_GENMDM_SECOND_AMPLITUDE_OP3:
    case CC_GENMDM_SECOND_AMPLITUDE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSecondaryAmplitude(devChan->number,
            controller - CC_GENMDM_SECOND_AMPLITUDE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorReleaseRate(devChan->number,
            controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAmplitudeModulation(devChan->number,
            controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSsgEg(devChan->number, controller - CC_GENMDM_SSG_EG_OP1,
            RANGE(value, 16));
        break;
    case CC_GENMDM_GLOBAL_LFO_ENABLE:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_enableLfo(RANGE(value, 2));
        break;
    case CC_GENMDM_GLOBAL_LFO_FREQUENCY:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_globalLfoFrequency(RANGE(value, 8));
        break;
    case CC_GENMDM_AMS:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_ams(devChan->number, RANGE(value, 4));
        break;
    case CC_GENMDM_FMS:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_fms(devChan->number, RANGE(value, 8));
        break;
    case CC_GENMDM_STEREO:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_stereo(devChan->number, RANGE(value, 4));
        break;
    default:
        log_warn(
            "Ch %d: CC %02X %02X?", devChan->midiChannel, controller, value);
        break;
    }
}

static void setInvertTotalLevel(bool invert)
{
    invertTotalLevel = invert;
}

static void fmParameterCC(u8 chan, u8 controller, u8 value)
{
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* devChan = &deviceChannels[i];
        if (devChan->midiChannel == chan) {
            setFmChanParameter(devChan, controller, value);
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
    case CC_DEVICE_SELECT:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        channelDeviceSelect(chan, RANGE(value, 4));
        break;
    default:
        fmParameterCC(chan, controller, value);
        break;
    }
}

void midi_reset(void)
{
    init();
}
