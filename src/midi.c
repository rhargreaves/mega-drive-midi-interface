#include "midi.h"
#include "comm.h"
#include "log.h"
#include "memcmp.h"
#include "memory.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "midi_fm_sm.h"
#include "midi_dac.h"
#include "midi_sender.h"
#include "synth.h"
#include "ui_fm.h"
#include <stdbool.h>

#define MIN_MIDI_VELOCITY 0
#define RANGE(value, range) (value / (128 / range))
#define CHANNEL_UNASSIGNED 0xFF
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

#define NOTE_PRIORITY_STACK_SIZE 20

typedef struct NotePriorityStack {
    u8 slot[NOTE_PRIORITY_STACK_SIZE];
    s16 top;
} NotePriorityStack;

typedef enum DeviceSelect DeviceSelect;
enum DeviceSelect { Auto, FM, PSG_Tone, PSG_Noise };

typedef struct MidiChannel MidiChannel;
struct MidiChannel {
    u8 volume;
    u8 program;
    u8 pan;
    u16 pitchBend;
    u8 prevVelocity;
    NotePriorityStack notePriority;
    DeviceSelect deviceSelect;
};

typedef enum MappingMode MappingMode;
enum MappingMode { MappingMode_Static, MappingMode_Dynamic, MappingMode_Auto };

static const VTable PSG_VTable = { midi_psg_note_on, midi_psg_note_off,
    midi_psg_channel_volume, midi_psg_pitch_bend, midi_psg_program,
    midi_psg_all_notes_off, midi_psg_pan };

static const VTable FM_VTable = { midi_fm_note_on, midi_fm_note_off,
    midi_fm_channel_volume, midi_fm_pitch_bend, midi_fm_program,
    midi_fm_all_notes_off, midi_fm_pan };

static const VTable SpecialMode_VTable = { midi_fm_sm_note_on,
    midi_fm_sm_note_off, midi_fm_sm_channel_volume, midi_fm_sm_pitch_bend,
    midi_fm_sm_program, midi_fm_sm_all_notes_off, midi_fm_sm_pan };

static const VTable DAC_VTable = { midi_dac_note_on, midi_dac_note_off,
    midi_dac_channel_volume, midi_dac_pitch_bend, midi_dac_program,
    midi_dac_all_notes_off, midi_dac_pan };

static const u8** defaultEnvelopes;
static const FmChannel** defaultPresets;
static const PercussionPreset** defaultPercussionPresets;

static MappingMode mappingModePref;
static DeviceChannel deviceChannels[DEV_CHANS];
static MidiChannel midiChannels[MIDI_CHANNELS];
static bool dynamicMode;
static bool disableNonGeneralMidiCCs;
static bool stickToDeviceType;
static bool invertTotalLevel;

static void allNotesOff(u8 chan);
static void generalMidiReset(void);
static void applyDynamicMode(void);
static void sendPong(void);
static void setInvertTotalLevel(bool enabled);
static void setDynamicMode(MappingMode mode);
static void setOperatorTotalLevel(u8 chan, u8 op, u8 value);
static void updateDeviceChannelFromAssociatedMidiChannel(
    DeviceChannel* devChan);
static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel);
static void initAllDeviceChannels(void);
static void resetAllState(void);
static void initMidiChannel(u8 midiChan);
static void initDeviceChannel(u8 devChan);
static void reset(void);
static void init(void);

void midi_init(const FmChannel** presets,
    const PercussionPreset** percussionPresets, const u8** envelopes)
{
    defaultEnvelopes = envelopes;
    defaultPresets = presets;
    defaultPercussionPresets = percussionPresets;
    init();
}

static void init(void)
{
    midi_psg_init(defaultEnvelopes);
    midi_fm_init(defaultPresets, defaultPercussionPresets);
    reset();
}

static void reset(void)
{
    midi_psg_reset();
    midi_fm_reset();
    midi_fm_sm_reset();
    mappingModePref = MappingMode_Auto;
    dynamicMode = false;
    disableNonGeneralMidiCCs = false;
    stickToDeviceType = false;
    resetAllState();
}

static void resetAllState(void)
{
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        initMidiChannel(i);
    }
    initAllDeviceChannels();
    applyDynamicMode();
}

