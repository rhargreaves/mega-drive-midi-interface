#include "midi.h"
#include "comm/comm.h"
#include "log.h"
#include "memcmp.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "midi_fm_sm.h"
#include "midi_dac.h"
#include "midi_sender.h"
#include "synth.h"
#include "ui_fm.h"
#include "note_priority.h"
#include "scheduler.h"
#include "pitchcents.h"
#include "settings.h"

#define MIN_MIDI_VELOCITY 0
#define RANGE(value, range) (value / (128 / range))
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))
#define FOREACH_DEV_CHAN(ch)                                                                       \
    for (DeviceChannel* ch = &deviceChannels[0]; ch < &deviceChannels[DEV_CHANS]; ch++)
#define FOREACH_DEV_CHAN_WITH_MIDI(midiCh, ch)                                                     \
    FOREACH_DEV_CHAN(ch)                                                                           \
        if (ch->midiChannel == midiCh)
#define DEFAULT_PORTAMENTO_INTERVAL 47

typedef enum DeviceSelect { Auto, FM, PSG_Tone, PSG_Noise } DeviceSelect;

typedef struct MidiChannel {
    u8 volume;
    u8 program;
    u8 pan;
    u16 pitchBend;
    u8 lastVelocity;
    NotePriorityStack notePriority;
    DeviceSelect deviceSelect;
    bool portamento;
    u16 portamentoInterval;
    s8 fineTune;
} MidiChannel;

typedef enum MappingMode { MappingMode_Static, MappingMode_Dynamic, MappingMode_Auto } MappingMode;

static const VTable PSG_VTable = { midi_psg_note_on, midi_psg_note_off, midi_psg_channel_volume,
    midi_psg_program, midi_psg_all_notes_off, midi_psg_pan, midi_psg_pitch };

static const VTable FM_VTable = { midi_fm_note_on, midi_fm_note_off, midi_fm_channel_volume,
    midi_fm_program, midi_fm_all_notes_off, midi_fm_pan, midi_fm_pitch };

static const VTable SpecialMode_VTable
    = { midi_fm_sm_note_on, midi_fm_sm_note_off, midi_fm_sm_channel_volume, midi_fm_sm_program,
          midi_fm_sm_all_notes_off, midi_fm_sm_pan, midi_fm_sm_pitch };

static const VTable DAC_VTable = { midi_dac_note_on, midi_dac_note_off, midi_dac_channel_volume,
    midi_dac_program, midi_dac_all_notes_off, midi_dac_pan, midi_dac_pitch };

static const u8** defaultEnvelopes;
static const FmChannel** defaultPresets;
static const PercussionPreset** defaultPercussionPresets;

static const u16 portaTimeToInterval[128]
    = { 1145, 1089, 1036, 985, 937, 892, 848, 807, 768, 730, 694, 661, 628, 598, 569, 541, 514, 489,
          466, 443, 421, 401, 381, 363, 345, 328, 312, 297, 282, 269, 255, 243, 231, 220, 209, 199,
          189, 180, 171, 163, 155, 147, 140, 133, 127, 121, 115, 109, 104, 99, 94, 89, 85, 81, 77,
          73, 70, 66, 63, 60, 57, 54, 52, 49, 47, 44, 42, 40, 38, 36, 35, 33, 31, 30, 28, 27, 26,
          24, 23, 22, 21, 20, 19, 18, 17, 16, 16, 15, 14, 13, 13, 12, 12, 11, 10, 10, 9, 9, 9, 8, 8,
          7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1 };

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
static void updateDeviceChannelFromAssociatedMidiChannel(DeviceChannel* devChan);
static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel);
static void initAllDeviceChannels(void);
static void resetAllState(void);
static void initMidiChannel(u8 midiChan);
static void initDeviceChannel(u8 devChan);
static void reset(void);
static void init(void);
static void devChanNoteOn(DeviceChannel* devChan, u8 pitch, u8 velocity);
static void devChanNoteOff(DeviceChannel* devChan, u8 pitch);
static void setDownstreamPitch(DeviceChannel* devChan);

void midi_init(
    const FmChannel** presets, const PercussionPreset** percussionPresets, const u8** envelopes)
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
    scheduler_addFrameHandler(midi_tick);
    reset();
}

