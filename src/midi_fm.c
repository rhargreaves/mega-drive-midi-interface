#include "midi_fm.h"
#include "midi.h"
#include "synth.h"

static const u8 MIN_MIDI_PITCH = 11;
static const u8 MAX_MIDI_PITCH = 106;
static const u8 SEMITONES = 12;
static const u16 FREQ_NUMBERS[] = {
    617, // B
    653, 692, 733, 777, 823, 872, 924, 979, 1037, 1099,
    1164 // A#
};

typedef struct FmChannel FmChannel;

struct FmChannel {
    u8 pitch;
    u8 volume;
    u8 velocity;
    u8 pan;
    bool percussive;
};

static FmChannel fmChannels[MAX_FM_CHANS];

static u8 octave(u8 pitch);
static u16 freqNumber(u8 pitch);
static u8 pitchIsOutOfRange(u8 pitch);
static u8 effectiveVolume(FmChannel* channelState);
static void updatePan(u8 chan);

static const Channel** presets;
static const PercussionPreset** percussionPresets;

void midi_fm_init(const Channel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets)
{
    presets = defaultPresets;
    percussionPresets = defaultPercussionPresets;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        FmChannel* fmChan = &fmChannels[chan];
        fmChan->volume = MAX_MIDI_VOLUME;
        fmChan->velocity = MAX_MIDI_VOLUME;
        fmChan->pan = 0;
        fmChan->percussive = false;
    }
    synth_init(presets[0]);
}

void midi_fm_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (pitchIsOutOfRange(pitch)) {
        return;
    }
    FmChannel* fmChan = &fmChannels[chan];
    if (fmChan->percussive) {
        const PercussionPreset* percussionPreset = percussionPresets[pitch];
        synth_preset(chan, &percussionPreset->channel);
        pitch = percussionPreset->key;
    }
    fmChan->velocity = velocity;
    synth_volume(chan, effectiveVolume(fmChan));
    fmChan->pitch = pitch;
    synth_pitch(chan, octave(pitch), freqNumber(pitch));
    synth_noteOn(chan);
}

void midi_fm_noteOff(u8 chan, u8 pitch)
{
    synth_noteOff(chan);
}

void midi_fm_channelVolume(u8 chan, u8 volume)
{
    FmChannel* fmChan = &fmChannels[chan];
    fmChan->volume = volume;
    synth_volume(chan, effectiveVolume(fmChan));
}

void midi_fm_pitchBend(u8 chan, u16 bend)
{
    FmChannel* fmChan = &fmChannels[chan];
    u16 freq = freqNumber(fmChan->pitch);
    s16 bendRelative = bend - 0x2000;
    freq = freq + (bendRelative / 75);
    synth_pitch(chan, octave(fmChan->pitch), freq);
}

void midi_fm_program(u8 chan, u8 program)
{
    const Channel* data = presets[program];
    synth_preset(chan, data);
    updatePan(chan);
}

void midi_fm_allNotesOff(u8 chan)
{
    midi_fm_noteOff(chan, 0);
}

void midi_fm_percussive(u8 chan, bool enabled)
{
    FmChannel* fmChan = &fmChannels[chan];
    fmChan->percussive = enabled;
}

void midi_fm_pan(u8 chan, u8 pan)
{
    FmChannel* fmChan = &fmChannels[chan];
    fmChan->pan = pan;
    updatePan(chan);
}

static void updatePan(u8 chan)
{
    FmChannel* fmChan = &fmChannels[chan];
    if (fmChan->pan > 95) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (fmChan->pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}

static u8 octave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 freqNumber(u8 pitch)
{
    return FREQ_NUMBERS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}

static u8 pitchIsOutOfRange(u8 pitch)
{
    return pitch < MIN_MIDI_PITCH || pitch > MAX_MIDI_PITCH;
}

static u8 effectiveVolume(FmChannel* fmChan)
{
    return (fmChan->volume * fmChan->velocity) / 0x7F;
}
