#include "midi.h"
#include "comm/comm.h"
#include "log.h"
#include "memcmp.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "midi_fm_sm.h"
#include "midi_dac.h"
#include "midi_tx.h"
#include "synth.h"
#include "ui_fm.h"
#include "note_priority.h"
#include "scheduler.h"
#include "pitchcents.h"
#include "user_prefs.h"

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
    PitchCents pitchBendRange;
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

static void all_notes_off(u8 chan);
static void general_midi_reset(void);
static void apply_dynamic_mode(void);
static void send_pong(void);
static void set_invert_total_level(bool enabled);
static void set_dynamic_mode(MappingMode mode);
static void set_operator_total_level(u8 chan, u8 op, u8 value);
static void update_device_channel_from_associated_midi_channel(DeviceChannel* devChan);
static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel);
static void init_all_device_channels(void);
static void reset_all_state(void);
static void init_midi_channel(u8 midiChan);
static void init_device_channel(u8 devChan);
static void reset(void);
static void init(void);
static void dev_chan_note_on(DeviceChannel* devChan, u8 pitch, u8 velocity);
static void dev_chan_note_off(DeviceChannel* devChan, u8 pitch);
static void set_downstream_pitch(DeviceChannel* devChan);

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
    reset_all_state();
}

static void reset_all_state(void)
{
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        init_midi_channel(i);
    }
    init_all_device_channels();
    apply_dynamic_mode();
}

static void init_midi_channel(u8 midiChan)
{
    MidiChannel* chan = &midiChannels[midiChan];
    chan->program = 0;
    chan->pan = DEFAULT_MIDI_PAN;
    chan->volume = MAX_MIDI_VOLUME;
    chan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
    chan->pitchBendRange = (PitchCents) { GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE, 0 };
    chan->lastVelocity = 0;
    note_priority_init(&chan->notePriority);
    chan->deviceSelect = Auto;
    chan->portamento = false;
    chan->portamentoInterval = DEFAULT_PORTAMENTO_INTERVAL;
    chan->fineTune = 0;
}

static void init_all_device_channels(void)
{
    for (u16 i = 0; i < DEV_CHANS; i++) {
        init_device_channel(i);
    }
}

static void init_device_channel(u8 devChan)
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
    update_device_channel_from_associated_midi_channel(chan);
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

static void set_device_min_max_chans(u8 incomingMidiChan, u8* minDevChan, u8* maxDevChan)
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
    set_device_min_max_chans(incomingMidiChan, &minDevChan, &maxDevChan);
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

static void update_volume(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->volume != midiChannel->volume) {
        devChan->ops->channelVolume(devChan->number, midiChannel->volume);
        devChan->volume = midiChannel->volume;
    }
}

static void update_pan(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->pan != midiChannel->pan) {
        devChan->ops->pan(devChan->number, midiChannel->pan);
        devChan->pan = midiChannel->pan;
    }
}

static void update_pitch_bend(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->pitchBend != midiChannel->pitchBend) {
        devChan->pitchBend = midiChannel->pitchBend;
        set_downstream_pitch(devChan);
    }
}

static void update_program(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->program != midiChannel->program) {
        devChan->ops->program(devChan->number, midiChannel->program);
        devChan->program = midiChannel->program;
    }
}

static void channel_device_select(u8 midiChan, DeviceSelect deviceSelect)
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

static void update_device_channel_from_associated_midi_channel(DeviceChannel* devChan)
{
    MidiChannel* midiChannel = &midiChannels[devChan->midiChannel];
    midi_fm_percussive(devChan->number, devChan->midiChannel == GENERAL_MIDI_PERCUSSION_CHANNEL);
    update_volume(midiChannel, devChan);
    update_pan(midiChannel, devChan);
    update_program(midiChannel, devChan);
    update_pitch_bend(midiChannel, devChan);
}

static DeviceChannel* findSuitableDeviceChannel(u8 midiChan)
{
    return dynamicMode ? findFreeChannel(midiChan) : deviceChannelByMidiChannel(midiChan);
}