static void initMidiChannel(u8 midiChan)
{
    MidiChannel* chan = &midiChannels[midiChan];
    chan->program = 0;
    chan->pan = DEFAULT_MIDI_PAN;
    chan->volume = MAX_MIDI_VOLUME;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
    chan->prevVelocity = 0;
    chan->notePriority.top = -1;
    memset(chan->notePriority.slot, 0, NOTE_PRIORITY_STACK_SIZE);
    chan->deviceSelect = Auto;
}

static void initAllDeviceChannels(void)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        initDeviceChannel(i);
    }
}

static void initDeviceChannel(u8 devChan)
{
    DeviceChannel* chan = &deviceChannels[devChan];

    if (devChan <= DEV_CHAN_MAX_FM) {
        chan->number = devChan;
        chan->ops = &FM_VTable;
    } else if (devChan <= DEV_CHAN_MAX_PSG) {
        chan->number = devChan - DEV_CHAN_MIN_PSG;
        chan->ops = &PSG_VTable;
    } else if (devChan <= DEV_CHAN_MAX_SPECIAL_MODE) {
        chan->number = devChan - DEV_CHAN_MIN_SPECIAL_MODE;
        chan->ops = &SpecialMode_VTable;
    } else {
        chan->number = DEV_CHAN_DAC;
        chan->ops = &DAC_VTable;
    }

    chan->noteOn = false;
    chan->midiChannel = devChan;
    chan->pitch = 0;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
    updateDeviceChannelFromAssociatedMidiChannel(chan);
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

// 0      1      2     3
// Oldest -> Most Recent
static void notePriorityPush(NotePriorityStack* notePriority, u8 pitch)
{
    if (notePriority->top == NOTE_PRIORITY_STACK_SIZE - 1) {
        // rewrite array
    }
    notePriority->slot[++notePriority->top] = pitch;
}

static u8 notePriorityPop(NotePriorityStack* notePriority)
{
    if (notePriority->top == -1) {
        return 0;
    }

    return notePriority->slot[notePriority->top--];
}

static void notePriorityRemove(NotePriorityStack* notePriority, u8 pitch)
{
    s16 writeCursor = -1;
    for (u16 i = 0; i <= notePriority->top; i++) {
        u8 item = notePriority->slot[i];
        if (item == pitch) {
            continue;
        } else {
            notePriority->slot[++writeCursor] = item;
        }
    }
    notePriority->top = writeCursor;
}

void midi_note_on(u8 chan, u8 pitch, u8 velocity)
{
    if (velocity == MIN_MIDI_VELOCITY) {
        midi_note_off(chan, pitch);
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

    if (!dynamicMode) {
        MidiChannel* midiChannel = &midiChannels[chan];
        notePriorityPush(&midiChannel->notePriority, pitch);
        midiChannel->prevVelocity = velocity;
    }

    devChan->midiChannel = chan;
    updateDeviceChannelFromAssociatedMidiChannel(devChan);
    devChan->pitch = pitch;
    devChan->noteOn = true;
    devChan->ops->noteOn(devChan->number, pitch, velocity);
}

void midi_note_off(u8 chan, u8 pitch)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    notePriorityRemove(&midiChannel->notePriority, pitch);

    DeviceChannel* devChan;
    while ((devChan = findChannelPlayingNote(chan, pitch)) != NULL) {
        u8 nextMostRecentPitch = notePriorityPop(&midiChannel->notePriority);
        if (!dynamicMode && nextMostRecentPitch != 0) {
            devChan->pitch = nextMostRecentPitch;
            devChan->noteOn = true;
            devChan->ops->noteOn(devChan->number, nextMostRecentPitch,
                midiChannel->prevVelocity);
        } else {
            devChan->noteOn = false;
            devChan->pitch = 0;
            devChan->ops->noteOff(devChan->number, pitch);
        }
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
            midi_remap_channel(0, chan);
        }
    }
}

void midi_pitch_bend(u8 chan, u16 bend)
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

bool midi_dynamic_mode(void)
{
    return dynamicMode;
}

DeviceChannel* midi_channel_mappings(void)
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
    for (u16 i = 0; i < length; i += 2) {
        buffer[eefStepIndex++] = (data[i] << 4) | data[i + 1];
    }
    buffer[eefStepIndex] = EEF_END;
    midi_psg_load_envelope(buffer);
    log_info("Loaded User Defined Envelope");
}

static void incrementSysExCursor(const u8** data, u16* length, u8 value)
{
    (*data) += value;
    (*length) -= value;
}