static void reset(void)
{
    midi_psg_reset();
    midi_fm_reset();
    midi_fm_sm_reset();
    mappingModePref = DEFAULT_MAPPING_MODE;
    dynamicMode = mappingModePref == MappingMode_Dynamic;
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
    chan->lastVelocity = 0;
    note_priority_init(&chan->notePriority);
    chan->deviceSelect = Auto;
    chan->portamento = false;
    chan->portamentoInterval = DEFAULT_PORTAMENTO_INTERVAL;
    chan->fineTune = 0;
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
    } else {
        chan->number = devChan - DEV_CHAN_MIN_SPECIAL_MODE;
        chan->ops = &SpecialMode_VTable;
    }
    chan->noteOn = false;
    chan->midiChannel = devChan;
    chan->pitch = 0;
    chan->cents = 0;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
    chan->glideTargetPitch = 0;
    updateDeviceChannelFromAssociatedMidiChannel(chan);
}

static bool isChannelPlayingNote(DeviceChannel* devChan, u8 pitch)
{
    u8 effectivePitch = devChan->glideTargetPitch != 0 ? devChan->glideTargetPitch : devChan->pitch;
    return devChan->noteOn && effectivePitch == pitch;
}

static bool isPsgAndIncomingChanIsPercussive(DeviceChannel* chan, u8 incomingChan)
{
    return chan->ops == &PSG_VTable && incomingChan == GENERAL_MIDI_PERCUSSION_CHANNEL;
}

static bool isChannelSuitable(DeviceChannel* chan, u8 incomingMidiChan)
{
    return !chan->noteOn && !isPsgAndIncomingChanIsPercussive(chan, incomingMidiChan);
}

static DeviceChannel* findFreeMidiAssignedChannel(u8 incomingMidiChan, u8 devChanMin, u8 devChanMax)
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

static DeviceChannel* findAnyFreeChannel(u8 incomingMidiChan, u8 minDevChan, u8 maxDevChan)
{
    for (u16 i = minDevChan; i < maxDevChan; i++) {
        DeviceChannel* chan = &deviceChannels[i];
        if (isChannelSuitable(chan, incomingMidiChan)) {
            return chan;
        }
    }
    return NULL;
}

static DeviceChannel* findDeviceSpecificChannel(u8 incomingMidiChan, u8 minDevChan, u8 maxDevChan)
{
    u8 minChan = minDevChan;
    u8 maxChan = maxDevChan;
    if (stickToDeviceType) {
        DeviceChannel* assignedChan = deviceChannelByMidiChannel(incomingMidiChan);
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

static void setDeviceMinMaxChans(u8 incomingMidiChan, u8* minDevChan, u8* maxDevChan)
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
    DeviceChannel* chan = findFreeMidiAssignedChannel(incomingMidiChan, minDevChan, maxDevChan);
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
    FOREACH_DEV_CHAN_WITH_MIDI(GENERAL_MIDI_PERCUSSION_CHANNEL, chan) {
        if (chan->noteOn) {
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
        devChan->pitchBend = midiChannel->pitchBend;
        setDownstreamPitch(devChan);
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
    FOREACH_DEV_CHAN_WITH_MIDI(midiChannel, chan) {
        return chan;
    }
    return NULL;
}

static void updateDeviceChannelFromAssociatedMidiChannel(DeviceChannel* devChan)
{
    MidiChannel* midiChannel = &midiChannels[devChan->midiChannel];
    midi_fm_percussive(devChan->number, devChan->midiChannel == GENERAL_MIDI_PERCUSSION_CHANNEL);
    updateVolume(midiChannel, devChan);
    updatePan(midiChannel, devChan);
    updateProgram(midiChannel, devChan);
    updatePitchBend(midiChannel, devChan);
}

static DeviceChannel* findSuitableDeviceChannel(u8 midiChan)
{
    return dynamicMode ? findFreeChannel(midiChan) : deviceChannelByMidiChannel(midiChan);
}

static PitchCents effectivePitchCents(DeviceChannel* devChan)
{
    PitchCents pc = pitchcents_bend(devChan->pitch, devChan->cents, devChan->pitchBend);
    return pitchcents_shift(pc, midiChannels[devChan->midiChannel].fineTune);
}

static void setDownstreamPitch(DeviceChannel* devChan)
{
    PitchCents pc = effectivePitchCents(devChan);
    devChan->ops->pitch(devChan->number, pc.pitch, pc.cents);
}

static void setDownstreamNoteOn(DeviceChannel* devChan, u8 velocity)
{
    PitchCents pc = effectivePitchCents(devChan);
    devChan->ops->noteOn(devChan->number, pc.pitch, pc.cents, velocity);
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

    MidiChannel* midiChannel = &midiChannels[chan];
    if (!dynamicMode) {
        if (note_priority_isFull(&midiChannel->notePriority)) {
            log_warn("Ch %d: Note priority stack full", chan + 1);
            return;
        }
        note_priority_push(&midiChannel->notePriority, pitch);
        midiChannel->lastVelocity = velocity;

        if (midiChannel->portamento && note_priority_count(&midiChannel->notePriority) > 1) {
            devChan->glideTargetPitch = pitch;
            return;
        }
        devChan->glideTargetPitch = 0;
    }

    devChan->midiChannel = chan;
    devChan->noteOn = true;
    devChan->cents = 0;
    devChan->pitch = pitch;
    updateDeviceChannelFromAssociatedMidiChannel(devChan);
    setDownstreamNoteOn(devChan, velocity);
}

void midi_note_off(u8 chan, u8 pitch)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    note_priority_remove(&midiChannel->notePriority, pitch);

    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        if (isChannelPlayingNote(devChan, pitch)) {
            u8 nextMostRecentPitch = note_priority_pop(&midiChannel->notePriority);
            if (!dynamicMode && nextMostRecentPitch != 0) {
                if (midiChannel->portamento) {
                    note_priority_push(&midiChannel->notePriority, nextMostRecentPitch);
                    devChan->glideTargetPitch = nextMostRecentPitch;
                } else {
                    devChanNoteOn(devChan, nextMostRecentPitch, midiChannel->lastVelocity);
                }
            } else {
                devChanNoteOff(devChan, pitch);
            }
        }
    }
}

static void devChanNoteOn(DeviceChannel* devChan, u8 pitch, u8 velocity)
{
    devChan->noteOn = true;
    devChan->cents = 0;
    devChan->pitch = pitch;
    setDownstreamNoteOn(devChan, velocity);
}

static void devChanNoteOff(DeviceChannel* devChan, u8 pitch)
{
    devChan->noteOn = false;
    devChan->pitch = 0;
    devChan->ops->noteOff(devChan->number, pitch);
}

static void channelPan(u8 chan, u8 pan)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pan = pan;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        updatePan(midiChannel, devChan);
    }
}