static PitchCents effectivePitchCents(DeviceChannel* devChan)
{
    MidiChannel* midiChannel = &midiChannels[devChan->midiChannel];
    PitchCents pc
        = pc_bend(devChan->pitch, devChan->cents, devChan->pitchBend, midiChannel->pitchBendRange);
    return pc_shift(pc, midiChannel->fineTune);
}

static void set_downstream_pitch(DeviceChannel* devChan)
{
    PitchCents pc = effectivePitchCents(devChan);
    devChan->ops->pitch(devChan->number, pc.pitch, pc.cents);
}

static void set_downstream_note_on(DeviceChannel* devChan, u8 velocity)
{
    PitchCents pc = effectivePitchCents(devChan);
    devChan->ops->noteOn(devChan->number, pc.pitch, pc.cents, velocity);
}

static void reapply_program_if_channel_was_formerly_percussive(DeviceChannel* devChan, u8 midiChan)
{
    u8 prevMidiChan = devChan->midiChannel;
    if (prevMidiChan == GENERAL_MIDI_PERCUSSION_CHANNEL
        && midiChan != GENERAL_MIDI_PERCUSSION_CHANNEL) {
        devChan->ops->program(devChan->number, midiChannels[midiChan].program);
    }
}

void midi_note_on(u8 midiChan, u8 pitch, u8 velocity)
{
    if (velocity == MIN_MIDI_VELOCITY) {
        midi_note_off(midiChan, pitch);
        return;
    }
    if (tooManyPercussiveNotes(midiChan)) {
        return;
    }
    DeviceChannel* devChan = findSuitableDeviceChannel(midiChan);
    if (devChan == NULL) {
        log_warn("Ch %d: Dropped note %d", midiChan + 1, pitch);
        return;
    }

    MidiChannel* midiChannel = &midiChannels[midiChan];
    if (!dynamicMode) {
        if (note_priority_isFull(&midiChannel->notePriority)) {
            log_warn("Ch %d: Note priority stack full", midiChan + 1);
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

    reapply_program_if_channel_was_formerly_percussive(devChan, midiChan);
    devChan->midiChannel = midiChan;
    devChan->noteOn = true;
    devChan->cents = 0;
    devChan->pitch = pitch;
    update_device_channel_from_associated_midi_channel(devChan);
    set_downstream_note_on(devChan, velocity);
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
                    dev_chan_note_on(devChan, nextMostRecentPitch, midiChannel->lastVelocity);
                }
            } else {
                dev_chan_note_off(devChan, pitch);
            }
        }
    }
}

static void dev_chan_note_on(DeviceChannel* devChan, u8 pitch, u8 velocity)
{
    devChan->noteOn = true;
    devChan->cents = 0;
    devChan->pitch = pitch;
    set_downstream_note_on(devChan, velocity);
}

static void dev_chan_note_off(DeviceChannel* devChan, u8 pitch)
{
    devChan->noteOn = false;
    devChan->pitch = 0;
    devChan->ops->noteOff(devChan->number, pitch);
}

static void channel_pan(u8 chan, u8 pan)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->pan = pan;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        update_pan(midiChannel, devChan);
    }
}

static void channel_volume(u8 chan, u8 volume)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->volume = volume;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        update_volume(midiChannel, devChan);
    }
}

void resetAllControllers(u8 chan)
{
    init_midi_channel(chan);
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* state = &deviceChannels[i];
        if (state->midiChannel == chan) {
            init_device_channel(i);
        }
    }
    set_dynamic_mode(dynamicMode);
}

static bool isIgnoringNonGeneralMidiCCs(void)
{
    return disableNonGeneralMidiCCs;
}

static void set_polyphonic_mode(bool enable)
{
    set_dynamic_mode(enable);
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
            update_pitch_bend(midiChannel, state);
        }
    }
}

void midi_program(u8 chan, u8 program)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->program = program;
    FOREACH_DEV_CHAN_WITH_MIDI(chan, state) {
        update_program(midiChannel, state);
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

static void all_notes_off(u8 chan)
{
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        devChan->noteOn = false;
        devChan->pitch = 0;
        devChan->ops->allNotesOff(devChan->number);
    }
}

static void set_non_general_midi_ccs(bool enable)
{
    disableNonGeneralMidiCCs = !enable;
}

static void set_stick_to_device_type(bool enable)
{
    stickToDeviceType = enable;
}