static void directWriteYm2612(u8 part, const u8* data, u16 length)
{
    if (length != 4) {
        return;
    }
    synth_directWriteYm2612(
        part, data[0] << 4 | data[1], data[2] << 4 | data[3]);
}

static void handleCustomSysEx(const u8* data, u16 length)
{
    u8 command = *data;
    incrementSysExCursor(&data, &length, 1);
    switch (command) {
    case SYSEX_COMMAND_REMAP:
        if (length == 2) {
            midi_remap_channel(data[0], data[1]);
        }
        break;
    case SYSEX_COMMAND_PING:
        if (length == 0) {
            sendPong();
        }
        break;
    case SYSEX_COMMAND_DYNAMIC:
        if (length == 1) {
            setDynamicMode((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_NON_GENERAL_MIDI_CCS:
        if (length == 1) {
            setNonGeneralMidiCCs((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_STICK_TO_DEVICE_TYPE:
        if (length == 1) {
            setStickToDeviceType((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_INVERT_TOTAL_LEVEL:
        if (length == 1) {
            setInvertTotalLevel((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_LOAD_PSG_ENVELOPE:
        loadPsgEnvelope(data, length);
        break;
    case SYSEX_COMMAND_WRITE_YM2612_REG_PART_0:
        directWriteYm2612(0, data, length);
        break;
    case SYSEX_COMMAND_WRITE_YM2612_REG_PART_1:
        directWriteYm2612(1, data, length);
        break;
    }
}

void midi_sysex(const u8* data, u16 length)
{
    const u8 GENERAL_MIDI_RESET_SEQ[] = { 0x7E, 0x7F, 0x09, 0x01 };
    if (length < LENGTH_OF(GENERAL_MIDI_RESET_SEQ)) {
        return;
    }
    if (memcmp(data, GENERAL_MIDI_RESET_SEQ, LENGTH_OF(GENERAL_MIDI_RESET_SEQ))
        == 0) {
        generalMidiReset();
        return;
    }

    const u8 CUSTOM_SYSEX_SEQ[]
        = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID };
    if (memcmp(data, CUSTOM_SYSEX_SEQ, LENGTH_OF(CUSTOM_SYSEX_SEQ)) == 0) {
        incrementSysExCursor(&data, &length, LENGTH_OF(CUSTOM_SYSEX_SEQ));
        handleCustomSysEx(data, length);
    }
}

static void sendPong(void)
{
    const u8 pongSequence[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION,
        SYSEX_MANU_ID, SYSEX_COMMAND_PONG };

    midi_sender_send_sysex(pongSequence, sizeof(pongSequence));
}

void midi_remap_channel(u8 midiChan, u8 devChan)
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
    if (mappingModePref == MappingMode_Auto) {
        dynamicMode = true;
    }
    resetAllState();
}

static void applyDynamicMode(void)
{
    for (u8 chan = 0; chan < DEV_CHANS; chan++) {
        DeviceChannel* devChan = &deviceChannels[chan];
        devChan->midiChannel = dynamicMode ? DEFAULT_MIDI_CHANNEL : chan;
    }
}

static void setDynamicMode(MappingMode mode)
{
    mappingModePref = mode;
    if (mode == MappingMode_Dynamic) {
        dynamicMode = true;
        applyDynamicMode();
    } else if (mode == MappingMode_Static) {
        dynamicMode = false;
        applyDynamicMode();
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
    case CC_GENMDM_ENABLE_DAC:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_enableDac(RANGE(value, 2));
        break;
    case CC_EXPRESSION:
    case CC_SUSTAIN_PEDAL:
    case CC_DATA_ENTRY_LSB:
    case CC_DATA_ENTRY_MSB:
    case CC_NRPN_LSB:
    case CC_NRPN_MSB:
    case CC_RPN_LSB:
    case CC_RPN_MSB:
        break;
    default:
        log_warn("Ch %d: CC 0x%02X 0x%02X?", devChan->midiChannel, controller,
            value);
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
    case CC_GENMDM_POLYPHONIC_MODE:
        setPolyphonicMode(RANGE(value, 2) != 0);
        break;
    case CC_GENMDM_CH3_SPECIAL_MODE:
        synth_setSpecialMode(RANGE(value, 2) != 0);
        break;
    case CC_RESET_ALL_CONTROLLERS:
        resetAllControllers(chan);
        break;
    case CC_SHOW_PARAMETERS_ON_UI:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        ui_fm_set_parameters_visibility(chan, RANGE(value, 2));
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
    reset();
}