static void channelVolume(u8 chan, u8 volume)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->volume = volume;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        updateVolume(midiChannel, devChan);
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
    FOREACH_DEV_CHAN_WITH_MIDI(chan, state) {
        if (state->noteOn) {
            updatePitchBend(midiChannel, state);
        }
    }
}

void midi_program(u8 chan, u8 program)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->program = program;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, state) {
        updateProgram(midiChannel, state);
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
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        devChan->noteOn = false;
        devChan->pitch = 0;
        devChan->ops->allNotesOff(devChan->number);
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
    synth_directWriteYm2612(part, data[0] << 4 | data[1], data[2] << 4 | data[3]);
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
    if (memcmp(data, GENERAL_MIDI_RESET_SEQ, LENGTH_OF(GENERAL_MIDI_RESET_SEQ)) == 0) {
        generalMidiReset();
        return;
    }

    const u8 CUSTOM_SYSEX_SEQ[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID };
    if (memcmp(data, CUSTOM_SYSEX_SEQ, LENGTH_OF(CUSTOM_SYSEX_SEQ)) == 0) {
        incrementSysExCursor(&data, &length, LENGTH_OF(CUSTOM_SYSEX_SEQ));
        handleCustomSysEx(data, length);
    }
}

static void sendPong(void)
{
    const u8 pongSequence[]
        = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_PONG };

    midi_sender_send_sysex(pongSequence, sizeof(pongSequence));
}

