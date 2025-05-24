#include "midi_fm.h"
#include "midi.h"
#include "synth.h"
#include "crc.h"
#include "log.h"

static const u8 SEMITONES = 12;
static const u16 FREQS[] = { 607, // B
    644, 681, 722, 765, 810, 858, 910, 964, 1021, 1081,
    1146, // A#
    // One more freqNum used for cents calculation
    1214 };

typedef struct MidiFmChannel {
    u8 pitch;
    s8 cents;
    u8 volume;
    u8 velocity;
    u8 pan;
    bool percussive;
} MidiFmChannel;

static MidiFmChannel fmChannels[MAX_FM_CHANS];

static u8 pitch_is_out_of_range(u8 pitch);
static u8 effective_volume(MidiFmChannel* channelState);
static void update_pan(u8 chan);
static u16 lookup_freq_num(u8 pitch, u8 offset);
void midi_fm_reset(void);

static const FmPreset** defaultPresets;
static const PercussionPreset** percussionPresets;

static FmPreset userPresets[MIDI_PROGRAMS];
static FmPreset* activeUserPresets[MIDI_PROGRAMS];

void midi_fm_init(const FmPreset** defPresets, const PercussionPreset** defaultPercussionPresets)
{
    defaultPresets = defPresets;
    percussionPresets = defaultPercussionPresets;
    midi_fm_reset();
}

void midi_fm_reset(void)
{
    memset(userPresets, 0, sizeof(userPresets));
    memset(activeUserPresets, 0, sizeof(activeUserPresets));

    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        MidiFmChannel* fmChan = &fmChannels[chan];
        fmChan->volume = MAX_MIDI_VOLUME;
        fmChan->velocity = MAX_MIDI_VOLUME;
        fmChan->pan = 0;
        fmChan->percussive = false;
        fmChan->cents = 0;
    }

    synth_init(defaultPresets[0]);
}

u16 midi_fm_pitch_cents_to_freq_num(u8 pitch, s8 cents)
{
    u16 freq = lookup_freq_num(pitch, 0);
    u16 nextFreq = lookup_freq_num(pitch, 1);
    return freq + (((nextFreq - freq) * cents) / 100);
}

static void set_synth_pitch(u8 chan)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    synth_pitch(chan, midi_fm_pitch_to_octave(fmChan->pitch),
        midi_fm_pitch_cents_to_freq_num(fmChan->pitch, fmChan->cents));
}

void midi_fm_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity)
{
    if (pitch_is_out_of_range(pitch)) {
        return;
    }
    MidiFmChannel* fmChan = &fmChannels[chan];
    if (fmChan->percussive) {
        const PercussionPreset* percussionPreset = percussionPresets[pitch];
        synth_preset(chan, &percussionPreset->preset);
        pitch = percussionPreset->key;
    }
    fmChan->velocity = velocity;
    synth_volume(chan, effective_volume(fmChan));
    fmChan->pitch = pitch;
    fmChan->cents = cents;

    set_synth_pitch(chan);
    synth_note_on(chan);
}

void midi_fm_note_off(u8 chan, u8 pitch)
{
    (void)pitch;
    synth_note_off(chan);
}

void midi_fm_channel_volume(u8 chan, u8 volume)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->volume = volume;
    synth_volume(chan, effective_volume(fmChan));
}

void midi_fm_program(u8 chan, u8 program)
{
    const FmPreset* data
        = activeUserPresets[program] ? activeUserPresets[program] : defaultPresets[program];
    synth_preset(chan, data);
}

void midi_fm_all_notes_off(u8 chan)
{
    midi_fm_note_off(chan, 0);
}

void midi_fm_percussive(u8 chan, bool enabled)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->percussive = enabled;
}

void midi_fm_pan(u8 chan, u8 pan)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->pan = pan;
    update_pan(chan);
}

static void update_pan(u8 chan)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    if (fmChan->pan > 95) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (fmChan->pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}

u8 midi_fm_pitch_to_octave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 lookup_freq_num(u8 pitch, u8 offset)
{
    return FREQS[((u8)(pitch - MIN_MIDI_PITCH) % SEMITONES) + offset];
}

static u8 pitch_is_out_of_range(u8 pitch)
{
    return pitch < MIN_MIDI_PITCH || pitch > MAX_MIDI_PITCH;
}

static u8 effective_volume(MidiFmChannel* fmChan)
{
    return (fmChan->volume * fmChan->velocity) / 0x7F;
}

void midi_fm_pitch(u8 chan, u8 pitch, s8 cents)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->pitch = pitch;
    fmChan->cents = cents;
    set_synth_pitch(chan);
}

static void sram_preset_slot(SramFmPresetSlot* slot, const FmPreset* preset)
{
    const u16 FM_PRESET_MAGIC_NUMBER = 0x9E1D;
    const u8 FM_PRESET_VERSION = 1;

    slot->magic_number = FM_PRESET_MAGIC_NUMBER;
    slot->version = FM_PRESET_VERSION;
    slot->preset.algorithm = preset->algorithm;
    slot->preset.feedback = preset->feedback;
    slot->preset.ams = preset->ams;
    slot->preset.fms = preset->fms;
    for (u8 i = 0; i < sizeof(slot->reserved); i++) {
        slot->reserved[i] = 0;
    }
    slot->checksum = 0;
    for (u8 i = 0; i < MAX_FM_OPERATORS; i++) {
        slot->preset.operators[i].multiple = preset->operators[i].multiple;
        slot->preset.operators[i].detune = preset->operators[i].detune;
        slot->preset.operators[i].attackRate = preset->operators[i].attackRate;
        slot->preset.operators[i].rateScaling = preset->operators[i].rateScaling;
        slot->preset.operators[i].decayRate = preset->operators[i].decayRate;
        slot->preset.operators[i].amplitudeModulation = preset->operators[i].amplitudeModulation;
        slot->preset.operators[i].sustainLevel = preset->operators[i].sustainLevel;
        slot->preset.operators[i].sustainRate = preset->operators[i].sustainRate;
        slot->preset.operators[i].releaseRate = preset->operators[i].releaseRate;
        slot->preset.operators[i].totalLevel = preset->operators[i].totalLevel;
        slot->preset.operators[i].ssgEg = preset->operators[i].ssgEg;
    }
}

void midi_fm_store_preset(u8 program, const FmPreset* preset)
{
    memcpy(&userPresets[program], preset, sizeof(FmPreset));
    activeUserPresets[program] = &userPresets[program];

    const u8 EXPECTED_SRAM_PRESET_SLOT_SIZE = 36;

    SramFmPresetSlot slot = { 0 };
    sram_preset_slot(&slot, preset);
    u16 crc = crc_calc_crc16((u8*)&slot, sizeof(SramFmPresetSlot) - sizeof(slot.checksum));
    slot.checksum = crc;

    if (sizeof(SramFmPresetSlot) != EXPECTED_SRAM_PRESET_SLOT_SIZE) {
        log_warn("FM preset slot size is %d", (u8)sizeof(SramFmPresetSlot));
        return;
    }

    const size_t SRAM_PRESET_BASE_OFFSET = 32;
    size_t offset = SRAM_PRESET_BASE_OFFSET + EXPECTED_SRAM_PRESET_SLOT_SIZE * program;
    SRAM_enable();
    for (u8 i = 0; i < sizeof(SramFmPresetSlot); i++) {
        SRAM_writeByte(offset++, ((u8*)&slot)[i]);
    }
    SRAM_disable();
}

void midi_fm_clear_preset(u8 program)
{
    memset(&userPresets[program], 0, sizeof(FmPreset));
    activeUserPresets[program] = NULL;
}