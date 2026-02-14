#include "midi.h"
#include "comm/comm.h"
#include "log.h"
#include "utils.h"
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

#define FOREACH_DEV_CHAN(ch)                                                                       \
    for (DeviceChannel* ch = &deviceChannels[0]; ch < &deviceChannels[DEV_CHANS]; ch++)
#define FOREACH_DEV_CHAN_WITH_MIDI(midiCh, ch)                                                     \
    FOREACH_DEV_CHAN(ch)                                                                           \
        if (ch->midiChannel == midiCh)
#define DEFAULT_PORTAMENTO_INTERVAL 47

typedef enum DeviceSelect { Auto, FM, PSG_Tone, PSG_Noise } DeviceSelect;

typedef struct MidiChannel {
    u8 num;
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
    u16 rpn;
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
static const FmPreset** defaultPresets;
static const PercussionPreset** defaultPercussionPresets;

static const u16 portaTimeToInterval[128]
    = { 1145, 1089, 1036, 985, 937, 892, 848, 807, 768, 730, 694, 661, 628, 598, 569, 541, 514, 489,
          466, 443, 421, 401, 381, 363, 345, 328, 312, 297, 282, 269, 255, 243, 231, 220, 209, 199,
          189, 180, 171, 163, 155, 147, 140, 133, 127, 121, 115, 109, 104, 99, 94, 89, 85, 81, 77,
          73, 70, 66, 63, 60, 57, 54, 52, 49, 47, 44, 42, 40, 38, 36, 35, 33, 31, 30, 28, 27, 26,
          24, 23, 22, 21, 20, 19, 18, 17, 16, 16, 15, 14, 13, 13, 12, 12, 11, 10, 10, 9, 9, 9, 8, 8,
          7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1 };

static DeviceChannel deviceChannels[DEV_CHANS];
static MidiChannel midiChannels[MIDI_CHANNELS];
static bool dynamicMode;
static void (*changeCallback)(MidiChangeEvent);

typedef struct MidiConfig {
    MappingMode mappingModePref;
    bool disableNonGeneralMidiCCs;
    bool stickToDeviceType;
    bool invertTotalLevel;
} MidiConfig;

static MidiConfig config;

static void all_notes_off(u8 ch);
static void general_midi_reset(void);
static void apply_dynamic_mode(void);
static void send_pong(const u8* data, u16 length);
static void set_invert_total_level(const u8* data, u16 length);
static void set_dynamic_mode(MappingMode mode);
static void set_operator_total_level(u8 chan, u8 op, u8 value);
static void update_device_channel_from_associated_midi_channel(DeviceChannel* devChan);
static DeviceChannel* deviceChannelByMidiChannel(u8 midiChannel);
static void reset_channels(void);
static void init_midi_channel(u8 midiChan);
static void init_device_channel(u8 devChan);
static void reset(void);
static void dev_chan_note_on(DeviceChannel* devChan, u8 pitch, u8 velocity);
static void dev_chan_note_off(DeviceChannel* devChan, u8 pitch);
static void set_downstream_pitch(DeviceChannel* devChan);
static bool valid_midi_channel(u8 midiChan);
static bool valid_device_channel(u8 devChan);

void midi_init(
    const FmPreset** presets, const PercussionPreset** percussionPresets, const u8** envelopes)
{
    defaultEnvelopes = envelopes;
    defaultPresets = presets;
    defaultPercussionPresets = percussionPresets;
    changeCallback = NULL;
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
    config = (MidiConfig) {
        .mappingModePref = DEFAULT_MAPPING_MODE,
        .disableNonGeneralMidiCCs = false,
        .stickToDeviceType = false,
        .invertTotalLevel = false,
    };
    dynamicMode = config.mappingModePref == MappingMode_Dynamic;
    reset_channels();
}

static void reset_channels(void)
{
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        init_midi_channel(i);
    }
    for (u16 i = 0; i < DEV_CHANS; i++) {
        init_device_channel(i);
    }
    apply_dynamic_mode();
}