void midi_remap_channel(u8 midiChan, u8 devChan)
{
    const u8 SYSEX_UNASSIGNED_DEVICE_CHANNEL = 0x7F;

    if (devChan == SYSEX_UNASSIGNED_DEVICE_CHANNEL) {
        DeviceChannel* assignedChan = deviceChannelByMidiChannel(midiChan);
        if (assignedChan != NULL) {
            assignedChan->midiChannel = UNASSIGNED_MIDI_CHANNEL;
        }
        return;
    }
    DeviceChannel* chan = &deviceChannels[devChan];
    chan->midiChannel = midiChan;
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
        devChan->midiChannel = dynamicMode ? UNASSIGNED_MIDI_CHANNEL : chan;
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

static void enableDac(bool enable)
{
    deviceChannels[5].ops = enable ? &DAC_VTable : &FM_VTable;
    synth_enableDac(enable);
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
        setOperatorTotalLevel(devChan->number, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorMultiple(
            devChan->number, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorDetune(devChan->number, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorRateScaling(
            devChan->number, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAttackRate(
            devChan->number, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_DECAY_RATE_OP1:
    case CC_GENMDM_DECAY_RATE_OP2:
    case CC_GENMDM_DECAY_RATE_OP3:
    case CC_GENMDM_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorDecayRate(
            devChan->number, controller - CC_GENMDM_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SUSTAIN_RATE_OP1:
    case CC_GENMDM_SUSTAIN_RATE_OP2:
    case CC_GENMDM_SUSTAIN_RATE_OP3:
    case CC_GENMDM_SUSTAIN_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSustainRate(
            devChan->number, controller - CC_GENMDM_SUSTAIN_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SUSTAIN_LEVEL_OP1:
    case CC_GENMDM_SUSTAIN_LEVEL_OP2:
    case CC_GENMDM_SUSTAIN_LEVEL_OP3:
    case CC_GENMDM_SUSTAIN_LEVEL_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSustainLevel(
            devChan->number, controller - CC_GENMDM_SUSTAIN_LEVEL_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorReleaseRate(
            devChan->number, controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorAmplitudeModulation(
            devChan->number, controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operatorSsgEg(devChan->number, controller - CC_GENMDM_SSG_EG_OP1, RANGE(value, 16));
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
        enableDac(RANGE(value, 2));
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
        log_warn("Ch %d: CC 0x%02X 0x%02X?", devChan->midiChannel, controller, value);
        break;
    }
}

static void setInvertTotalLevel(bool invert)
{
    invertTotalLevel = invert;
}

static void fmParameterCC(u8 chan, u8 controller, u8 value)
{
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        setFmChanParameter(devChan, controller, value);
    }
}

static void setPortamentoMode(u8 chan, bool enable)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->portamento = enable;
}

static void setPortamentoTime(u8 chan, u8 value)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->portamentoInterval = portaTimeToInterval[value];
}

static void setFineTune(u8 chan, u8 value)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->fineTune = value - 64;
}

void midi_cc(u8 chan, u8 controller, u8 value)
{
    switch (controller) {
    case CC_PORTAMENTO_TIME_MSB:
        setPortamentoTime(chan, value);
        break;
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
    case CC_PORTAMENTO_ENABLE:
        setPortamentoMode(chan, RANGE(value, 2));
        break;
    case CC_FINE_TUNE:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        setFineTune(chan, value);
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

static void processChannelGlide(DeviceChannel* chan, u16 portamentoTime)
{
    if (chan->glideTargetPitch == 0
        || (chan->glideTargetPitch == chan->pitch && chan->cents == 0)) {
        return;
    }

    s16 effectiveIncrement = 0;

    if (chan->glideTargetPitch > chan->pitch) {
        effectiveIncrement = portamentoTime;
    } else if (chan->glideTargetPitch < chan->pitch
        || (chan->glideTargetPitch == chan->pitch && chan->cents > 0)) {
        effectiveIncrement = 0 - portamentoTime;
    }

    PitchCents pc = { .pitch = chan->pitch, .cents = chan->cents };
    pc = pitchcents_shift(pc, effectiveIncrement);

    if ((effectiveIncrement > 0 && pc.pitch >= chan->glideTargetPitch)
        || (effectiveIncrement < 0 && pc.pitch <= chan->glideTargetPitch)) {
        pc.pitch = chan->glideTargetPitch;
        pc.cents = 0;
        chan->glideTargetPitch = 0;
    }

    chan->pitch = pc.pitch;
    chan->cents = pc.cents;
    setDownstreamPitch(chan);
}

static void processPortamento(void)
{
    FOREACH_DEV_CHAN(chan) {
        if (chan->midiChannel == UNASSIGNED_MIDI_CHANNEL) {
            continue;
        }
        MidiChannel* midiChannel = &midiChannels[chan->midiChannel];
        if (midiChannel->portamento && chan->noteOn) {
            processChannelGlide(chan, midiChannel->portamentoInterval);
        }
    }
}

void midi_tick(void)
{
    processPortamento();
}