static void load_psg_envelope(const u8* data, u16 length)
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

static void increment_sysex_cursor(const u8** data, u16* length, u8 value)
{
    (*data) += value;
    (*length) -= value;
}

static void direct_write_ym2612(u8 part, const u8* data, u16 length)
{
    if (length != 4) {
        return;
    }
    synth_direct_write_ym2612(part, data[0] << 4 | data[1], data[2] << 4 | data[3]);
}

static void handle_custom_sysex(const u8* data, u16 length)
{
    u8 command = *data;
    increment_sysex_cursor(&data, &length, 1);
    switch (command) {
    case SYSEX_COMMAND_REMAP:
        if (length == 2) {
            midi_remap_channel(data[0], data[1]);
        }
        break;
    case SYSEX_COMMAND_PING:
        if (length == 0) {
            send_pong();
        }
        break;
    case SYSEX_COMMAND_DYNAMIC:
        if (length == 1) {
            set_dynamic_mode((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_NON_GENERAL_MIDI_CCS:
        if (length == 1) {
            set_non_general_midi_ccs((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_STICK_TO_DEVICE_TYPE:
        if (length == 1) {
            set_stick_to_device_type((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_INVERT_TOTAL_LEVEL:
        if (length == 1) {
            set_invert_total_level((bool)data[0]);
        }
        break;
    case SYSEX_COMMAND_LOAD_PSG_ENVELOPE:
        load_psg_envelope(data, length);
        break;
    case SYSEX_COMMAND_WRITE_YM2612_REG_PART_0:
        direct_write_ym2612(0, data, length);
        break;
    case SYSEX_COMMAND_WRITE_YM2612_REG_PART_1:
        direct_write_ym2612(1, data, length);
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
        general_midi_reset();
        return;
    }

    const u8 CUSTOM_SYSEX_SEQ[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID };
    if (memcmp(data, CUSTOM_SYSEX_SEQ, LENGTH_OF(CUSTOM_SYSEX_SEQ)) == 0) {
        increment_sysex_cursor(&data, &length, LENGTH_OF(CUSTOM_SYSEX_SEQ));
        handle_custom_sysex(data, length);
    }
}

static void send_pong(void)
{
    const u8 pongSequence[]
        = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_PONG };

    midi_tx_send_sysex(pongSequence, sizeof(pongSequence));
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

static void general_midi_reset(void)
{
    for (u8 chan = 0; chan < MIDI_CHANNELS; chan++) {
        all_notes_off(chan);
    }
    if (mappingModePref == MappingMode_Auto) {
        dynamicMode = true;
    }
    reset_all_state();
}

static void apply_dynamic_mode(void)
{
    for (u8 chan = 0; chan < DEV_CHANS; chan++) {
        DeviceChannel* devChan = &deviceChannels[chan];
        devChan->midiChannel = dynamicMode ? UNASSIGNED_MIDI_CHANNEL : chan;
    }
}

static void set_dynamic_mode(MappingMode mode)
{
    mappingModePref = mode;
    if (mode == MappingMode_Dynamic) {
        dynamicMode = true;
        apply_dynamic_mode();
    } else if (mode == MappingMode_Static) {
        dynamicMode = false;
        apply_dynamic_mode();
    }
}

static void set_operator_total_level(u8 chan, u8 op, u8 value)
{
    const u8 MAX_TOTAL_LEVEL = 127;

    if (invertTotalLevel) {
        value = MAX_TOTAL_LEVEL - value;
    }
    synth_operator_total_level(chan, op, value);
}

static void enable_dac(bool enable)
{
    deviceChannels[5].ops = enable ? &DAC_VTable : &FM_VTable;
    synth_enable_dac(enable);
}

static void set_fm_chan_parameter(DeviceChannel* devChan, u8 controller, u8 value)
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
        set_operator_total_level(devChan->number, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_multiple(
            devChan->number, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_detune(devChan->number, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_rate_scaling(
            devChan->number, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_attack_rate(
            devChan->number, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_DECAY_RATE_OP1:
    case CC_GENMDM_DECAY_RATE_OP2:
    case CC_GENMDM_DECAY_RATE_OP3:
    case CC_GENMDM_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_decay_rate(
            devChan->number, controller - CC_GENMDM_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SUSTAIN_RATE_OP1:
    case CC_GENMDM_SUSTAIN_RATE_OP2:
    case CC_GENMDM_SUSTAIN_RATE_OP3:
    case CC_GENMDM_SUSTAIN_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_sustain_rate(
            devChan->number, controller - CC_GENMDM_SUSTAIN_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SUSTAIN_LEVEL_OP1:
    case CC_GENMDM_SUSTAIN_LEVEL_OP2:
    case CC_GENMDM_SUSTAIN_LEVEL_OP3:
    case CC_GENMDM_SUSTAIN_LEVEL_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_sustain_level(
            devChan->number, controller - CC_GENMDM_SUSTAIN_LEVEL_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_release_rate(
            devChan->number, controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_amplitude_modulation(
            devChan->number, controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_ssg_eg(devChan->number, controller - CC_GENMDM_SSG_EG_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_GLOBAL_LFO_ENABLE:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_enable_lfo(RANGE(value, 2));
        break;
    case CC_GENMDM_GLOBAL_LFO_FREQUENCY:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_global_lfo_frequency(RANGE(value, 8));
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
        enable_dac(RANGE(value, 2));
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

static void set_invert_total_level(bool invert)
{
    invertTotalLevel = invert;
}

static void fm_parameter_cc(u8 chan, u8 controller, u8 value)
{
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        set_fm_chan_parameter(devChan, controller, value);
    }
}

static void set_portamento_mode(u8 chan, bool enable)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->portamento = enable;
}

static void set_portamento_time(u8 chan, u8 value)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->portamentoInterval = portaTimeToInterval[value];
}

static void set_fine_tune(u8 chan, u8 value)
{
    MidiChannel* midiChannel = &midiChannels[chan];
    midiChannel->fineTune = value - 64;
}

void midi_cc(u8 chan, u8 controller, u8 value)
{
    switch (controller) {
    case CC_PORTAMENTO_TIME_MSB:
        set_portamento_time(chan, value);
        break;
    case CC_VOLUME:
        channel_volume(chan, value);
        break;
    case CC_PAN:
        channel_pan(chan, value);
        break;
    case CC_ALL_NOTES_OFF:
    case CC_ALL_SOUND_OFF:
        all_notes_off(chan);
        break;
    case CC_GENMDM_POLYPHONIC_MODE:
        set_polyphonic_mode(RANGE(value, 2) != 0);
        break;
    case CC_GENMDM_CH3_SPECIAL_MODE:
        synth_set_special_mode(RANGE(value, 2) != 0);
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
        channel_device_select(chan, RANGE(value, 4));
        break;
    case CC_PORTAMENTO_ENABLE:
        set_portamento_mode(chan, RANGE(value, 2));
        break;
    case CC_FINE_TUNE:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        set_fine_tune(chan, value);
        break;
    default:
        fm_parameter_cc(chan, controller, value);
        break;
    }
}

void midi_reset(void)
{
    reset();
}

static void process_channel_glide(DeviceChannel* chan, u16 portamentoTime)
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
    pc = pc_shift(pc, effectiveIncrement);

    if ((effectiveIncrement > 0 && pc.pitch >= chan->glideTargetPitch)
        || (effectiveIncrement < 0 && pc.pitch <= chan->glideTargetPitch)) {
        pc.pitch = chan->glideTargetPitch;
        pc.cents = 0;
        chan->glideTargetPitch = 0;
    }

    chan->pitch = pc.pitch;
    chan->cents = pc.cents;
    set_downstream_pitch(chan);
}

static void process_portamento(void)
{
    FOREACH_DEV_CHAN(chan) {
        if (chan->midiChannel == UNASSIGNED_MIDI_CHANNEL) {
            continue;
        }
        MidiChannel* midiChannel = &midiChannels[chan->midiChannel];
        if (midiChannel->portamento && chan->noteOn) {
            process_channel_glide(chan, midiChannel->portamentoInterval);
        }
    }
}

void midi_tick(u16 delta)
{
    process_portamento();
}

void midi_rpn_msb(u16 rpn, u8 value)
{
}

void midi_rpn_lsb(u16 rpn, u8 value)
{
}