static void init_midi_channel(u8 midiChan)
{
    MidiChannel* chan = &midiChannels[midiChan];
    chan->num = midiChan;
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
    chan->rpn = NULL_RPN;
}

static void init_device_channel(u8 devChan)
{
    DeviceChannel* chan = &deviceChannels[devChan];
    if (devChan <= DEV_CHAN_MAX_FM) {
        chan->num = devChan;
        chan->ops = &FM_VTable;
    } else if (devChan <= DEV_CHAN_MAX_PSG) {
        chan->num = devChan - DEV_CHAN_MIN_PSG;
        chan->ops = &PSG_VTable;
    } else {
        chan->num = devChan - DEV_CHAN_MIN_SPECIAL_MODE;
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
    if (config.stickToDeviceType) {
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
        devChan->ops->channelVolume(devChan->num, midiChannel->volume);
        devChan->volume = midiChannel->volume;
    }
}

static void update_pan(MidiChannel* midiChannel, DeviceChannel* devChan)
{
    if (devChan->pan != midiChannel->pan) {
        devChan->ops->pan(devChan->num, midiChannel->pan);
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
        devChan->ops->program(devChan->num, midiChannel->program);
        devChan->program = midiChannel->program;
    }
}

static void channel_device_select(MidiChannel* chan, DeviceSelect deviceSelect)
{
    chan->deviceSelect = deviceSelect;
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
    midi_fm_percussive(devChan->num, devChan->midiChannel == GENERAL_MIDI_PERCUSSION_CHANNEL);
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
    devChan->ops->pitch(devChan->num, pc.pitch, pc.cents);
}

static void set_downstream_note_on(DeviceChannel* devChan, u8 velocity)
{
    PitchCents pc = effectivePitchCents(devChan);
    devChan->ops->noteOn(devChan->num, pc.pitch, pc.cents, velocity);
}

static void reapply_program_if_channel_was_formerly_percussive(DeviceChannel* devChan, u8 midiChan)
{
    u8 prevMidiChan = devChan->midiChannel;
    if (prevMidiChan == GENERAL_MIDI_PERCUSSION_CHANNEL
        && midiChan != GENERAL_MIDI_PERCUSSION_CHANNEL) {
        devChan->ops->program(devChan->num, midiChannels[midiChan].program);
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
    devChan->ops->noteOff(devChan->num, pitch);
}

static void channel_pan(MidiChannel* chan, u8 pan)
{
    chan->pan = pan;
    FOREACH_DEV_CHAN_WITH_MIDI(chan->num, devChan) {
        update_pan(chan, devChan);
    }
}

static void channel_volume(MidiChannel* chan, u8 volume)
{
    chan->volume = volume;
    FOREACH_DEV_CHAN_WITH_MIDI(chan->num, devChan) {
        update_volume(chan, devChan);
    }
}

void resetAllControllers(u8 ch)
{
    init_midi_channel(ch);
    for (u8 i = 0; i < DEV_CHANS; i++) {
        DeviceChannel* state = &deviceChannels[i];
        if (state->midiChannel == ch) {
            init_device_channel(i);
        }
    }
    set_dynamic_mode(dynamicMode);
}

static bool isIgnoringNonGeneralMidiCCs(void)
{
    return config.disableNonGeneralMidiCCs;
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

    if (changeCallback != NULL) {
        MidiChangeEvent event = {
            .type = MidiChangeType_Program,
            .chan = chan,
            .value = midiChannel->program,
        };
        changeCallback(event);
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

static void all_notes_off(u8 ch)
{
    FOREACH_DEV_CHAN_WITH_MIDI(ch, devChan) {
        devChan->noteOn = false;
        devChan->pitch = 0;
        devChan->ops->allNotesOff(devChan->num);
    }
}

static void set_non_general_midi_ccs(const u8* data, u16 length)
{
    config.disableNonGeneralMidiCCs = !data[0];
}

static void set_stick_to_device_type(const u8* data, u16 length)
{
    config.stickToDeviceType = data[0];
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

static void store_program(const u8* data, u16 length)
{
    u8 type = data[0];
    u8 program = data[1];

    switch (type) {
    case STORE_PROGRAM_TYPE_FM: {
        FmPreset fmPreset;
        fmPreset.algorithm = data[2];
        fmPreset.feedback = data[3];
        fmPreset.ams = data[4];
        fmPreset.fms = data[5];
        for (u8 i = 0; i < MAX_FM_OPERATORS; i++) {
            fmPreset.operators[i].multiple = data[6 + i * 11];
            fmPreset.operators[i].detune = data[7 + i * 11];
            fmPreset.operators[i].attackRate = data[8 + i * 11];
            fmPreset.operators[i].rateScaling = data[9 + i * 11];
            fmPreset.operators[i].decayRate = data[10 + i * 11];
            fmPreset.operators[i].amplitudeModulation = data[11 + i * 11];
            fmPreset.operators[i].sustainLevel = data[12 + i * 11];
            fmPreset.operators[i].sustainRate = data[13 + i * 11];
            fmPreset.operators[i].releaseRate = data[14 + i * 11];
            fmPreset.operators[i].totalLevel = data[15 + i * 11];
            fmPreset.operators[i].ssgEg = data[16 + i * 11];
        }
        midi_fm_store_preset(program, &fmPreset);
        log_info("Prg %d: FM preset stored", program);
        break;
    }
    default:
        log_warn("Invalid store program type: %d", type);
        break;
    }
}

static void clear_program(const u8* data, u16 length)
{
    u8 type = data[0];
    u8 program = data[1];

    switch (type) {
    case STORE_PROGRAM_TYPE_FM:
        midi_fm_clear_preset(program);
        log_info("Prg %d: FM preset cleared", program);
        break;
    default:
        log_warn("Invalid clear program type: %d", type);
        break;
    }
}

static void clear_all_programs(const u8* data, u16 length)
{
    u8 type = data[0];

    switch (type) {
    case STORE_PROGRAM_TYPE_FM: {
        for (u8 i = 0; i < MIDI_PROGRAMS; i++) {
            midi_fm_clear_preset(i);
        }
        log_info("All FM presets cleared");
        break;
    }
    default:
        log_warn("Invalid clear all programs type: %d", type);
        break;
    }
}

static void direct_write_ym2612_part_0(const u8* data, u16 length)
{
    direct_write_ym2612(0, data, length);
}

static void direct_write_ym2612_part_1(const u8* data, u16 length)
{
    direct_write_ym2612(1, data, length);
}

static void set_dynamic_mode_sysex(const u8* data, u16 length)
{
    set_dynamic_mode((bool)data[0]);
}

static void midi_remap_channel_sysex(const u8* data, u16 length)
{
    midi_remap_channel(data[0], data[1]);
}

static void send_preset_dump(u8 context, u8 command, const FmPreset* preset)
{
    u8 sysexData[4 + 2 + 4 + (MAX_FM_OPERATORS * 11)];
    u16 index = 0;

    sysexData[index++] = SYSEX_MANU_EXTENDED;
    sysexData[index++] = SYSEX_MANU_REGION;
    sysexData[index++] = SYSEX_MANU_ID;
    sysexData[index++] = command;

    sysexData[index++] = STORE_PROGRAM_TYPE_FM;
    sysexData[index++] = context;

    sysexData[index++] = preset->algorithm;
    sysexData[index++] = preset->feedback;
    sysexData[index++] = preset->ams;
    sysexData[index++] = preset->fms;

    for (u8 i = 0; i < MAX_FM_OPERATORS; i++) {
        sysexData[index++] = preset->operators[i].multiple;
        sysexData[index++] = preset->operators[i].detune;
        sysexData[index++] = preset->operators[i].attackRate;
        sysexData[index++] = preset->operators[i].rateScaling;
        sysexData[index++] = preset->operators[i].decayRate;
        sysexData[index++] = preset->operators[i].amplitudeModulation;
        sysexData[index++] = preset->operators[i].sustainLevel;
        sysexData[index++] = preset->operators[i].sustainRate;
        sysexData[index++] = preset->operators[i].releaseRate;
        sysexData[index++] = preset->operators[i].totalLevel;
        sysexData[index++] = preset->operators[i].ssgEg;
    }

    midi_tx_send_sysex(sysexData, index);
}

static void dump_channel_request(const u8* data, u16 length)
{
    u8 type = data[0];
    u8 midiChannel = data[1];

    if (!valid_midi_channel(midiChannel)) {
        return;
    }

    switch (type) {
    case STORE_PROGRAM_TYPE_FM: {
        DeviceChannel* devChan = deviceChannelByMidiChannel(midiChannel);
        if (devChan == NULL || devChan->ops != &FM_VTable) {
            log_warn("Ch %d: No FM channel assigned", midiChannel + 1);
            return;
        }
        FmPreset preset;
        synth_extract_preset(devChan->num, &preset);
        send_preset_dump(midiChannel, SYSEX_COMMAND_CHANNEL_DATA, &preset);
        log_info("Ch %d: FM %d dumped", midiChannel + 1, devChan->num);
        break;
    }
    default:
        log_warn("Invalid dump channel request type: %d", type);
        break;
    }
}

static void dump_preset_request(const u8* data, u16 length)
{
    u8 type = data[0];
    u8 program = data[1];

    switch (type) {
    case STORE_PROGRAM_TYPE_FM: {
        const FmPreset* preset = midi_fm_get_stored_preset(program);
        if (preset != NULL) {
            send_preset_dump(program, SYSEX_COMMAND_PRESET_DATA, preset);
            log_info("Prg %d: FM preset dumped", program);
        } else {
            log_warn("Prg %d: No FM preset to dump", program);
        }
        break;
    }
    default:
        log_warn("Invalid dump preset request type: %d", type);
        break;
    }
}

typedef struct SysexCommand {
    u8 command;
    void (*handler)(const u8* data, u16 length);
    u16 length;
    bool validateLength;
} SysexCommand;

static const SysexCommand SYSEX_COMMANDS[] = {
    { SYSEX_COMMAND_REMAP, midi_remap_channel_sysex, 2, true },
    { SYSEX_COMMAND_PING, send_pong, 0, true },
    { SYSEX_COMMAND_PONG, NULL, 0, true },
    { SYSEX_COMMAND_DYNAMIC, set_dynamic_mode_sysex, 1, true },
    { SYSEX_COMMAND_NON_GENERAL_MIDI_CCS, set_non_general_midi_ccs, 1, true },
    { SYSEX_COMMAND_STICK_TO_DEVICE_TYPE, set_stick_to_device_type, 1, true },
    { SYSEX_COMMAND_LOAD_PSG_ENVELOPE, load_psg_envelope, 0, false },
    { SYSEX_COMMAND_INVERT_TOTAL_LEVEL, set_invert_total_level, 1, true },
    { SYSEX_COMMAND_WRITE_YM2612_REG_PART_0, direct_write_ym2612_part_0, 4, true },
    { SYSEX_COMMAND_WRITE_YM2612_REG_PART_1, direct_write_ym2612_part_1, 4, true },
    { SYSEX_COMMAND_STORE_PROGRAM, store_program, 6 + (MAX_FM_OPERATORS * 11), true },
    { SYSEX_COMMAND_CLEAR_PROGRAM, clear_program, 2, true },
    { SYSEX_COMMAND_CLEAR_ALL_PROGRAMS, clear_all_programs, 1, true },
    { SYSEX_COMMAND_DUMP_PRESET, dump_preset_request, 2, true },
    { SYSEX_COMMAND_PRESET_DATA, NULL, 0, false },
    { SYSEX_COMMAND_DUMP_CHANNEL, dump_channel_request, 2, true },
    { SYSEX_COMMAND_CHANNEL_DATA, NULL, 0, false },
};

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

    const u8 SYSEX_COMMAND_HEAD[] = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID };
    if (memcmp(data, SYSEX_COMMAND_HEAD, LENGTH_OF(SYSEX_COMMAND_HEAD)) != 0) {
        return;
    }
    increment_sysex_cursor(&data, &length, LENGTH_OF(SYSEX_COMMAND_HEAD));

    const u8 command = data[0];
    if (command >= LENGTH_OF(SYSEX_COMMANDS)) {
        log_warn("Sysex %02X: Invalid command", command);
        return;
    }

    const SysexCommand* cmd = &SYSEX_COMMANDS[command];
    if (cmd->handler == NULL) {
        log_warn("Sysex %02X: Invalid command", command);
        return;
    }

    increment_sysex_cursor(&data, &length, 1);
    if (cmd->validateLength && length != cmd->length) {
        log_warn("Sysex %02X: Invalid length: %d (!= %d)", command, length, cmd->length);
        return;
    }

    cmd->handler(data, length);
}

static void send_pong(const u8* data, u16 length)
{
    const u8 pongSequence[]
        = { SYSEX_MANU_EXTENDED, SYSEX_MANU_REGION, SYSEX_MANU_ID, SYSEX_COMMAND_PONG };

    midi_tx_send_sysex(pongSequence, sizeof(pongSequence));
}

static bool valid_midi_channel(u8 midiChan)
{
    if (midiChan >= MIDI_CHANNELS) {
        log_warn("Invalid MIDI channel: %d", midiChan);
        return false;
    }
    return true;
}

static bool valid_device_channel(u8 devChan)
{
    if (devChan >= DEV_CHANS) {
        log_warn("Invalid device channel: %d", devChan);
        return false;
    }
    return true;
}

void midi_remap_channel(u8 midiChan, u8 devChan)
{
    const u8 SYSEX_UNASSIGNED_DEVICE_CHANNEL = 0x7F;

    if (midiChan != UNASSIGNED_MIDI_CHANNEL && !valid_midi_channel(midiChan)) {
        return;
    }

    if (devChan == SYSEX_UNASSIGNED_DEVICE_CHANNEL) {
        DeviceChannel* assignedChan = deviceChannelByMidiChannel(midiChan);
        if (assignedChan != NULL) {
            assignedChan->midiChannel = UNASSIGNED_MIDI_CHANNEL;
        }
        return;
    }

    if (!valid_device_channel(devChan)) {
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
    if (config.mappingModePref == MappingMode_Auto) {
        dynamicMode = true;
    }
    reset_channels();
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
    config.mappingModePref = mode;
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

    if (config.invertTotalLevel) {
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
        synth_algorithm(devChan->num, RANGE(value, 8));
        break;
    case CC_GENMDM_FM_FEEDBACK:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_feedback(devChan->num, RANGE(value, 8));
        break;
    case CC_GENMDM_TOTAL_LEVEL_OP1:
    case CC_GENMDM_TOTAL_LEVEL_OP2:
    case CC_GENMDM_TOTAL_LEVEL_OP3:
    case CC_GENMDM_TOTAL_LEVEL_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        set_operator_total_level(devChan->num, controller - CC_GENMDM_TOTAL_LEVEL_OP1, value);
        break;
    case CC_GENMDM_MULTIPLE_OP1:
    case CC_GENMDM_MULTIPLE_OP2:
    case CC_GENMDM_MULTIPLE_OP3:
    case CC_GENMDM_MULTIPLE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_multiple(
            devChan->num, controller - CC_GENMDM_MULTIPLE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_DETUNE_OP1:
    case CC_GENMDM_DETUNE_OP2:
    case CC_GENMDM_DETUNE_OP3:
    case CC_GENMDM_DETUNE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_detune(devChan->num, controller - CC_GENMDM_DETUNE_OP1, RANGE(value, 8));
        break;
    case CC_GENMDM_RATE_SCALING_OP1:
    case CC_GENMDM_RATE_SCALING_OP2:
    case CC_GENMDM_RATE_SCALING_OP3:
    case CC_GENMDM_RATE_SCALING_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_rate_scaling(
            devChan->num, controller - CC_GENMDM_RATE_SCALING_OP1, RANGE(value, 4));
        break;
    case CC_GENMDM_ATTACK_RATE_OP1:
    case CC_GENMDM_ATTACK_RATE_OP2:
    case CC_GENMDM_ATTACK_RATE_OP3:
    case CC_GENMDM_ATTACK_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_attack_rate(
            devChan->num, controller - CC_GENMDM_ATTACK_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_DECAY_RATE_OP1:
    case CC_GENMDM_DECAY_RATE_OP2:
    case CC_GENMDM_DECAY_RATE_OP3:
    case CC_GENMDM_DECAY_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_decay_rate(
            devChan->num, controller - CC_GENMDM_DECAY_RATE_OP1, RANGE(value, 32));
        break;
    case CC_GENMDM_SUSTAIN_RATE_OP1:
    case CC_GENMDM_SUSTAIN_RATE_OP2:
    case CC_GENMDM_SUSTAIN_RATE_OP3:
    case CC_GENMDM_SUSTAIN_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_sustain_rate(
            devChan->num, controller - CC_GENMDM_SUSTAIN_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_SUSTAIN_LEVEL_OP1:
    case CC_GENMDM_SUSTAIN_LEVEL_OP2:
    case CC_GENMDM_SUSTAIN_LEVEL_OP3:
    case CC_GENMDM_SUSTAIN_LEVEL_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_sustain_level(
            devChan->num, controller - CC_GENMDM_SUSTAIN_LEVEL_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_RELEASE_RATE_OP1:
    case CC_GENMDM_RELEASE_RATE_OP2:
    case CC_GENMDM_RELEASE_RATE_OP3:
    case CC_GENMDM_RELEASE_RATE_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_release_rate(
            devChan->num, controller - CC_GENMDM_RELEASE_RATE_OP1, RANGE(value, 16));
        break;
    case CC_GENMDM_AMPLITUDE_MODULATION_OP1:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP2:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP3:
    case CC_GENMDM_AMPLITUDE_MODULATION_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_amplitude_modulation(
            devChan->num, controller - CC_GENMDM_AMPLITUDE_MODULATION_OP1, RANGE(value, 2));
        break;
    case CC_GENMDM_SSG_EG_OP1:
    case CC_GENMDM_SSG_EG_OP2:
    case CC_GENMDM_SSG_EG_OP3:
    case CC_GENMDM_SSG_EG_OP4:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_operator_ssg_eg(devChan->num, controller - CC_GENMDM_SSG_EG_OP1, RANGE(value, 16));
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
        synth_ams(devChan->num, RANGE(value, 4));
        break;
    case CC_GENMDM_FMS:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_fms(devChan->num, RANGE(value, 8));
        break;
    case CC_GENMDM_STEREO:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        synth_stereo(devChan->num, RANGE(value, 4));
        break;
    case CC_GENMDM_ENABLE_DAC:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        enable_dac(RANGE(value, 2));
        break;
    default:
        log_warn("Ch %d: CC 0x%02X 0x%02X?", devChan->midiChannel + 1, controller, value);
        break;
    }
}

static void set_invert_total_level(const u8* data, u16 length)
{
    config.invertTotalLevel = data[0];
}

static void fm_parameter_cc(u8 chan, u8 controller, u8 value)
{
    FOREACH_DEV_CHAN_WITH_MIDI(chan, devChan) {
        set_fm_chan_parameter(devChan, controller, value);
    }
}

static void set_portamento_mode(MidiChannel* chan, bool enable)
{
    chan->portamento = enable;
}

static void set_portamento_time(MidiChannel* chan, u8 value)
{
    chan->portamentoInterval = portaTimeToInterval[value];
}

static void set_fine_tune(MidiChannel* chan, u8 value)
{
    chan->fineTune = value - 64;
}

static void update_pb_range(MidiChannel* chan, u8 value, bool is_msb)
{
    if (is_msb) {
        chan->pitchBendRange.pitch = value;
    } else {
        chan->pitchBendRange.cents = value;
    }
    FOREACH_DEV_CHAN_WITH_MIDI(chan->num, devChan) {
        if (devChan->pitchBend != DEFAULT_MIDI_PITCH_BEND) {
            set_downstream_pitch(devChan);
        }
    }
}

static void update_rpn(MidiChannel* chan, u8 value, bool is_msb)
{
    if (is_msb) {
        chan->rpn = (value << 7) | (chan->rpn & 0x7F);
    } else {
        chan->rpn = (chan->rpn & 0xFF80) | (value & 0x7F);
    }
}

static void rpn_data_entry(MidiChannel* chan, u8 value, bool is_msb)
{
    switch (chan->rpn) {
    case RPN_PITCH_BEND_SENSITIVITY:
        update_pb_range(chan, value, is_msb);
        break;
    default:
        if (is_msb) {
            log_warn("Ch %d: RPN? 0x%04X MSB=0x%02X", chan->num + 1, chan->rpn, value);
        } else {
            log_warn("Ch %d: RPN? 0x%04X LSB=0x%02X", chan->num + 1, chan->rpn, value);
        }
        break;
    }
}

static void store_program_from_channel(u8 ch, u8 program)
{
    FOREACH_DEV_CHAN_WITH_MIDI(ch, devChan) {
        if (devChan->ops == &FM_VTable) {
            midi_fm_store_preset_from_channel(devChan->num, program);
            log_info("Prg %d: FM preset stored", program);
            break; // first chan we find only
        }
    }
}

void midi_cc(u8 ch, u8 controller, u8 value)
{
    MidiChannel* chan = &midiChannels[ch];
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
        all_notes_off(ch);
        break;
    case CC_GENMDM_POLYPHONIC_MODE:
        set_polyphonic_mode(RANGE(value, 2) != 0);
        break;
    case CC_GENMDM_CH3_SPECIAL_MODE:
        synth_set_special_mode(RANGE(value, 2) != 0);
        break;
    case CC_RESET_ALL_CONTROLLERS:
        resetAllControllers(ch);
        break;
    case CC_SHOW_PARAMETERS_ON_UI:
        if (isIgnoringNonGeneralMidiCCs())
            break;
        ui_fm_set_parameters_visibility(ch, RANGE(value, 2));
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
    case CC_DATA_ENTRY_MSB:
        rpn_data_entry(chan, value, true);
        break;
    case CC_DATA_ENTRY_LSB:
        rpn_data_entry(chan, value, false);
        break;
    case CC_RPN_MSB:
        update_rpn(chan, value, true);
        break;
    case CC_RPN_LSB:
        update_rpn(chan, value, false);
        break;
    case CC_EXPRESSION:
    case CC_SUSTAIN_PEDAL:
    case CC_NRPN_LSB:
    case CC_NRPN_MSB:
        break;
    case CC_STORE_PROGRAM:
        store_program_from_channel(ch, value);
        break;
    default:
        fm_parameter_cc(ch, controller, value);
        break;
    }

    if (changeCallback != NULL) {
        MidiChangeEvent event = {
            .type = MidiChangeType_CC,
            .chan = ch,
            .key = controller,
            .value = value,
        };
        changeCallback(event);
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

void midi_register_change_callback(void (*callback)(MidiChangeEvent))
{
    changeCallback = callback;
}
