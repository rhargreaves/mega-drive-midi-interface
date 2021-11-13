#include "midi_fm.h"
#include "midi.h"
#include "region.h"
#include "synth.h"

static const u8 MIN_MIDI_PITCH = 11;
static const u8 MAX_MIDI_PITCH = 106;
static const u8 SEMITONES = 12;
static const u16 FREQS_NTSC[] = {
    607, // B
    644, 681, 722, 765, 810, 858, 910, 964, 1021, 1081,
    1146 // A#
};

static const u16 FREQS_PAL[] = {
    613, // B
    649, 688, 729, 772, 818, 867, 918, 973, 1031, 1092,
    1157 // A#
};

typedef struct MidiFmChannel MidiFmChannel;

struct MidiFmChannel {
    u8 pitch;
    u8 volume;
    u8 velocity;
    u8 pan;
    bool percussive;
};

static MidiFmChannel fmChannels[MAX_FM_CHANS];

static u8 octave(u8 pitch);
static u16 freqNumber(u8 pitch);
static u8 pitchIsOutOfRange(u8 pitch);
static u8 effectiveVolume(MidiFmChannel* channelState);
static void updatePan(u8 chan);

static const FmChannel** presets;
static const PercussionPreset** percussionPresets;

void midi_fm_init(const FmChannel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets)
{
    presets = defaultPresets;
    percussionPresets = defaultPercussionPresets;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        MidiFmChannel* fmChan = &fmChannels[chan];
        fmChan->volume = MAX_MIDI_VOLUME;
        fmChan->velocity = MAX_MIDI_VOLUME;
        fmChan->pan = 0;
        fmChan->percussive = false;
    }
    synth_init(presets[0]);
}

void midi_fm_note_on(u8 chan, u8 pitch, u8 velocity)
{
    if (pitchIsOutOfRange(pitch)) {
        return;
    }
    MidiFmChannel* fmChan = &fmChannels[chan];
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

void midi_fm_note_off(u8 chan, u8 pitch)
{
    (void)pitch;
    synth_noteOff(chan);
}

void midi_fm_channel_volume(u8 chan, u8 volume)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->volume = volume;
    synth_volume(chan, effectiveVolume(fmChan));
}

void midi_fm_pitch_bend(u8 chan, u16 bend)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    u16 freq = freqNumber(fmChan->pitch);
    s16 bendRelative = bend - MIDI_PITCH_BEND_CENTRE;
    freq = freq + (bendRelative / 75);
    synth_pitch(chan, octave(fmChan->pitch), freq);
}

void midi_fm_program(u8 chan, u8 program)
{
    const FmChannel* data = presets[program];
    synth_preset(chan, data);
    updatePan(chan);
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
    updatePan(chan);
}

static void updatePan(u8 chan)
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

static u8 octave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 freqNumber(u8 pitch)
{
    const u16* freqs = region_isPal() ? FREQS_PAL : FREQS_NTSC;
    return freqs[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}

static u8 pitchIsOutOfRange(u8 pitch)
{
    return pitch < MIN_MIDI_PITCH || pitch > MAX_MIDI_PITCH;
}

static u8 effectiveVolume(MidiFmChannel* fmChan)
{
    return (fmChan->volume * fmChan->velocity) / 0x7F;
}
