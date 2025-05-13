#include "midi_fm.h"
#include "midi.h"
#include "synth.h"

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

static const FmChannel** presets;
static const PercussionPreset** percussionPresets;

void midi_fm_init(
    const FmChannel** defaultPresets, const PercussionPreset** defaultPercussionPresets)
{
    presets = defaultPresets;
    percussionPresets = defaultPercussionPresets;
    midi_fm_reset();
}

void midi_fm_reset(void)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        MidiFmChannel* fmChan = &fmChannels[chan];
        fmChan->volume = MAX_MIDI_VOLUME;
        fmChan->velocity = MAX_MIDI_VOLUME;
        fmChan->pan = 0;
        fmChan->percussive = false;
        fmChan->cents = 0;
    }
    synth_init(presets[0]);
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
        synth_preset(chan, &percussionPreset->channel);
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
    const FmChannel* data = presets[program];
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
